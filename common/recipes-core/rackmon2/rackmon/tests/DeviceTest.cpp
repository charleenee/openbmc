// Copyright 2021-present Facebook. All Rights Reserved.
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <chrono>
#include <fstream>
#include <thread>
#include "Device.hpp"

#if (__GNUC__ < 8)
#include <experimental/filesystem>
namespace std {
namespace filesystem = experimental::filesystem;
}
#else
#include <filesystem>
#endif

using namespace std::literals;
using namespace rackmon;

class DeviceTest : public ::testing::Test {
 public:
  std::string test_device_path{};
 protected:
  void SetUp() override {
    std::filesystem::path test_path = std::filesystem::temp_directory_path();
    if (!std::filesystem::exists(test_path)) {
      test_path = std::filesystem::current_path();
    }
    test_device_path = test_path / "test.bin";
    std::vector<uint8_t> out = {1, 2, 3, 4};
    std::ofstream fp(test_device_path, std::ios::binary);
    std::copy(out.begin(), out.end(), std::ostreambuf_iterator<char>(fp));
    fp.close();
  }
  void TearDown() override {
    remove(test_device_path.c_str());
  }
};

TEST_F(DeviceTest, OpenClose) {
  Device dev_bad("./test2.bin");
  EXPECT_THROW(dev_bad.open(), std::system_error);
  Device dev(test_device_path);
  dev.open();
  dev.close();
}

TEST_F(DeviceTest, ReadTest) {
  Device dev(test_device_path);
  dev.open();
  std::vector<uint8_t> buf(4);
  dev.read(buf.data(), buf.size(), 10);
  std::vector<uint8_t> exp = {1, 2, 3, 4};
  ASSERT_EQ(buf, exp);
}

TEST_F(DeviceTest, WriteTest) {
  Device dev(test_device_path);
  dev.open();
  std::vector<uint8_t> wbuf = {5, 6, 7, 8};
  dev.write(wbuf.data(), wbuf.size());
  dev.close();
  dev.open();
  std::vector<uint8_t> rbuf(4);
  dev.read(rbuf.data(), rbuf.size(), 10);
  ASSERT_EQ(rbuf, wbuf);
}
