#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <sys/mman.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <openbmc/kv.h>
#include <openbmc/libgpio.h>
#include <openbmc/obmc-i2c.h>
#include <openbmc/obmc-sensors.h>
#include <facebook/exp.h>
#include <facebook/fbgc_gpio.h>
#include "pal.h"

#define MAX_CMD_LEN 64

static int
server_power_12v_on() {
  int i2cfd = 0;
  uint8_t status = 0;
  int ret = 0, retry = 0, times = 0;
  i2c_master_rw_command command;
  char cmd[MAX_CMD_LEN] = {0};
  
  memset(&command, 0, sizeof(command));
  command.offset = 0x00;
  command.val = STAT_12V_ON;

  i2cfd = i2c_cdev_slave_open(I2C_UIC_FPGA_BUS, UIC_FPGA_SLAVE_ADDR >> 1, I2C_SLAVE_FORCE_CLAIM);
  if (i2cfd < 0) {
    ret = i2cfd;
    syslog(LOG_WARNING, "%s() Failed to open %d", __func__, I2C_UIC_FPGA_BUS);
    goto exit;
  }

  while (retry < MAX_RETRY) {
    ret = i2c_rdwr_msg_transfer(i2cfd, UIC_FPGA_SLAVE_ADDR, (uint8_t *)&command, sizeof(command), NULL, 0);
    if (ret < 0) {
      retry++;
      msleep(100);
    } else {
      break;
    }
  }
  if (retry == MAX_RETRY) {
    syslog(LOG_WARNING, "%s() Failed to do i2c_rdwr_msg_transfer, tlen=%d", __func__, sizeof(command));
    goto exit;
  }
  
  while(times < WAIT_POWER_STATUS_CHANGE_TIME) {
    ret = pal_get_server_12v_power(FRU_SERVER, &status);
    if ((ret == 0) && (status == SERVER_12V_ON)) {
      // store VR version to cache
      snprintf(cmd, sizeof(cmd), "/usr/bin/fw-util server --version vr > /dev/null 2>&1");
      if (system(cmd) != 0) {
        syslog(LOG_WARNING, "%s(): get VR version failed\n", __func__);
        ret = PAL_ENOTSUP;
      }
      goto exit;
    } else {
      ret = -1;
      times++;
      sleep(1);
    }
  }

exit:
  if (i2cfd >= 0) {
    close(i2cfd);
  }
    
  return ret;
}

static int
server_power_12v_off() {
  int i2cfd = 0;
  uint8_t status = 0;
  int ret = 0, retry = 0, times = 0;
  i2c_master_rw_command command;
  char cmd[MAX_CMD_LEN] = {0};
  
  memset(&command, 0, sizeof(command));
  command.offset = 0x00;
  command.val = STAT_12V_OFF;

  i2cfd = i2c_cdev_slave_open(I2C_UIC_FPGA_BUS, UIC_FPGA_SLAVE_ADDR >> 1, I2C_SLAVE_FORCE_CLAIM);
  if (i2cfd < 0) {
    ret = i2cfd;
    syslog(LOG_WARNING, "%s() Failed to open %d", __func__, I2C_UIC_FPGA_BUS);
    goto exit;
  }

  while (retry < MAX_RETRY) {
    ret = i2c_rdwr_msg_transfer(i2cfd, UIC_FPGA_SLAVE_ADDR, (uint8_t *)&command, sizeof(command), NULL, 0);
    if (ret < 0) {
      retry++;
      msleep(100);
    } else {
      break;
    }
  }
  if (retry == MAX_RETRY) {
    syslog(LOG_WARNING, "%s() Failed to do i2c_rdwr_msg_transfer fails, tlen=%d", __func__, sizeof(command));
    goto exit;
  }
  
  while(times < WAIT_POWER_STATUS_CHANGE_TIME) {
    ret = pal_get_server_12v_power(FRU_SERVER, &status);
    if ((ret == 0) && (status == SERVER_12V_OFF)) {
      // clear VR verison cache
      snprintf(cmd, sizeof(cmd), "rm -f /tmp/cache_store/vr_*_crc");
      if (system(cmd) != 0) {
        syslog(LOG_WARNING, "%s(): remove VR version cache failed\n", __func__);
        ret = PAL_ENOTSUP;
      }
      goto exit;
    } else {
      ret = -1;
      times++;
      sleep(1);
    }
  }

exit:
  if (i2cfd >= 0) {
    close(i2cfd);
  }
  
  return ret;
}

uint8_t
pal_set_power_restore_policy(uint8_t slot, uint8_t *pwr_policy, uint8_t *res_data) {
  uint8_t completion_code = CC_SUCCESS; // Fill response with default values
  char key[MAX_KEY_LEN] = {0};
  unsigned char policy = 0x00;
  
  if (pwr_policy == NULL) {
    syslog(LOG_WARNING, "%s() NULL pointer: *pwr_policy", __func__);
    return -1;
  }
  
  policy = *pwr_policy & 0x07;  // Power restore policy
  memset(&key, 0, sizeof(key));
  snprintf(key, MAX_KEY_LEN, "server_por_cfg");
  switch (policy)
  {
    case POWER_CFG_OFF:
      if (pal_set_key_value(key, "off") != 0) {
        completion_code = CC_UNSPECIFIED_ERROR;
      }
      break;
    case POWER_CFG_LPS:
      if (pal_set_key_value(key, "lps") != 0) {
        completion_code = CC_UNSPECIFIED_ERROR;
      }
      break;
    case POWER_CFG_ON:
      if (pal_set_key_value(key, "on") != 0) {
        completion_code = CC_UNSPECIFIED_ERROR;
      }
      break;
    case POWER_CFG_UKNOWN:
      // no change (just get present policy support)
      break;
    default:
      completion_code = CC_PARAM_OUT_OF_RANGE;
      break;
  }
  return completion_code;
}

void
pal_get_chassis_status(uint8_t fru, uint8_t *req_data, uint8_t *res_data, uint8_t *res_len) {
  char buff[MAX_VALUE_LEN] = {0};
  int policy = POWER_CFG_UKNOWN;
  uint8_t status = 0;
  int ret = 0;
  unsigned char *data = res_data;

  if (res_data == NULL) {
    syslog(LOG_WARNING, "%s() NULL pointer: *res_data", __func__);
  }
  
  if (res_len == NULL) {
    syslog(LOG_WARNING, "%s() NULL pointer: *res_len", __func__);
  }
  
  memset(&buff, 0, sizeof(buff));
  if (pal_get_key_value("server_por_cfg", buff) == 0) {
    if (!memcmp(buff, "off", strlen("off"))) {
      policy = POWER_CFG_OFF;
    } else if (!memcmp(buff, "lps", strlen("lps"))) {
      policy = POWER_CFG_LPS;
    } else if (!memcmp(buff, "on", strlen("on"))) {
      policy = POWER_CFG_ON;
    } else {
      policy = POWER_CFG_UKNOWN;
    }
  }

  // Current Power State
  ret = pal_get_server_power(FRU_SERVER, &status);
  if (ret >= 0) {
    *data++ = status | (policy << 5);
  } else {
    syslog(LOG_WARNING, "%s: pal_get_server_power failed", __func__);
    *data++ = 0x00 | (policy << 5);
  }

  *data++ = 0x00;   // Last Power Event
  *data++ = 0x40;   // Misc. Chassis Status
  *data++ = 0x00;   // Front Panel Button Disable
  *res_len = data - res_data;
}

static int
set_nic_power_mode(nic_power_control_mode nic_mode) {
  SET_NIC_POWER_MODE_CMD cmd = {0};
  int fd = 0, ret = 0, retry = 0;

  cmd.nic_power_control_cmd_code = CMD_CODE_NIC_POWER_CONTROL;
  cmd.nic_power_control_mode = (uint8_t)nic_mode;

  fd = i2c_cdev_slave_open(I2C_UIC_FPGA_BUS, UIC_FPGA_SLAVE_ADDR >> 1, I2C_SLAVE_FORCE_CLAIM);
  if (fd < 0) {
    syslog(LOG_WARNING, "%s() Failed to open i2c bus %d", __func__, I2C_UIC_FPGA_BUS);
    return -1;
  }

  while (retry < MAX_RETRY) {
    ret = i2c_rdwr_msg_transfer(fd, UIC_FPGA_SLAVE_ADDR, (uint8_t *)&cmd, sizeof(cmd), NULL, 0);
    if (ret < 0) {
      retry++;
      msleep(100);
    } else {
      break;
    }
  }
  if (retry == MAX_RETRY) {
    syslog(LOG_WARNING, "%s() Failed to send \"set NIC power mode\" command to UIC FPGA", __func__);
  }

  if (fd >= 0) {
    close(fd);
  }

  if (ret == 0 && nic_mode == NIC_VMAIN_MODE) {
    sleep(2); //sleep 2s to wait for PERST, 2s is enough for FPGA
  }

  return ret;
}

int
pal_host_power_off_pre_actions() {
  if (gpio_set_init_value_by_shadow(fbgc_get_gpio_name(GPIO_E1S_1_P3V3_PG_R), GPIO_VALUE_LOW) < 0) {
    syslog(LOG_ERR, "%s() Failed to disable E1S0/IOCM I2C\n", __func__);
    return -1;
  }
  if (gpio_set_init_value_by_shadow(fbgc_get_gpio_name(GPIO_E1S_2_P3V3_PG_R), GPIO_VALUE_LOW) < 0) {
    syslog(LOG_ERR, "%s() Failed to disable E1S1/IOCM I2C\n", __func__);
    return -1;
  }

  return 0;
}

int
pal_host_power_off_post_actions() {
  int ret = 0;

  ret = set_nic_power_mode(NIC_VAUX_MODE);
  if (ret < 0) {
    syslog(LOG_ERR, "%s() Failed to set NIC to VAUX mode\n", __func__);
  }

  return ret;
}

int
pal_host_power_on_pre_actions() {
  int ret = 0;

  ret = set_nic_power_mode(NIC_VMAIN_MODE);
  if (ret < 0) {
    syslog(LOG_ERR, "%s() Failed to set NIC to VMAIN mode, stop power on host\n", __func__);
  }

  return ret;
}

int
pal_restore_host_power_on_pre_actions() {
  int ret = 0;

  ret = set_nic_power_mode(NIC_VAUX_MODE);
  if (ret < 0) {
    syslog(LOG_ERR, "%s() Failed to set NIC to VAUX mode\n", __func__);
  }

  return ret;
}

int
pal_host_power_on_post_actions() {
  char path[MAX_PATH_LEN] = {0};
  int ret = 0;
  uint8_t chassis_type = 0;

  if (is_e1s_iocm_present(T5_E1S0_T7_IOC_AVENGER) == true) {
    if (gpio_set_init_value_by_shadow(fbgc_get_gpio_name(GPIO_E1S_1_P3V3_PG_R), GPIO_VALUE_HIGH) < 0) {
      syslog(LOG_ERR, "%s() Failed to enable E1.S0/IOCM I2C\n", __func__);
      return -1;
    }
  }

  if (is_e1s_iocm_present(T5_E1S1_T7_IOCM_VOLT) == true) {
    if (gpio_set_init_value_by_shadow(fbgc_get_gpio_name(GPIO_E1S_2_P3V3_PG_R), GPIO_VALUE_HIGH) < 0) {
      syslog(LOG_ERR, "%s() Failed to enable E1.S1/IOCM I2C\n", __func__);
      return -1;
    }
  }

  if (fbgc_common_get_chassis_type(&chassis_type) < 0) {
    syslog(LOG_WARNING, "%s() Failed to get chassis type. If the system is Type 7, will not get IOCM FRU information.\n", __func__);
    return -2;
  }

  if (chassis_type == CHASSIS_TYPE7) {
    if (access(IOCM_EEPROM_BIND_DIR, F_OK) == -1) { // bind device if not bound
      ret = pal_bind_i2c_device(I2C_T5E1S1_T7IOC_BUS, IOCM_EEPROM_ADDR, IOCM_EEPROM_DRIVER_NAME);
      if (ret < 0) {
        syslog(LOG_WARNING, "%s() Failed to bind i2c device on bus: %u, addr: 0x%X, device: %s, ret : %d\n",
          __func__, I2C_T5E1S1_T7IOC_BUS, IOCM_EEPROM_ADDR, IOCM_EEPROM_DRIVER_NAME, ret);
        syslog(LOG_WARNING, "%s() Will not get IOCM FRU information", __func__);
        return -2;
      }
    }
    if (access(FRU_IOCM_BIN, F_OK) == -1) {
      snprintf(path, sizeof(path), EEPROM_PATH, I2C_T5E1S1_T7IOC_BUS, IOCM_FRU_ADDR);
      if (pal_copy_eeprom_to_bin(path, FRU_IOCM_BIN) < 0) {
        syslog(LOG_WARNING, "%s() Failed to copy %s to %s", __func__, path, FRU_IOCM_BIN);
        return -2;
      }
      if (pal_check_fru_is_valid(FRU_IOCM_BIN) < 0) {
        syslog(LOG_WARNING, "%s() The FRU %s is wrong.", __func__, FRU_IOCM_BIN);
      }
    }
  }

  return 0;
}

static int
power_12v_on_post_actions() {
  int ret = 0;
  
  // Update Server FPGA version
  ret = pal_set_fpga_ver_cache(I2C_BS_FPGA_BUS, GET_FPGA_VER_ADDR);
  if (ret < 0) {
    syslog(LOG_WARNING, "%s(): Failed to update Server FPGA version", __func__);
  }
  
  return ret;
}

int
pal_get_server_12v_power(uint8_t fru, uint8_t *status) {
  int ret = 0;
  uint8_t status_12v = 0;
  int i2cfd = 0, retry = 0;
  uint8_t server_present_status = 0;
  i2c_master_rw_command command;
  
  if (status == NULL) {
    syslog(LOG_WARNING, "%s() NULL pointer: *status", __func__);
    return -1;
  }
  
  memset(&command, 0, sizeof(command));
  command.offset = 0x00;
  
  ret = pal_is_fru_prsnt(FRU_SERVER, &server_present_status);
  if (ret < 0) {
    syslog(LOG_WARNING, "%s(): fail to get server present status\n", __func__);
    return ret;
  }

  if (server_present_status == FRU_PRESENT) {
    ret = fbgc_common_server_stby_pwr_sts(&status_12v);
    if (ret < 0) {
      syslog(LOG_WARNING, "%s(): fail to get server 12V status\n", __func__);
      return ret;
    }
  
    if (status_12v == STAT_AC_ON) {
      *status = SERVER_12V_ON;
    } else {
      *status = SERVER_12V_OFF;
    }
  
  // When server not present, get power status from UIC FPGA register
  } else { 
    i2cfd = i2c_cdev_slave_open(I2C_UIC_FPGA_BUS, UIC_FPGA_SLAVE_ADDR >> 1, I2C_SLAVE_FORCE_CLAIM);
    if (i2cfd < 0) {
      syslog(LOG_WARNING, "%s() fail to open device: I2C bus: %d", __func__, I2C_UIC_FPGA_BUS);
      return i2cfd;
    }

    while (retry < MAX_RETRY) {
      ret = i2c_rdwr_msg_transfer(i2cfd, UIC_FPGA_SLAVE_ADDR, (uint8_t *)&command.offset, sizeof(command.offset), &status_12v, sizeof(status_12v));
      if (ret < 0) {
        retry++;
        msleep(100);
      } else {
        break;
      }
    }

    if (retry == MAX_RETRY) {
      syslog(LOG_WARNING, "%s() fail to read UIC FPGA offset: 0x%02X via i2c", __func__, command.offset);
      close(i2cfd);
      return ret;
    }

    if (status_12v == STAT_AC_ON) {
      *status = SERVER_12V_ON;
    } else {
      *status = SERVER_12V_OFF;
    }
    close(i2cfd);
  }  

  return ret;
}

int
pal_get_server_power(uint8_t fru, uint8_t *status) {
  int ret = 0, i2cfd = 0;
  char server_fpga_ver[MAX_VALUE_LEN] = {0};
  char server_fpga_stage[MAX_VALUE_LEN] = {0};
  char server_fpga_ver_num[MAX_VALUE_LEN] = {0};
  bool is_ctrl_via_bic = true;
  i2c_master_rw_command command;
  uint8_t res = 0;
  int rlen = sizeof(res);
  
  if (status == NULL) {
    syslog(LOG_WARNING, "%s() NULL pointer: *status", __func__);
    return -1;
  }

  if (fru != FRU_SERVER) {
    syslog(LOG_WARNING, "%s: fru %u not a server\n", __func__, fru);
    return POWER_STATUS_FRU_ERR;
  }

  ret = pal_get_server_12v_power(fru, status);
  if (ret < 0 || (*status) == SERVER_12V_OFF) {
    // return earlier if power state is SERVER_12V_OFF or error happened
    return ret;
  }
  
  ret = pal_get_fpga_ver_cache(I2C_BS_FPGA_BUS, GET_FPGA_VER_ADDR, server_fpga_ver);
  if (ret == 0) {
    snprintf(server_fpga_stage, sizeof(server_fpga_stage), "%c%c", server_fpga_ver[4], server_fpga_ver[5]);
    snprintf(server_fpga_ver_num, sizeof(server_fpga_ver_num), "%c%c", server_fpga_ver[6], server_fpga_ver[7]);
    
    if (((strcmp(server_fpga_stage, "0D") == 0) && (atoi(server_fpga_ver_num) >= 0x02))
      || (strcmp(server_fpga_stage, "0A") == 0)) {
      is_ctrl_via_bic = false;
    }
  }

  if (is_ctrl_via_bic == false) {
    i2cfd = i2c_cdev_slave_open(I2C_BS_FPGA_BUS, BS_FPGA_SLAVE_ADDR >> 1, I2C_SLAVE_FORCE_CLAIM);
    if (i2cfd < 0) {
      syslog(LOG_ERR, "Fail to control server power because I2C BUS: %d open failed", I2C_BS_FPGA_BUS);
      return i2cfd;
    }
  
    memset(&command, 0, sizeof(command));
    command.offset = BS_FPGA_SERVER_POWER_STATUS;
  
    ret = i2c_rdwr_msg_transfer(i2cfd, BS_FPGA_SLAVE_ADDR, (uint8_t *)&command.offset, sizeof(command.offset), &res, rlen);
    close(i2cfd);
    
    *status = GETBIT(res, 0);
    if (ret < 0) {
      // if server FPGA not responding, we reset status to SERVER_12V_ON
      *status = SERVER_12V_ON;
      syslog(LOG_WARNING, "%s(): SERVER FPGA no response, server DC power status is unknown\n", __func__);
    }
    
  } else {
    ret = bic_get_server_power_status(status);
    if (ret < 0) {
      // if bic not responding, we reset status to SERVER_12V_ON
      *status = SERVER_12V_ON;
      syslog(LOG_WARNING, "%s(): BIC no response, server DC power status is unknown\n", __func__);
    }
  }

  return 0;
}

// Host DC Off, Host DC On, or Host Reset the server
int
pal_set_server_power(uint8_t fru, uint8_t cmd) {
  uint8_t status = 0;
  int ret = 0;
  bool is_ctrl_via_bic = true;
  char server_fpga_ver[MAX_VALUE_LEN] = {0};
  char server_fpga_stage[MAX_VALUE_LEN] = {0};
  char server_fpga_ver_num[MAX_VALUE_LEN] = {0};

  if (fru != FRU_SERVER) {
    syslog(LOG_WARNING, "%s: fru %u not a server\n", __func__, fru);
    return POWER_STATUS_FRU_ERR;
  }
  
  if (pal_get_server_power(fru, &status) < 0) {
    syslog(LOG_WARNING, "%s: fru %u get sever power error %u\n", __func__, fru, status);
    return POWER_STATUS_ERR;
  }
  
  // Check Server FPGA f/w version
  // (1)new: BMC control directly (2)old: control via BIC
  ret = pal_get_fpga_ver_cache(I2C_BS_FPGA_BUS, GET_FPGA_VER_ADDR, server_fpga_ver);
  if (ret == 0) {
    snprintf(server_fpga_stage, sizeof(server_fpga_stage), "%c%c", server_fpga_ver[4], server_fpga_ver[5]);
    snprintf(server_fpga_ver_num, sizeof(server_fpga_ver_num), "%c%c", server_fpga_ver[6], server_fpga_ver[7]);
    
    // after stage: DVT num: 02 support BMC control directly
    if (((strcmp(server_fpga_stage, "0D") == 0) && (atoi(server_fpga_ver_num) >= 0x02))
      || (strcmp(server_fpga_stage, "0A") == 0)) {
      is_ctrl_via_bic = false;
    }
  }
  
  if (is_ctrl_via_bic == true) {
    printf("Warning: Server FPGA fw version is too old, please update\n");
  }

  // Discard all the non-12V power control commands if 12V is off
  switch (cmd) {
    case SERVER_12V_OFF:
    case SERVER_12V_ON:
    case SERVER_12V_CYCLE:
      //do nothing
      break;
    default:
      if (status == SERVER_12V_OFF) {
        // discard the commands if power state is 12V-off
        return POWER_STATUS_ERR;
      }
      break;
  }

  switch(cmd) {
    case SERVER_POWER_ON:
      if (status == SERVER_POWER_ON) {
        return POWER_STATUS_ALREADY_OK;
      }
      ret = pal_host_power_on_pre_actions();
      if (ret < 0) {
        return POWER_STATUS_ERR;
      }
      
      if (is_ctrl_via_bic == true) {
        ret = bic_server_power_ctrl(SET_DC_POWER_ON);
      } else {
        ret = pal_server_power_ctrl(SET_DC_POWER_ON);
      }
      
      if (ret == 0) {
        ret = pal_check_server_power_change_correct(SET_DC_POWER_ON);
      }

      if (ret == 0) {
        pal_host_power_on_post_actions();
      }
      return ret;
      
    case SERVER_POWER_OFF:
      if (status == SERVER_POWER_OFF) {
        return POWER_STATUS_ALREADY_OK;
      }
      if (pal_host_power_off_pre_actions() < 0 ) {
        return POWER_STATUS_ERR;
      }

      if (is_ctrl_via_bic == true) {
        ret = bic_server_power_ctrl(SET_DC_POWER_OFF);
      } else {
        ret = pal_server_power_ctrl(SET_DC_POWER_OFF);
      }
      
      if (ret == 0) {
        ret = pal_check_server_power_change_correct(SET_DC_POWER_OFF);
      }
      
      if (ret == 0) {
        pal_host_power_off_post_actions();
      }
      return ret;

    case SERVER_POWER_CYCLE:
      if (status == SERVER_POWER_ON) {
        if (pal_host_power_off_pre_actions() < 0 ) {
          return POWER_STATUS_ERR;
        }
        
        if (is_ctrl_via_bic == true) {
          ret = bic_server_power_cycle();
        } else {
          ret = pal_server_power_cycle();
        }
        
        if (ret == 0) {
          pal_host_power_on_post_actions();
        }
      }
      
      if (is_ctrl_via_bic == true) {
        ret = bic_server_power_ctrl(SET_DC_POWER_ON);
      } else {
        ret = pal_server_power_ctrl(SET_DC_POWER_ON);
      }
      
      if (ret == 0) {
        ret = pal_check_server_power_change_correct(SET_DC_POWER_ON);
      }
      
      if (ret == 0) {
        pal_host_power_on_post_actions();
      }
      return ret;

    case SERVER_GRACEFUL_SHUTDOWN:
      if (status == SERVER_POWER_OFF) {
        return POWER_STATUS_ALREADY_OK;
      }
      if (pal_host_power_off_pre_actions() < 0 ) {
        return POWER_STATUS_ERR;
      }
      
      if (is_ctrl_via_bic == true) {
        ret = bic_server_power_ctrl(SET_GRACEFUL_POWER_OFF);
      } else {
        ret = pal_server_power_ctrl(SET_GRACEFUL_POWER_OFF);
      }
      
      if (ret == 0) {
        ret = pal_check_server_power_change_correct(SET_GRACEFUL_POWER_OFF);
      }
      
      if (ret == 0) {
        pal_host_power_off_post_actions();
      }
      return ret;

    case SERVER_POWER_RESET:
      if (status == SERVER_POWER_OFF) {
        return POWER_STATUS_ERR;
      }
      return bic_server_power_ctrl(SET_HOST_RESET);

    case SERVER_12V_ON:
      if (status != SERVER_12V_OFF) {
        return POWER_STATUS_ALREADY_OK;
      }
      
      ret = server_power_12v_on();
      if (ret == 0) {
        power_12v_on_post_actions();
      }
      
      return ret;

    case SERVER_12V_OFF:
      if (status == SERVER_12V_OFF) {
        return POWER_STATUS_ALREADY_OK;
      }
      if (pal_host_power_off_pre_actions() < 0 ) {
        return POWER_STATUS_ERR;
      }
      ret = server_power_12v_off();
      if (ret == 0) {
        pal_host_power_off_post_actions();
      }
      return ret;

    case SERVER_12V_CYCLE:
      if (status == SERVER_12V_OFF) {
        return server_power_12v_on();
      } else {
        if (pal_host_power_off_pre_actions() < 0 ) {
          return POWER_STATUS_ERR;
        }
        if (server_power_12v_off() < 0) {
          return POWER_STATUS_ERR;
        }
        sleep(SERVER_AC_CYCLE_DELAY);
        if (server_power_12v_on() < 0) {
          return POWER_STATUS_ERR;
        } else {
          power_12v_on_post_actions();
        }
      }
      break;

    default:
      syslog(LOG_WARNING, "%s() wrong cmd: 0x%02X", __func__, cmd);
      return POWER_STATUS_ERR;
  }

  return POWER_STATUS_OK;
}

int
pal_sled_cycle(void) {
  int ret = 0;
  
  uint8_t tbuf[MAX_IPMB_BUFFER] = {0x00};
  uint8_t rbuf[MAX_IPMB_BUFFER] = {0x00};
  uint8_t rlen = 0, tlen = 0;

  ret = expander_ipmb_wrapper(NETFN_OEM_REQ, CME_OEM_EXP_SLED_CYCLE, tbuf, tlen, rbuf, &rlen);
  if (ret < 0) {
    syslog(LOG_WARNING, "%s() expander_ipmb_wrapper failed. ret: %d\n", __func__, ret);
    return PAL_ENOTSUP;
  }

  return POWER_STATUS_OK;
}

int
pal_set_last_pwr_state(uint8_t fru, char *state) {
  int ret = 0;
  
  if (state == NULL) {
    syslog(LOG_WARNING, "%s() NULL pointer: *state", __func__);
    return -1;
  }

  ret = pal_set_key_value("pwr_server_last_state", state);
  if (ret < 0) {
    syslog(LOG_WARNING, "%s: pal_set_key_value failed", __func__);
  }

  return ret;
}

int
pal_get_last_pwr_state(uint8_t fru, char *state) {
  int ret = 0;
  
  if (state == NULL) {
    syslog(LOG_WARNING, "%s() NULL pointer: *state", __func__);
    return -1;
  }

  ret = pal_get_key_value("pwr_server_last_state", state);
  if (ret < 0) {
    syslog(LOG_WARNING, "%s: pal_get_key_value failed", __func__);
  }

  return ret;
}

int
pal_server_power_ctrl(uint8_t action) {
  uint8_t pwr_seq[PWR_CTRL_ACT_CNT] = {SERVER_POWER_BTN_HIGH, SERVER_POWER_BTN_LOW, SERVER_POWER_BTN_HIGH};
  int ret = 0, i = 0;
  
  if ( (action != SET_DC_POWER_OFF) && (action != SET_DC_POWER_ON) && (action != SET_GRACEFUL_POWER_OFF)) {
    syslog(LOG_ERR, "%s() invalid action\n", __func__);
    return -1;
  }
  
  for (i = 0; i < PWR_CTRL_ACT_CNT; i++) {
    ret = pal_set_pwr_btn(pwr_seq[i]);
    if (pwr_seq[i] == SERVER_POWER_BTN_LOW) {
      switch (action) {
        case SET_DC_POWER_OFF:
          sleep(DELAY_DC_POWER_OFF);
          break;
        case SET_DC_POWER_ON:
          sleep(DELAY_DC_POWER_ON);
          break;
        case SET_GRACEFUL_POWER_OFF:
          sleep(DELAY_GRACEFUL_SHUTDOWN);
          break;
        default:
          return -1;
      }
    }
  }
  
  return ret;
}

int
pal_set_pwr_btn(uint8_t val) {
  int i2cfd = 0, ret = 0;
  i2c_master_rw_command command;
  
  i2cfd = i2c_cdev_slave_open(I2C_BS_FPGA_BUS, BS_FPGA_SLAVE_ADDR >> 1, I2C_SLAVE_FORCE_CLAIM);
  if (i2cfd < 0) {
    syslog(LOG_ERR, "Fail to control server power because I2C BUS: %d open failed", I2C_BS_FPGA_BUS);
    return i2cfd;
  }
  
  memset(&command, 0, sizeof(command));
  command.offset = BS_FPGA_SERVER_POWER_CTRL;
  command.val = val;
  
  ret = i2c_rdwr_msg_transfer(i2cfd, BS_FPGA_SLAVE_ADDR, (uint8_t *)&command, sizeof(command), NULL, 0);
  close(i2cfd);
  return ret;
}

int
pal_server_power_cycle() {
  int ret = 0;

  ret = pal_server_power_ctrl(SET_DC_POWER_OFF);
  if (ret < 0) {
    syslog(LOG_ERR, "%s() Cannot set power state to off\n", __func__);
    return ret;
  }

  sleep(DELAY_DC_POWER_CYCLE);

  ret = pal_server_power_ctrl(SET_DC_POWER_ON);
  if (ret < 0) {
    syslog(LOG_ERR, "%s() Cannot set power state to on\n", __func__);
  }

  return ret;
}

int
pal_get_device_power(uint8_t slot_id, uint8_t dev_id, uint8_t *status, uint8_t *type) {
  gpio_value_t val = 0;
  int ret = 0;
  uint8_t chassis_type = 0;
  
  if ((status == NULL) || (type == NULL)) {
    syslog(LOG_WARNING, "%s(): Failed to get device power due to parameter is NULL", __func__);
    return POWER_STATUS_ERR;
  }
  
  ret = fbgc_common_get_chassis_type(&chassis_type);
  if ((ret < 0) || (chassis_type != CHASSIS_TYPE5)) {
    syslog(LOG_WARNING, "%s(): Only Type5 support E1S", __func__);
    return POWER_STATUS_ERR;
  }
  
  if (pal_get_server_12v_power(slot_id, status) < 0) {
    return POWER_STATUS_ERR;
  }
  
  if ((*status) == SERVER_12V_OFF) {
    *status = DEVICE_POWER_OFF;
    syslog(LOG_WARNING, "%s(): device power is off due to server is 12V-off", __func__);
    return 0;
  }
  
  if (dev_id == DEV_ID0_E1S) {
    val = gpio_get_value_by_shadow(fbgc_get_gpio_name(GPIO_E1S_1_3V3EFUSE_PGOOD));
  } else if (dev_id == DEV_ID1_E1S) {
    val = gpio_get_value_by_shadow(fbgc_get_gpio_name(GPIO_E1S_2_3V3EFUSE_PGOOD));
  } else {
    syslog(LOG_WARNING, "%s(): wrong device id: %d", __func__, dev_id);
    return POWER_STATUS_ERR;
  }
  
  *status = val;
  
  return 0;
}

int
pal_set_device_power(uint8_t slot_id, uint8_t dev_id, uint8_t cmd) {
  uint8_t e1s_pwr_status = 0, type = 0;
  int ret = 0;
  uint8_t chassis_type = 0, server_power_status = 0;
  
  ret = fbgc_common_get_chassis_type(&chassis_type);
  if ((ret < 0) || (chassis_type != CHASSIS_TYPE5)) {
    syslog(LOG_WARNING, "%s(): Only Type5 support control E1.S power", __func__);
    return -1;
  }
  
  ret = pal_get_server_12v_power(slot_id, &server_power_status);
  if ((ret < 0) || (server_power_status == SERVER_12V_OFF)) {
    syslog(LOG_WARNING, "%s(): Failed to control E1.S power due to server is 12V-off", __func__);
    return POWER_STATUS_ERR;
  }
  
  if (pal_get_device_power(slot_id, dev_id, &e1s_pwr_status, &type) < 0) {
    return -1;
  }
  
  switch(cmd) {
    case SERVER_POWER_ON:
      if (e1s_pwr_status == DEVICE_POWER_ON) {
        return POWER_STATUS_ALREADY_OK;
      }
      ret = pal_set_dev_power_status(dev_id, DEVICE_POWER_ON);
      break;
    case SERVER_POWER_OFF:
      if (e1s_pwr_status == DEVICE_POWER_OFF) {
        return POWER_STATUS_ALREADY_OK;
      }
      ret = pal_set_dev_power_status(dev_id, DEVICE_POWER_OFF);
      break;
    default:
      ret = -1;
  }
  
  return ret;
}

int
pal_set_dev_power_status(uint8_t dev_id, uint8_t cmd) {
  int i2cfd, ret = 0;
  i2c_master_rw_command command;
  
  memset(&command, 0, sizeof(command));
  
  if (dev_id == DEV_ID0_E1S) {
    command.offset = BS_FPGA_E1S0_POWER_CTRL;
  } else if (dev_id == DEV_ID1_E1S) {
    command.offset = BS_FPGA_E1S1_POWER_CTRL;
  } else {
    syslog(LOG_ERR, "Fail to control E1.S power due to wrong device id: %d", dev_id);
    return -1;
  }
  
  if (cmd == DEVICE_POWER_ON) {
    command.val = E1S_POWER_ADD;
  } else if (cmd == DEVICE_POWER_OFF){
    command.val = E1S_POWER_REMOVE;
  } else {
    syslog(LOG_ERR, "Fail to control E1.S power due to wrong action: %d", cmd);
    return -1;
  }
  
  i2cfd = i2c_cdev_slave_open(I2C_BS_FPGA_BUS, BS_FPGA_SLAVE_ADDR >> 1, I2C_SLAVE_FORCE_CLAIM);
  if (i2cfd < 0) {
    syslog(LOG_ERR, "Fail to control E1.S power due to I2C BUS: %d open failed", I2C_BS_FPGA_BUS);
    return i2cfd;
  }
    
  ret = i2c_rdwr_msg_transfer(i2cfd, BS_FPGA_SLAVE_ADDR, (uint8_t *)&command, sizeof(command), NULL, 0);
  close(i2cfd);
  return ret;
}

int
pal_check_server_power_change_correct(uint8_t action) {
  int times = 0, ret = 0;
  uint8_t status = 0;
  
  while (times < WAIT_POWER_STATUS_CHANGE_TIME) {
    ret = pal_get_server_power(FRU_SERVER, &status);
    
    if (ret == 0) {
      if ((action == SET_DC_POWER_ON) && (status == STAT_DC_ON)) {
        return ret;
      } else if ((action == SET_DC_POWER_OFF) && (status == STAT_DC_OFF)) {
        return ret;
      } else if ((action == SET_GRACEFUL_POWER_OFF) && (status == STAT_DC_OFF)) {
        return ret;
      }
    }
    
    times++;
    sleep(1);
  }

  syslog(LOG_ERR, "%s(): power state doesn't change correctly.\n", __func__);

  return -1;

}
