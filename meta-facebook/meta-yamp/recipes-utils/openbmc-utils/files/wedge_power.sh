#!/bin/bash
#
# Copyright 2018-present Facebook. All Rights Reserved.
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

. /usr/local/bin/openbmc-utils.sh

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

prog="$0"

usage() {
    echo "Usage: $prog <command> [command options]"
    echo
    echo "Commands:"
    echo "  status: Get the current microserver power status"
    echo
    echo "  on: Power on microserver if not powered on already"
    echo "    options:"
    echo "      -f: Re-do power on sequence no matter if microserver has "
    echo "          been powered on or not."
    echo
    echo "  off: Power off microserver ungracefully"
    echo
    echo "  reset: Power reset microserver ungracefully"
    echo "    options:"
    echo "      -s: Power reset whole yamp system ungracefully"
    echo
    echo "  lcreset: Power-cycle one or all LC(s)"
    echo "    options:"
    echo "      -a  : Reset all LCs or "
    echo "      -1 , -2 , ... , -8 : Reset a single LC (1, 2 ... 8) "
    echo
}

do_status() {
    echo -n "Microserver power is "
    return_code=0

    if wedge_is_us_on; then
        echo "on"
    else
        echo "off"
        return_code=1
    fi

    return $return_code
}

do_on() {
    local force opt ret
    force=0
    while getopts "f" opt; do
        case $opt in
            f)
                force=1
                ;;
            *)
                usage
                exit -1
                ;;
        esac
    done
    echo -n "Power on microserver ..."
    if [ $force -eq 0 ]; then
        # need to check if uS is on or not
        if wedge_is_us_on; then
            echo " Already on. Skip!"
            return 1
        fi
    fi

    wedge_power_on_board
    ret=$?
    if [ $ret -eq 0 ]; then
        echo " Done"
        logger "Successfully power on micro-server"
    else
        echo " Failed"
        logger "Failed to power on micro-server"
    fi
    return $ret
}

do_off() {
    local ret
    echo -n "Power off microserver ..."
    wedge_power_off_board
    ret=$?
    if [ $ret -eq 0 ]; then
        echo " Done"
        logger "Successfully power off micro-server"
    else
        echo " Failed"
        logger "Failed to power off micro-server"
    fi
    return $ret
}

do_reset() {
    local system opt
    system=0
    while getopts "s" opt; do
        case $opt in
            s)
                system=1
                ;;
            *)
                usage
                exit -1
                ;;
        esac
    done
    if [ $system -eq 1 ]; then
        logger "Power reset the whole system ..."
        echo -n "Power reset the whole system ..."
        sleep 1
        echo 0xde > $PWR_SYSTEM_SYSFS
        sleep 8
        # The chassis shall be reset now... if not, we are in trouble
        echo " Failed"
        return 254
    else
        do_off
        sleep 1
        do_on
    fi
}

toggle_pim_reset() {
  pim=$1
  # First, put the PIMs we want into reset
  for slot in 1 2 3 4 5 6 7 8; do
    if [ $pim -eq 0 ] || [ $slot -eq $pim ]; then
      # Unlike Minipack, YAMP pim index is 1 based
      index=$slot
      # Unlike Minipack, YAMP uses GPIO
      echo Power-cycling LC in slot $slot
      echo out > /tmp/gpionames/LC${index}_LC_PWR_CYC/direction
      echo high > /tmp/gpionames/LC${index}_LC_PWR_CYC/direction
    fi
  done
  # Sleep 1 sec
  sleep 1
  # Finally, put the LC_PWR_CYC bit to 0
  for slot in 1 2 3 4 5 6 7 8; do
    if [ $pim -eq 0 ] || [ $slot -eq $pim ]; then
      # Unlike Minipack, YAMP pim index is 1 based
      index=$slot
      # Unlike Minipack, YAMP uses GPIO
      echo Putting LC in $slot into normal operation
      echo low > /tmp/gpionames/LC${index}_LC_PWR_CYC/direction
    fi
  done
}
do_pimreset() {
    local pim opt retval rc
    retval=0
    pim=-1
    while getopts "12345678a" opt; do
        case $opt in
            a)
                pim=0
                ;;
            1)
                pim=1
                ;;
            2)
                pim=2
                ;;
            3)
                pim=3
                ;;
            4)
                pim=4
                ;;
            5)
                pim=5
                ;;
            6)
                pim=6
                ;;
            7)
                pim=7
                ;;
            8)
                pim=8
                ;;
            *)
                usage
                exit -1
                ;;
        esac
    done
    if [ $pim -eq -1 ]; then
      usage
      exit -1
    fi

    toggle_pim_reset $pim

    return $retval
}

if [ $# -lt 1 ]; then
    usage
    exit -1
fi

command="$1"
shift

case "$command" in
    status)
        do_status $@
        ;;
    on)
        do_on $@
        ;;
    off)
        do_off $@
        ;;
    reset)
        do_reset $@
        ;;
    pimreset)
        do_pimreset $@
        ;;
    lcreset)
        do_pimreset $@
        ;;
    *)
        usage
        exit -1
        ;;
esac

exit $?
