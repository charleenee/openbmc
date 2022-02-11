#include <iostream>
#include <cstdio>
#include <cstring>
#include <openbmc/pal.h>
#include "expansion.h"
#ifdef BIC_SUPPORT
#include <facebook/bic.h>

using namespace std;

void ExpansionBoard::ready()
{
  uint8_t type_2ou = 0xff;
  uint8_t config_status = 0;
  bool is_present = true;
  int ret = 0;
  int exp_present = 0;

  switch(fw_comp) {
    case FW_CPLD:
    case FW_BIC:
    case FW_BIC_BOOTLOADER:
    case FW_BB_BIC:
    case FW_BB_BIC_BOOTLOADER:
    case FW_BB_CPLD:
      return;
  }

  //we check the present status of GPv3 board by using one of devices
  if ( fw_comp >= FW_2OU_M2_DEV0 && fw_comp <= FW_2OU_M2_DEV11 ) {
    fw_comp = FW_2OU_M2_DEV0;
  } else if ( fw_comp >= FW_TOP_M2_DEV0 && fw_comp <= FW_TOP_M2_DEV11 ) {
    fw_comp = FW_TOP_M2_DEV0;
  } else if ( fw_comp >= FW_BOT_M2_DEV0 && fw_comp <= FW_BOT_M2_DEV11 ) {
    fw_comp = FW_BOT_M2_DEV0;
  }

  ret = bic_is_m2_exp_prsnt(slot_id);
  if ( ret < 0 ) {
    throw "Failed to get the config from " + fru + ":" + board_name;
  }

  config_status = (uint8_t) ret;
  switch (fw_comp) {
    case FW_1OU_BIC:
    case FW_1OU_BIC_BOOTLOADER:
      if ( (config_status & PRESENT_1OU) != PRESENT_1OU )
        is_present = false;
      break;
    case FW_1OU_CPLD:
      if ( (config_status & PRESENT_1OU) != PRESENT_1OU ) {
        is_present = false;
      } else {
        uint8_t type = 0xff;
        if (bic_get_1ou_type(slot_id, &type)){
          throw string("Failed to get 1OU board type");
        } else if (type == EDSFF_1U) {
          throw string("Not present");
        }
      }
      break;
    case FW_2OU_BIC:
    case FW_2OU_BIC_BOOTLOADER:
    case FW_2OU_CPLD:
      if ( fby3_common_get_2ou_board_type(slot_id, &type_2ou) < 0 ) {
        throw string("Failed to get 2OU board type");
      }
      if ( (config_status & PRESENT_2OU) != PRESENT_2OU || type_2ou == DP_RISER_BOARD)
        is_present = false;
      break;
    case FW_2OU_PESW:
    case FW_2OU_PESW_VR:
    case FW_2OU_3V3_VR1:
    case FW_2OU_3V3_VR2:
    case FW_2OU_3V3_VR3:
    case FW_2OU_1V8_VR:
    case FW_2OU_M2_DEV0:
    case FW_TOP_M2_DEV0:
    case FW_BOT_M2_DEV0:
      if ( (config_status & PRESENT_2OU) != PRESENT_2OU ) {
        is_present = false;
      } else {
        uint8_t type = 0xff;
        if ( fby3_common_get_2ou_board_type(slot_id, &type) < 0 ) {
          throw string("Failed to get 2OU board type");
        } else if ( type != GPV3_MCHP_BOARD && type != GPV3_BRCM_BOARD && type != CWC_MCHP_BOARD ) {
          throw string("Not present");
        }
      }

      // PESW are present when the power is in S0 state
      if ( is_present == true && (fw_comp == FW_2OU_PESW || fw_comp == FW_2OU_M2_DEV0 ||
                                  fw_comp == FW_TOP_M2_DEV0 || fw_comp == FW_BOT_M2_DEV0) ) {
        uint8_t pwr_sts = 0x0;
        ret = pal_get_server_power(slot_id, &pwr_sts);
        if ( ret < 0 || pwr_sts == 0 )
          throw string("DC-off");
      }
      break;
    case FW_CWC_BIC:
    case FW_CWC_BIC_BL:
    case FW_CWC_CPLD:
    case FW_CWC_PESW:
    case FW_GPV3_TOP_BIC:
    case FW_GPV3_TOP_BIC_BL:
    case FW_GPV3_TOP_CPLD:
    case FW_GPV3_TOP_PESW:
    case FW_GPV3_BOT_BIC:
    case FW_GPV3_BOT_BIC_BL:
    case FW_GPV3_BOT_CPLD:
    case FW_GPV3_BOT_PESW:
    case FW_CWC_PESW_VR:
    case FW_GPV3_TOP_PESW_VR:
    case FW_GPV3_BOT_PESW_VR:
      if ( fby3_common_get_2ou_board_type(slot_id, &type_2ou) < 0 ) {
        throw string("Failed to get 2OU board type");
      }
      if ((config_status & PRESENT_2OU) != PRESENT_2OU || type_2ou != CWC_MCHP_BOARD) {
        is_present = false;
      } else {
        exp_present = bic_is_2u_top_bot_prsnt(slot_id);
      }
      break;
    default:
      break;
  }

  if (is_present) {
    switch (fw_comp) {
      case FW_GPV3_TOP_BIC:
      case FW_GPV3_TOP_BIC_BL:
      case FW_GPV3_TOP_CPLD:
      case FW_GPV3_TOP_PESW:
      case FW_GPV3_TOP_PESW_VR:
        if ((exp_present & PRESENT_2U_TOP) == 0) {
          throw board_name + " is empty";
        }
        break;
      case FW_GPV3_BOT_BIC:
      case FW_GPV3_BOT_BIC_BL:
      case FW_GPV3_BOT_CPLD:
      case FW_GPV3_BOT_PESW:
      case FW_GPV3_BOT_PESW_VR:
        if ((exp_present & PRESENT_2U_BOT) == 0) {
          throw board_name + " is empty";
        }
        break;
    }
  }

  if ( is_present == false )
    throw board_name + " is empty";
}
#endif
