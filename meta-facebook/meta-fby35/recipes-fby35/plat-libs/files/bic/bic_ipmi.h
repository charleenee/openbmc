/*
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
 *
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

#ifndef __BIC_IPMI_H__
#define __BIC_IPMI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <openbmc/ipmi.h>

enum {
  DEV_TYPE_UNKNOWN,
  DEV_TYPE_M2,
  DEV_TYPE_SSD,
  DEV_TYPE_BRCM_ACC,
  DEV_TYPE_SPH_ACC,
  DEV_TYPE_DUAL_M2,
};

enum {
  FFI_STORAGE,
  FFI_ACCELERATOR,
};

enum {
  VENDOR_SAMSUNG = 0x144D,
  VENDOR_VSI = 0x1D9B,
  VENDOR_BRCM = 0x14E4,
  VENDOR_SPH = 0x8086,
};

<<<<<<< HEAD
=======
enum {
  STANDARD_CMD = 0,
  ACCURATE_CMD = 1,
  UNKNOWN_CMD = 2,
};

typedef struct
{
  uint8_t iana_id[3];
  uint8_t val_msb;
  uint8_t val_lsb;
  uint8_t flags;
} ipmi_accurate_sensor_reading_t;

typedef struct
{
  uint16_t value;
  uint8_t flags;
  uint8_t status;
  uint8_t ext_status;
  uint8_t read_type;
} snr_reading_ret;


>>>>>>> facebook/helium
#define MAX_READ_RETRY 5

int bic_get_dev_id(uint8_t slot_id, ipmi_dev_id_t *dev_id, uint8_t intf);
int bic_get_self_test_result(uint8_t slot_id, uint8_t *self_test_result, uint8_t intf);
int bic_get_fruid_info(uint8_t slot_id, uint8_t fru_id, ipmi_fruid_info_t *info, uint8_t intf);
int bic_read_fruid(uint8_t slot_id, uint8_t fru_id, const char *path, int *fru_size, uint8_t intf);
int bic_write_fruid(uint8_t slot_id, uint8_t fru_id, const char *path, uint8_t intf);
int bic_get_sdr(uint8_t slot_id, ipmi_sel_sdr_req_t *req, ipmi_sel_sdr_res_t *res, uint8_t *rlen, uint8_t intf);
int bic_get_fw_ver(uint8_t slot_id, uint8_t comp, uint8_t *ver);
int bic_get_1ou_type(uint8_t slot_id, uint8_t *type);
int bic_get_1ou_type_cache(uint8_t slot_id, uint8_t *type);
int bic_set_amber_led(uint8_t slot_id, uint8_t dev_id, uint8_t status);
int bic_get_80port_record(uint8_t slot_id, uint8_t *rbuf, uint8_t *rlen, uint8_t intf);
int bic_get_cpld_ver(uint8_t slot_id, uint8_t comp, uint8_t *ver, uint8_t bus, uint8_t addr, uint8_t intf);
<<<<<<< HEAD
int bic_get_vr_device_id(uint8_t slot_id, uint8_t comp, uint8_t *rbuf, uint8_t *rlen, uint8_t bus, uint8_t addr, uint8_t intf);
int bic_get_vr_ver(uint8_t slot_id, uint8_t intf, uint8_t bus, uint8_t addr, char *key, char *ver_str);
int bic_get_vr_ver_cache(uint8_t slot_id, uint8_t intf, uint8_t bus, uint8_t addr, char *ver_str);
int bic_get_exp_cpld_ver(uint8_t slot_id, uint8_t comp, uint8_t *ver, uint8_t bus, uint8_t addr, uint8_t intf);
int bic_get_sensor_reading(uint8_t slot_id, uint8_t sensor_num, ipmi_sensor_reading_t *sensor, uint8_t intf);
=======
int bic_get_vr_device_id(uint8_t slot_id, uint8_t *devid, uint8_t *id_len, uint8_t bus, uint8_t addr, uint8_t intf);
int bic_get_vr_ver(uint8_t slot_id, uint8_t intf, uint8_t bus, uint8_t addr, char *key, char *ver_str);
int bic_get_vr_ver_cache(uint8_t slot_id, uint8_t intf, uint8_t bus, uint8_t addr, char *ver_str);
int bic_get_exp_cpld_ver(uint8_t slot_id, uint8_t comp, uint8_t *ver, uint8_t bus, uint8_t addr, uint8_t intf);
int bic_get_sensor_reading(uint8_t slot_id, uint8_t sensor_num, snr_reading_ret *sensor, uint8_t intf);
>>>>>>> facebook/helium
int bic_is_m2_exp_prsnt(uint8_t slot_id);
int me_recovery(uint8_t slot_id, uint8_t command);
int me_reset(uint8_t slot_id);
int bic_switch_mux_for_bios_spi(uint8_t slot_id, uint8_t mux);
int bic_set_gpio(uint8_t slot_id, uint8_t gpio_num,uint8_t value);
int remote_bic_set_gpio(uint8_t slot_id, uint8_t gpio_num,uint8_t value, uint8_t intf);
int bic_get_one_gpio_status(uint8_t slot_id, uint8_t gpio_num, uint8_t *value);
int bic_asd_init(uint8_t slot_id, uint8_t cmd);
int bic_get_gpio_config(uint8_t slot_id, uint8_t gpio, uint8_t *data);
int bic_set_gpio_config(uint8_t slot_id, uint8_t gpio, uint8_t data);
int bic_get_sys_guid(uint8_t slot_id, uint8_t *guid);
int bic_set_sys_guid(uint8_t slot_id, uint8_t *guid);
int bic_do_sled_cycle(uint8_t slot_id);
int bic_set_fan_speed(uint8_t fan_id, uint8_t pwm);
int bic_manual_set_fan_speed(uint8_t fan_id, uint8_t pwm);
int bic_get_fan_speed(uint8_t fan_id, float *value);
int bic_get_fan_pwm(uint8_t fan_id, float *value);
int bic_do_12V_cycle(uint8_t slot_id);
int bic_get_dev_info(uint8_t slot_id, uint8_t dev_id, uint8_t *nvme_ready, uint8_t *status, uint8_t *type);
int bic_get_dev_power_status(uint8_t slot_id, uint8_t dev_id, uint8_t *nvme_ready, uint8_t *status, \
                             uint8_t *ffi, uint8_t *meff, uint16_t *vendor_id, uint8_t *major_ver, uint8_t *minor_ver, uint8_t intf);
int bic_set_dev_power_status(uint8_t slot_id, uint8_t dev_id, uint8_t status, uint8_t intf);
<<<<<<< HEAD
int bic_get_ifx_vr_remaining_writes(uint8_t slot_id, uint8_t bus, uint8_t addr, uint8_t *writes, uint8_t intf);
int bic_get_isl_vr_remaining_writes(uint8_t slot_id, uint8_t bus, uint8_t addr, uint8_t *writes, uint8_t intf);
int bic_reset(uint8_t slot_id);
int bic_clear_cmos(uint8_t slot_id);
=======
int bic_ifx_vr_mfr_fw(uint8_t slot_id, uint8_t bus, uint8_t addr, uint8_t code, uint8_t *data, uint8_t *resp, uint8_t intf);
int bic_get_ifx_vr_remaining_writes(uint8_t slot_id, uint8_t bus, uint8_t addr, uint8_t *writes, uint8_t intf);
int bic_get_isl_vr_remaining_writes(uint8_t slot_id, uint8_t bus, uint8_t addr, uint8_t *writes, uint8_t intf);
int bic_disable_sensor_monitor(uint8_t slot_id, uint8_t dis, uint8_t intf);
int bic_reset(uint8_t slot_id);
>>>>>>> facebook/helium
int bic_inform_sled_cycle(void);
int bic_enable_ssd_sensor_monitor(uint8_t slot_id, bool enable, uint8_t intf);
uint8_t get_gpv3_bus_number(uint8_t dev_id);
uint8_t get_gpv3_channel_number(uint8_t dev_id);
int bic_notify_fan_mode(int mode);
int bic_get_dp_pcie_config(uint8_t slot_id, uint8_t *pcie_config);
int bic_set_bb_fw_update_ongoing(uint8_t component, uint8_t option);
int bic_check_bb_fw_update_ongoing();
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __BIC_IPMI_H__ */
