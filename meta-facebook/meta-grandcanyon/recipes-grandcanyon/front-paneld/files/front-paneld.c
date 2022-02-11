/*
 *
 * Copyright 2020-present Facebook. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <sys/time.h>
#include <openbmc/kv.h>
#include <openbmc/pal.h>
#include <openbmc/kv.h>
#include <facebook/fbgc_common.h>
#include <facebook/fbgc_gpio.h>

#define LED_INTERVAL_DEFAULT              500 //millisecond
#define MONITOR_FRU_HEALTH_INTERVAL       1 //second
#define SYNC_SYSTEM_STATUS_LED_INTERVAL   1 //second
#define DBG_CARD_SHOW_ERR_INTERVAL        1 //second
#define DBG_CARD_UPDATE_ERR_INTERVAL      5 //second
#define MONITOR_HB_HEALTH_INTERVAL        5 //second
#define SYNC_E1S_STATUS_LED_INTERVAL      1 //second

#define MAX_NUM_CHECK_FRU_HEALTH          5

#define HEARTBEAT_TIMEOUT                 180 // second = 3 mins
#define MAX_NUM_CHECK_HB_HEALTH           4

// Thread to handle LED state of the SLED
static void *
led_sync_handler() {
  int ret = 0, ret2 = 0;
  char identify[MAX_VALUE_LEN] = {0};
  char interval[MAX_VALUE_LEN] = {0};

  // set flag to notice BMC front-paneld led_sync_handler is ready
  kv_set("flag_front_led_sync", STR_VALUE_1, 0, 0);

  while (1) {
    // Handle Slot IDENTIFY condition
    memset(identify, 0x0, sizeof(identify));
    memset(interval, 0x0, sizeof(interval));
    
    ret = pal_get_key_value("system_identify_server", identify);
    ret2 = pal_get_key_value("system_identify_led_interval", interval);
    
    if ((ret == 0) && (strcmp(identify, "on") == 0)) {
      // Start blinking the ID LED
      pal_set_id_led(FRU_UIC, LED_ON);
      
      if ((ret2 == 0) && (strcmp(interval, "default") == 0)) {
        msleep(LED_INTERVAL_DEFAULT);
      } else if (ret2 == 0) {
        sleep(atoi(interval));
      }
      
      pal_set_id_led(FRU_UIC, LED_OFF);

      if ((ret2 == 0) && (strcmp(interval, "default") == 0)) {
        msleep(LED_INTERVAL_DEFAULT);
      } else if (ret2 == 0) {
        sleep(atoi(interval));
      }
      continue;
    } else if ((ret == 0) && (strcmp(identify, "off") == 0)) {
      pal_set_id_led(FRU_UIC, LED_ON);
    }
    sleep(1);
  }

  return NULL;
}

// Thread to handle different case of system status LED
// Define on OpenBMC spec. "3.2.4 Status/Fault LED"
static void *
system_status_led_handler() {
  int ret = 0, blink_count = 1, i = 0;
  uint8_t server_power_status = SERVER_12V_OFF;
  uint8_t error[MAX_NUM_ERR_CODES] = {0}, error_count = 0;
  bool is_bmc_fault = false;
  char value[MAX_VALUE_LEN] = {0};

  // set flag to notice BMC front-paneld system_status_led_handler is ready
  kv_set("flag_front_sys_status_led", STR_VALUE_1, 0, 0);

  while(1) {
    // Get flag to check if status LED is setting by fpc-util
    ret = kv_get("flag_fpc_status", value, NULL, 0);
    if ((ret == 0) && (strcmp(value, STR_VALUE_1) == 0)) {
      sleep(SYNC_SYSTEM_STATUS_LED_INTERVAL);
      continue;
    }

    ret = pal_get_server_power(FRU_SERVER, &server_power_status);
    if (ret < 0) {
      //if can't get server power status, keep system status LED solid yellow
      syslog(LOG_WARNING, "%s(): failed to get server power status", __func__);
      
      ret = pal_set_status_led(FRU_UIC, STATUS_LED_YELLOW);
      if (ret < 0) {
        syslog(LOG_WARNING, "%s(): failed to set server status LED to solid yellow", __func__);
      }
      
    } else {
      ret = pal_get_error_code(error, &error_count);
      
      // When server power on
      if (server_power_status == SERVER_POWER_ON) {
        if ((error_count == 0) && (ret == 0)) {
          // Solid Blue: BMC, server, and Expander have no fault
          ret = pal_set_status_led(FRU_UIC, STATUS_LED_BLUE);
          if (ret < 0) {
            syslog(LOG_WARNING, "%s(): failed to set server status LED to solid blue", __func__);
          }
        } else {
          // Solid Yellow: BMC, server, or Expander have fault
          ret = pal_set_status_led(FRU_UIC, STATUS_LED_YELLOW);
          if (ret < 0) {
            syslog(LOG_WARNING, "%s(): failed to set server status LED to solid yellow", __func__);
          }
        }
      
      // When server power off: only check BMC has error or not
      } else {
        // Check BMC fault
        for (i = 0; i < error_count; i++) {
          if (error[i] >= MAX_NUM_EXP_ERR_CODES ) {
            is_bmc_fault = true;
            break;
          } else {
            is_bmc_fault = false;
          }
        }
        
        // Blinking Yellow: BMC have no fault
        if ((is_bmc_fault == false) && (ret == 0)){
          if (blink_count > 0) {
            ret = pal_set_status_led(FRU_UIC, STATUS_LED_YELLOW);
          } else {
            ret = pal_set_status_led(FRU_UIC, STATUS_LED_OFF);
          }
          blink_count = blink_count * -1;
        
        // Solid Yellow: BMC have fault
        } else {
          ret = pal_set_status_led(FRU_UIC, STATUS_LED_YELLOW);
          if (ret < 0) {
            syslog(LOG_WARNING, "%s(): failed to set server status LED to solid yellow", __func__);
          }
        }
      }
    }
    
    sleep(SYNC_SYSTEM_STATUS_LED_INTERVAL);
  } // end while loop 
  
  return NULL;
}

// Thread to handle fru health
static void *
fru_health_handler() {
  uint8_t health = FRU_STATUS_GOOD;
  uint8_t fru_list[MAX_NUM_CHECK_FRU_HEALTH] = {FRU_SERVER, FRU_UIC, FRU_DPB, FRU_SCC, FRU_NIC};
  uint8_t health_error_code_list[MAX_NUM_CHECK_FRU_HEALTH] = {
       ERR_CODE_SERVER_HEALTH, ERR_CODE_UIC_HEALTH, ERR_CODE_DPB_HEALTH, 
       ERR_CODE_SCC_HEALTH, ERR_CODE_NIC_HEALTH};
  int i = 0, ret = 0;

  // set flag to notice BMC front-paneld fru_health_handler is ready
  kv_set("flag_front_health", STR_VALUE_1, 0, 0);

  while(1) {
    // fru health
    for (i = 0; i < sizeof(fru_list); i++) {
      ret = pal_get_fru_health(fru_list[i], &health);
      if (ret < 0) {
        health = FRU_STATUS_BAD;
      }
      
      if (health == FRU_STATUS_GOOD) {
        pal_set_error_code(health_error_code_list[i], ERR_CODE_DISABLE);
      } else if (health == FRU_STATUS_BAD) {
        pal_set_error_code(health_error_code_list[i], ERR_CODE_ENABLE);
      }
    } // end for loop
    sleep(MONITOR_FRU_HEALTH_INTERVAL);
  } // end while loop 
  
  return NULL;
}

// Thread to show error code on debug card
static void *
dbg_card_show_error_code() {
  uint8_t dbg_present = 0;
  uint8_t uart_sel = 0;
  uint8_t error[MAX_NUM_ERR_CODES] = {0}, error_code = 0;
  uint8_t cur_error_count = 0, pre_error_count = 0;
  int ret = 0;
  int poll_error_timer = 0, error_index = 0;
  
  // set flag to notice BMC front-paneld dbg_card_show_error_code is ready
  kv_set("flag_front_err_code", STR_VALUE_1, 0, 0);

  while(1) {
    ret = pal_is_debug_card_present(&dbg_present);
    if ((ret == 0) && (dbg_present == FRU_PRESENT)) {
      
      ret = pal_get_debug_card_uart_sel(&uart_sel);
      if ((ret == 0) && (uart_sel == DEBUG_UART_SEL_BMC)) {
        
        // update error code
        if (poll_error_timer == 0) {
          memset(error, 0, sizeof(error));
          ret = pal_get_error_code(error, &cur_error_count);
        }
        
        if (cur_error_count == 0) {
          error_code = 0;

        } else {
          // if the new error count is smaller, showing the error code start from scratch
          if (cur_error_count < pre_error_count) {
            error_index = 0;
          }
          pre_error_count = cur_error_count;
          
          error_code = error[error_index];
          // Expander error code (1~100) need to change
          // ex: decimal 99 change to hexadecimal 0x99
          if (error_code < MAX_NUM_EXP_ERR_CODES) {
            error_code = error_code/10*16+error_code%10;
          }
        }
        
        // show error code on debug card
        pal_post_display(error_code);
        
        error_index++;
        if (error_index >= cur_error_count) {
          error_index = 0;
        }
        
        poll_error_timer++;
        if (poll_error_timer > DBG_CARD_UPDATE_ERR_INTERVAL) {
          poll_error_timer = 0;
        }

      }
    }

    sleep(DBG_CARD_SHOW_ERR_INTERVAL);
  } // while loop end

  pthread_exit(NULL);
}

// Thread to handle heartbeat health
static void *
heartbeat_health_handler() {
  // 0: BMC remote hb  1: scc local hb  2: scc remote hb
  uint8_t hb_list[MAX_NUM_CHECK_HB_HEALTH] = {HEARTBEAT_REMOTE_BMC, HEARTBEAT_LOCAL_SCC, HEARTBEAT_REMOTE_SCC, HEARTBEAT_BIC};
  uint8_t cur_hb_status_list[MAX_NUM_CHECK_HB_HEALTH] = {HEARTBEAT_NORMAL, HEARTBEAT_NORMAL, HEARTBEAT_NORMAL, HEARTBEAT_NORMAL};
  uint8_t pre_hb_status_list[MAX_NUM_CHECK_HB_HEALTH] = {HEARTBEAT_NORMAL, HEARTBEAT_NORMAL, HEARTBEAT_NORMAL, HEARTBEAT_NORMAL};
  int hb_timer[MAX_NUM_CHECK_HB_HEALTH] = {0, 0, 0, 0};
  uint8_t hb_error_code_list[MAX_NUM_CHECK_HB_HEALTH] = {
    ERR_CODE_BMC_REMOTE_HB_HEALTH, ERR_CODE_SCC_LOCAL_HB_HEALTH, ERR_CODE_SCC_REMOTE_HB_HEALTH, ERR_CODE_BIC_HB_HEALTH};
  char* log_desc_list[MAX_NUM_CHECK_HB_HEALTH] = {"BMC remote", "SCC local", "SCC remote", "BIC"};
  uint8_t chassis_type = 0;
  char val[MAX_VALUE_LEN] = {0};
  int ret = 0, i = 0;
  int hb_health_kv_state = HEARTBEAT_NORMAL, hb_health_last_state = HEARTBEAT_NORMAL;
  
  // set flag to notice BMC front-paneld heartbeat_health_handler is ready
  kv_set("flag_front_hb", STR_VALUE_1, 0, 0);

  while(1) {
    // Get kv value
    memset(val, 0, sizeof(val));
    ret = pal_get_key_value("heartbeat_health", val);
    if (ret < 0) {
      syslog(LOG_ERR, "%s(): fail to get key: heartbeat_health value", __func__);
    }
    hb_health_kv_state = atoi(val);
  
    ret = fbgc_common_get_chassis_type(&chassis_type);
    if (ret < 0) {
      syslog(LOG_WARNING, "%s(): failed to get chassis type", __func__);
    }

    for (i = 0; i < MAX_NUM_CHECK_HB_HEALTH; i++) {
      // Type 5 detect: BMC_RMT, SCC_LOC, BIC
      // Type 7 detect: SCC_LOC, SCC_RMT, BIC
      // Unknown type detect: SCC_LOC, BIC
      if (((ret == 0) && (chassis_type == CHASSIS_TYPE5) && (hb_list[i] != HEARTBEAT_REMOTE_SCC))
         || ((ret == 0) && (chassis_type == CHASSIS_TYPE7) && (hb_list[i] != HEARTBEAT_REMOTE_BMC))
         || (hb_list[i] == HEARTBEAT_LOCAL_SCC)
         || (hb_list[i] == HEARTBEAT_BIC)) {
         
        // Get and check HB value
        if (pal_is_heartbeat_ok(hb_list[i]) == true) {
          hb_timer[i] = 0;
        } else {
          hb_timer[i] += MONITOR_HB_HEALTH_INTERVAL;
        }
        
        // Timeout detect: no response continuous 3 mins
        if (hb_timer[i] > HEARTBEAT_TIMEOUT) {
          hb_timer[i] = HEARTBEAT_TIMEOUT;
          cur_hb_status_list[i] = HEARTBEAT_ABNORMAL;
        } else {
          cur_hb_status_list[i] = HEARTBEAT_NORMAL;
        }
        
        // Status chagne: set error code enable/disable and key value 
        if ((cur_hb_status_list[i] == HEARTBEAT_ABNORMAL) && (pre_hb_status_list[i] == HEARTBEAT_NORMAL)) {
          syslog(LOG_CRIT, "%s heartbeat is abnormal", log_desc_list[i]);
          
          pal_set_error_code(hb_error_code_list[i], ERR_CODE_ENABLE);
          
          memset(val, 0, sizeof(val));
          snprintf(val, sizeof(val), "%d", HEARTBEAT_ABNORMAL);
          ret = pal_set_key_value("heartbeat_health", val);
          if (ret < 0) {
            syslog(LOG_ERR, "%s(): %s abnormal and fail to set key: heartbeat_health value: %s", __func__, log_desc_list[i], val);
          }
          
        } else if ((cur_hb_status_list[i] == HEARTBEAT_NORMAL) && (pre_hb_status_list[i] == HEARTBEAT_ABNORMAL)) {
          pal_set_error_code(hb_error_code_list[i], ERR_CODE_DISABLE);
        }
        
        // Update heartbeat status
        pre_hb_status_list[i] = cur_hb_status_list[i];
      }
    } // for loop end
    
    // if log-util clear all
    // clean heartbeat timer and will regenerate assert
    if ((hb_health_kv_state != hb_health_last_state) && (hb_health_kv_state == HEARTBEAT_NORMAL)) {
      for (i = 0; i < MAX_NUM_CHECK_HB_HEALTH; i++) {
        hb_timer[i] = 0;
      }
    }
    hb_health_last_state = hb_health_kv_state;
    
    sleep(MONITOR_HB_HEALTH_INTERVAL);
  } // while loop end
  
  return NULL;
}

static void *
e1s_led_sync_handler() {
  char val[MAX_VALUE_LEN] = {0};
  int ret = 0, i = 0;
  char *key_list[E1S_IOCM_SLOT_NUM] = {"e1s0_led_status", "e1s1_led_status"};
  e1s_led_id id_list[E1S_IOCM_SLOT_NUM] = {ID_E1S0_LED, ID_E1S1_LED};
  enum LED_HIGH_ACTIVE blinking_list[E1S_IOCM_SLOT_NUM] = {LED_ON, LED_ON};
  
  // set default
  kv_set(key_list[0], "off", 0, 0);
  kv_set(key_list[1], "off", 0, 0);
  
  while(1) {
    for (i = 0; i < E1S_IOCM_SLOT_NUM; i++) {
      if (kv_get(key_list[i], val, NULL, 0) == 0) {
        if (strcmp(val, "on") == 0) {
          ret = pal_set_e1s_led(FRU_E1S_IOCM, id_list[i], LED_ON);
          
        } else if (strcmp(val, "off") == 0) {
          ret = pal_set_e1s_led(FRU_E1S_IOCM, id_list[i], LED_OFF);
          
        } else if (strcmp(val, "blinking") == 0) {
          ret = pal_set_e1s_led(FRU_E1S_IOCM, id_list[i], blinking_list[i]);
          
          if (blinking_list[i] == LED_ON) {
            blinking_list[i] = LED_OFF;
          } else if (blinking_list[i] == LED_OFF) {
            blinking_list[i] = LED_ON;
          }
        }
        
        if (ret < 0) {
          syslog(LOG_ERR, "%s(): Failed to set E1.S%d LED status due to pal_set_e1s_led() failed", __func__, i);
        }
        
      } else {
        syslog(LOG_ERR, "%s(): Failed to get %s due to kv_get() failed", __func__, key_list[i]);
      }
    } // for loop end
    
    sleep(SYNC_E1S_STATUS_LED_INTERVAL);
  } // while loop end
  
  pthread_exit(NULL);
}

int
main (int argc, char * const argv[]) {
  pthread_t tid_sync_led = 0;
  pthread_t tid_monitor_fru_health = 0;
  pthread_t tid_sync_system_status_led = 0;
  pthread_t tid_dbg_card_error_code = 0;
  pthread_t tid_monitor_heartbeat_health = 0;
  pthread_t tid_sync_e1s_led = 0;
  int ret = 0, pid_file = 0;
  int ret_sync_led = 0, ret_monitor_fru = 0, ret_sync_system_status_led = 0;
  int ret_dbg_card_error_code = 0;
  int ret_heartbeat = 0;
  int ret_sync_e1s_led = 0;
  uint8_t chassis_type = 0;

  pid_file = open("/var/run/front-paneld.pid", O_CREAT | O_RDWR, 0666);
  if (pid_file < 0) {
    syslog(LOG_WARNING, "Fail to open front-paneld.pid file\n");
    return -1;
  }

  ret = flock(pid_file, LOCK_EX | LOCK_NB);
  if (ret != 0) {
    if(EWOULDBLOCK == errno) {
      syslog(LOG_WARNING, "Another front-paneld instance is running...\n");
      ret = -1;
      goto err;
    }
  } else {
    openlog("front-paneld", LOG_CONS, LOG_DAEMON);
  }

  if (pthread_create(&tid_sync_led, NULL, led_sync_handler, NULL) < 0) {
    syslog(LOG_WARNING, "pthread_create for led sync error\n");
    ret_sync_led = -1;
  }

  if (pthread_create(&tid_monitor_fru_health, NULL, fru_health_handler, NULL) < 0) {
    syslog(LOG_WARNING, "fail to create thread to monitor fru health\n");
    ret_monitor_fru = -1;
  }

  if (pthread_create(&tid_sync_system_status_led, NULL, system_status_led_handler, NULL) < 0) {
    syslog(LOG_WARNING, "fail to create thread to sync system status LED\n");
    ret_sync_system_status_led = -1;
  }
  
  if (pthread_create(&tid_dbg_card_error_code, NULL, dbg_card_show_error_code, NULL) < 0) {
    syslog(LOG_WARNING, "fail to creat thread for show error code on debug card\n");
    ret_dbg_card_error_code = -1;
  }

  if (pthread_create(&tid_monitor_heartbeat_health, NULL, heartbeat_health_handler, NULL) < 0) {
    syslog(LOG_WARNING, "fail to create thread to monitor heartbeat health\n");
    ret_heartbeat = -1;
  }
  
  ret = fbgc_common_get_chassis_type(&chassis_type);
  if ((ret == 0) && (chassis_type == CHASSIS_TYPE5)) {
    if (pthread_create(&tid_sync_e1s_led, NULL, e1s_led_sync_handler, NULL) < 0) {
      syslog(LOG_WARNING, "fail to create thread to sync E1.S status LED\n");
      ret_sync_e1s_led = -1;
    }
  } else {
    ret_sync_e1s_led = -1;
  }
    
  if (ret_sync_led == 0) {
    pthread_join(tid_sync_led, NULL);
  }
  if (ret_monitor_fru == 0) {
    pthread_join(tid_monitor_fru_health, NULL);
  }
  if (ret_sync_system_status_led == 0) {
    pthread_join(tid_sync_system_status_led, NULL);
  }
  if (ret_dbg_card_error_code == 0) {
    pthread_join(tid_dbg_card_error_code, NULL);
  }
  if (ret_heartbeat == 0) {
    pthread_join(tid_monitor_heartbeat_health, NULL);
  }
  if (ret_sync_e1s_led == 0) {
    pthread_join(tid_sync_e1s_led, NULL);
  }

err:
  flock(pid_file, LOCK_UN);
  close(pid_file);
  return ret;
}
