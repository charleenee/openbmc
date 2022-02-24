// Copyright 2021-present Facebook. All Rights Reserved.
#include "ModbusDevice.h"
#include <iomanip>
#include <sstream>
#include "Log.h"

using nlohmann::json;

namespace rackmon {

void ModbusDeviceInfo::incErrors(uint32_t& counter) {
  counter++;
  if ((++numConsecutiveFailures) >= kMaxConsecutiveFailures) {
    mode = ModbusDeviceMode::DORMANT;
  }
}

ModbusDevice::ModbusDevice(
    Modbus& interface,
    uint8_t deviceAddress,
    const RegisterMap& registerMap)
    : interface_(interface) {
  info_.deviceAddress = deviceAddress;
  info_.baudrate = registerMap.defaultBaudrate;
  info_.deviceType = registerMap.name;

  for (auto& it : registerMap.registerDescriptors) {
    info_.registerList.emplace_back(it.second);
  }

  for (const auto& sp : registerMap.specialHandlers) {
    ModbusSpecialHandler hdl(deviceAddress);
    hdl.SpecialHandlerInfo::operator=(sp);
    specialHandlers_.push_back(hdl);
  }
}

void ModbusDevice::command(
    Msg& req,
    Msg& resp,
    ModbusTime timeout,
    ModbusTime settleTime) {
  // Try executing the command, if errors, catch the error
  // to maintain stats on types of errors and re-throw in
  // case the user wants to handle them in a special way.
  try {
    interface_.command(req, resp, info_.baudrate, timeout, settleTime);
    info_.numConsecutiveFailures = 0;
    info_.lastActive = std::time(nullptr);
  } catch (TimeoutException& e) {
    info_.incTimeouts();
    throw;
  } catch (CRCError& e) {
    info_.incCRCErrors();
    throw;
  } catch (std::runtime_error& e) {
    info_.incMiscErrors();
    logError << e.what() << std::endl;
    throw;
  } catch (...) {
    info_.incMiscErrors();
    logError << "Unknown exception" << std::endl;
    throw;
  }
}

void ModbusDevice::readHoldingRegisters(
    uint16_t registerOffset,
    std::vector<uint16_t>& regs,
    ModbusTime timeout) {
  ReadHoldingRegistersReq req(info_.deviceAddress, registerOffset, regs.size());
  ReadHoldingRegistersResp resp(info_.deviceAddress, regs);
  command(req, resp, timeout);
}

void ModbusDevice::writeSingleRegister(
    uint16_t registerOffset,
    uint16_t value,
    ModbusTime timeout) {
  WriteSingleRegisterReq req(info_.deviceAddress, registerOffset, value);
  WriteSingleRegisterResp resp(info_.deviceAddress, registerOffset);
  command(req, resp, timeout);
}

void ModbusDevice::writeMultipleRegisters(
    uint16_t registerOffset,
    std::vector<uint16_t>& value,
    ModbusTime timeout) {
  WriteMultipleRegistersReq req(info_.deviceAddress, registerOffset);
  for (uint16_t val : value) {
    req << val;
  }
  WriteMultipleRegistersResp resp(
      info_.deviceAddress, registerOffset, value.size());
  command(req, resp, timeout);
}

void ModbusDevice::readFileRecord(
    std::vector<FileRecord>& records,
    ModbusTime timeout) {
  ReadFileRecordReq req(info_.deviceAddress, records);
  ReadFileRecordResp resp(info_.deviceAddress, records);
  command(req, resp, timeout);
}

void ModbusDevice::monitor() {
  // If the number of consecutive failures has exceeded
  // a threshold, mark the device as dormant.
  uint32_t timestamp = std::time(nullptr);
  for (auto& specialHandler : specialHandlers_) {
    specialHandler.handle(*this);
  }
  std::unique_lock lk(registerListMutex_);
  for (auto& registerStore : info_.registerList) {
    uint16_t registerOffset = registerStore.regAddr();
    auto& nextRegister = registerStore.front();
    try {
      readHoldingRegisters(registerOffset, nextRegister.value);
      nextRegister.timestamp = timestamp;
      // If we dont care about changes or if we do
      // and we notice that the value is different
      // from the previous, increment store to
      // point to the next.
      if (!nextRegister.desc.storeChangesOnly ||
          nextRegister != registerStore.back()) {
        ++registerStore;
      }
    } catch (std::exception& e) {
      logInfo << "DEV:0x" << std::hex << int(info_.deviceAddress)
              << " ReadReg 0x" << std::hex << registerOffset << ' '
              << registerStore.name() << " caught: " << e.what() << std::endl;
      continue;
    }
  }
}

ModbusDeviceRawData ModbusDevice::getRawData() {
  std::unique_lock lk(registerListMutex_);
  // Makes a deep copy.
  return info_;
}

ModbusDeviceInfo ModbusDevice::getInfo() {
  std::unique_lock lk(registerListMutex_);
  return info_;
}

ModbusDeviceFmtData ModbusDevice::getFmtData() {
  std::unique_lock lk(registerListMutex_);
  ModbusDeviceFmtData data;
  data.ModbusDeviceInfo::operator=(info_);
  for (const auto& reg : info_.registerList) {
    std::string str = reg;
    data.registerList.emplace_back(std::move(str));
  }
  return data;
}

ModbusDeviceValueData ModbusDevice::getValueData() {
  std::unique_lock lk(registerListMutex_);
  ModbusDeviceValueData data;
  data.ModbusDeviceInfo::operator=(info_);
  for (const auto& reg : info_.registerList) {
    data.registerList.emplace_back(reg);
  }
  return data;
}

static std::string commandOutput(const std::string& shell) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(
      popen(shell.c_str(), "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

void ModbusSpecialHandler::handle(ModbusDevice& dev) {
  // Check if it is time to handle.
  if (!canHandle()) {
    return;
  }
  std::string strValue{};
  WriteMultipleRegistersReq req(deviceAddress_, reg);
  if (info.shell) {
    // The command is from the JSON configuration.
    // TODO, we currently only have need to set the
    // current UNIX time. If we want to avoid shell,
    // we might need a different way to generalize
    // this.
    strValue = commandOutput(info.shell.value());
  } else if (info.value) {
    strValue = info.value.value();
  } else {
    logError << "NULL action ignored" << std::endl;
    return;
  }
  if (info.interpret == RegisterValueType::INTEGER) {
    int32_t ival = std::stoi(strValue);
    if (len == 1) {
      req << uint16_t(ival);
    } else if (len == 2) {
      req << uint32_t(ival);
    } else {
      logError << "Value truncated to 32bits" << std::endl;
      req << uint32_t(ival);
    }
  } else if (info.interpret == RegisterValueType::STRING) {
    for (char c : strValue) {
      req << uint8_t(c);
    }
  }
  WriteMultipleRegistersResp resp(deviceAddress_, reg, len);
  try {
    dev.command(req, resp);
  } catch (std::exception& e) {
    logError << "Error executing special handler" << std::endl;
  }
  lastHandleTime_ = std::time(nullptr);
  handled_ = true;
}

NLOHMANN_JSON_SERIALIZE_ENUM(
    ModbusDeviceMode,
    {{ModbusDeviceMode::ACTIVE, "active"},
     {ModbusDeviceMode::DORMANT, "dormant"}})

// Legacy JSON format.
void to_json(json& j, const ModbusDeviceInfo& m) {
  j["addr"] = m.deviceAddress;
  j["crc_fails"] = m.crcErrors;
  j["timeouts"] = m.timeouts;
  j["misc_fails"] = m.miscErrors;
  j["mode"] = m.mode;
  j["baudrate"] = m.baudrate;
  j["deviceType"] = m.deviceType;
}

// Legacy JSON format.
void to_json(json& j, const ModbusDeviceRawData& m) {
  const ModbusDeviceInfo& s = m;
  to_json(j, s);
  j["now"] = std::time(nullptr);
  j["ranges"] = m.registerList;
}

// Deprecated string JSON format.
void to_json(json& j, const ModbusDeviceFmtData& m) {
  const ModbusDeviceInfo& s = m;
  to_json(j, s);
  j["now"] = std::time(nullptr);
  j["ranges"] = m.registerList;
}

// v2.0 JSON Format.
void to_json(json& j, const ModbusDeviceValueData& m) {
  j["deviceAddress"] = m.deviceAddress;
  j["deviceType"] = m.deviceType;
  j["crcErrors"] = m.crcErrors;
  j["timeouts"] = m.timeouts;
  j["miscErrors"] = m.miscErrors;
  j["baudrate"] = m.baudrate;
  j["mode"] = m.mode;
  j["now"] = std::time(0);
  j["registers"] = m.registerList;
}

} // namespace rackmon
