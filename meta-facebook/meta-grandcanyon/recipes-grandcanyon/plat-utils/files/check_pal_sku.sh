#!/bin/sh
#
# Copyright 2020-present Facebook. All Rights Reserved.
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
#

. /usr/local/fbpackages/utils/ast-functions

UIC_LOCATION_A_STR="01 "
UIC_LOCATION_B_STR="02 "

get_uic_location_by_gpio()
{
  if [ "$(gpio_get BMC_UIC_LOCATION_IN)" = "$GPIO_VALUE_LOW" ]; then
    return "$UIC_LOCATION_A"
  else
    return "$UIC_LOCATION_B"
  fi
}

get_uic_location_by_exp()
{
  RETRY=20

  while [ $RETRY -gt 0 ]
  do
    # * UIC_ID: 1=UIC_A; 2=UIC_B

    uic_id=$("$EXPANDERUTIL_CMD" "$NETFN_EXPANDER_REQ" "$CMD_GET_UIC_LOCATION")

    if [ "$uic_id" = "$UIC_LOCATION_A_STR" ] || [ "$uic_id" = "$UIC_LOCATION_B_STR" ]; then
      return "$uic_id"
    fi

    logger -s -p user.warn -t check_pal_sku "Retrying get uic location"
    RETRY=$((RETRY-1))
    sleep 1
  done
  logger -s -p user.warn -t check_pal_sku "Get uic location - failed"

  return 0
}

get_sku()
{
  stage=$(get_system_stage)
  if [ "$stage" -lt "$STAGE_DVT" ]; then
    get_uic_location_by_exp
  else
    get_uic_location_by_gpio
  fi

  uic_id=$?
  get_chassis_type
  uic_type=$?

  if [ "$uic_type" = "$UIC_TYPE_5" ]; then
    printf "System: Type5\n"
    if [ "$uic_id" = "$UIC_LOCATION_A" ]; then
      printf "UIC Location: Side A\n"
    elif [ "$uic_id" = "$UIC_LOCATION_B" ]; then
      printf "UIC Location: Side B\n"
    else
      printf "UIC Location: Unknown\n"
    fi
  elif [ "$uic_type" = "$UIC_TYPE_7_HEADNODE" ]; then
    printf "System: Type7 Headnode\n"
  else
    printf "System: Unknown\n"
  fi

  pal_sku=$(((uic_id << UIC_TYPE_SIZE) | uic_type))

  return $pal_sku
}


get_sku
pal_sku=$?
printf "Platform SKU: %s (" "$pal_sku"

i=0
while [ "${i}" -lt "$PAL_SKU_SIZE" ]
do
  tmp_sku_bit=$(((pal_sku >> i) & 1))
  sku_bit=$tmp_sku_bit$sku_bit
  i=$((i+1))
done

printf "%s" "$sku_bit"
printf ")\n"
printf "<SKU[5:0] = {UIC_ID0, UIC_ID1, UIC_TYPE0, UIC_TYPE1, UIC_TYPE2, UIC_TYPE3}>\n"

exit $pal_sku
