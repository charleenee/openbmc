#!/bin/sh
#
# Copyright 2014-present Facebook. All Rights Reserved.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA

### BEGIN INIT INFO
# Provides:          power-on
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description: Power on Server
### END INIT INFO
<<<<<<< HEAD
. /usr/local/fbpackages/utils/ast-functions

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

KEYDIR=/mnt/data/kv_store
=======

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

# shellcheck disable=SC1091
. /usr/local/fbpackages/utils/ast-functions

>>>>>>> facebook/helium
DEF_PWR_ON=1
TO_PWR_ON=

check_por_config() {
<<<<<<< HEAD
  TO_PWR_ON=-1

  # Check if the file/key doesn't exist
  if [ ! -f "${KEYDIR}/slot${1}_por_cfg" ]; then
    TO_PWR_ON=$DEF_PWR_ON
  else
    POR=`cat ${KEYDIR}/slot${1}_por_cfg`

    if [ $POR == "on" ]; then
      TO_PWR_ON=1;
    elif [ $POR == "lps" ]; then
      # Check if the file/key doesn't exist
      if [ ! -f "${KEYDIR}/pwr_server${1}_last_state" ]; then
        TO_PWR_ON=$DEF_PWR_ON
      else
        LS=`cat ${KEYDIR}/pwr_server${1}_last_state`
        if [ $LS == "on" ]; then
          TO_PWR_ON=1;
        fi
      fi
    fi
  fi
}

function init_class2_server() {
  if [ $(is_bmc_por) -eq 1 ]; then
    devmem_clear_bit $(scu_addr 70) 13

    check_por_config 1
    if [ $TO_PWR_ON -eq 1 ] ; then
=======
  TO_PWR_ON=0

  if POR=$(kv get "slot${1}_por_cfg" persistent); then
    if [ "$POR" = "lps" ]; then
      if LS=$(kv get "pwr_server${1}_last_state" persistent); then
        if [ "$LS" = "on" ]; then
          TO_PWR_ON=1;
        fi
      else
        TO_PWR_ON=$DEF_PWR_ON
      fi
    elif [ "$POR" = "on" ]; then
      TO_PWR_ON=1;
    fi
  else
    TO_PWR_ON=$DEF_PWR_ON
  fi
}

init_class2_server() {
  if [ "$(is_bmc_por)" -eq 1 ]; then
    check_por_config 1
    if [ "$(is_sb_bic_ready 1)" = "1" ] && [ $TO_PWR_ON -eq 1 ]; then
>>>>>>> facebook/helium
      power-util slot1 on
    fi
  fi

  show_sys_config -r > /dev/null
}

<<<<<<< HEAD
function init_class1_server() {
  # Check whether it is fresh power on reset
  if [ $(is_bmc_por) -eq 1 ]; then
    # Disable clearing of PWM block on WDT SoC Reset
    devmem_clear_bit $(scu_addr 70) 13

    check_por_config 1
    if [ $(is_sb_bic_ready 1) == "1" ] && [ $TO_PWR_ON -eq 1 ] ; then
=======
init_class1_server() {
  # Check whether it is fresh power on reset
  if [ "$(is_bmc_por)" -eq 1 ]; then
    # Disable clearing of PWM block on WDT SoC Reset
    devmem_clear_bit "$(scu_addr 70)" 13

    check_por_config 1
    if [ "$(is_sb_bic_ready 1)" = "1" ] && [ $TO_PWR_ON -eq 1 ]; then
>>>>>>> facebook/helium
      power-util slot1 on
    fi

    check_por_config 2
<<<<<<< HEAD
    if [ $(is_sb_bic_ready 2) == "1" ] && [ $TO_PWR_ON -eq 1 ] ; then
=======
    if [ "$(is_sb_bic_ready 2)" = "1" ] && [ $TO_PWR_ON -eq 1 ]; then
>>>>>>> facebook/helium
      power-util slot2 on
    fi

    check_por_config 3
<<<<<<< HEAD
    if [ $(is_sb_bic_ready 3) == "1" ] && [ $TO_PWR_ON -eq 1 ] ; then
=======
    if [ "$(is_sb_bic_ready 3)" = "1" ] && [ $TO_PWR_ON -eq 1 ]; then
>>>>>>> facebook/helium
      power-util slot3 on
    fi

    check_por_config 4
<<<<<<< HEAD
    if [ $(is_sb_bic_ready 4) == "1" ] && [ $TO_PWR_ON -eq 1 ] ; then
=======
    if [ "$(is_sb_bic_ready 4)" = "1" ] && [ $TO_PWR_ON -eq 1 ]; then
>>>>>>> facebook/helium
      power-util slot4 on
    fi
  fi
}

<<<<<<< HEAD
if ! /usr/sbin/ntpq -p | grep '^\*' > /dev/null ; then
  date -s "2018-01-01 00:00:00"
fi

/usr/local/bin/sync_date.sh

bmc_location=$(get_bmc_board_id)
if [ $bmc_location -eq 9 ]; then
  #The BMC of class2
  init_class2_server
elif [ $bmc_location -eq 14 ] || [ $bmc_location -eq 7 ]; then
  #The BMC of class1
  init_class1_server
else
  echo -n "Is board id correct(id=$bmc_location)?..."
=======
/usr/local/bin/sync_date.sh

bmc_location=$(get_bmc_board_id)
if [ "$bmc_location" -eq "$BMC_ID_CLASS1" ]; then
  #The BMC of class1
  init_class1_server
elif [ "$bmc_location" -eq "$BMC_ID_CLASS2" ]; then
  #The BMC of class2
  init_class2_server
else
  echo "Is board id correct(id=$bmc_location)?..."
>>>>>>> facebook/helium
fi
