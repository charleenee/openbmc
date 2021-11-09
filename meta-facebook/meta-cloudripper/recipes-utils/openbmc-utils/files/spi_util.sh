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
#shellcheck disable=SC1091
#shellcheck disable=SC2039
#shellcheck disable=SC2086
. /usr/local/bin/openbmc-utils.sh

ret=0
trap cleanup_spi INT TERM QUIT EXIT

PIDFILE="/var/run/spi_util.pid"
check_duplicate_process()
{
    exec 2>$PIDFILE
    flock -n 2 || (echo "Another process is running" && exit 1)
    ret=$?
    if [ $ret -eq 1 ]; then
      exit 1
    fi
    pid=$$
    echo $pid 1>&200
}

remove_pid_file()
{
    if [ -f $PIDFILE ]; then
        rm $PIDFILE
    fi
}

SPI1_MTD_INDEX=7
SPI1_DRIVER="1e630000.spi"
ASPEED_SMC_PATH="/sys/bus/platform/drivers/aspeed-smc/"

read_spi1_dev(){
    dev=$1
    file=$2
    echo "Reading ${dev} to $file..."
    flashrom -p linux_mtd:dev=$SPI1_MTD_INDEX -r "$file"
}

write_spi1_dev(){
    dev=$1
    file=$2
    echo "Writing $file to ${dev}..."

    tmpfile="$file"_ext
    flashsize=$(flashrom -p linux_mtd:dev=$SPI1_MTD_INDEX | grep -i kB | xargs echo | cut -d '(' -f 2 | cut -d ' ' -f 0)
    targetsize=$((flashsize * 1024))

    cp "$file" "$tmpfile"

    #extend the temp-file
    filesize=$(stat -c%s $tmpfile)
    add_size=$((targetsize - filesize))
    dd if=/dev/zero bs="$add_size" count=1 | tr "\000" "\377" >> "$tmpfile"
    flashrom -p linux_mtd:dev=$SPI1_MTD_INDEX -w "$tmpfile"
}

erase_spi1_dev(){
    flashrom -p linux_mtd:dev=$SPI1_MTD_INDEX -E
}

mtd_driver_unbind_spi1() {
    [ -e ${ASPEED_SMC_PATH}${SPI1_DRIVER} ] && echo ${SPI1_DRIVER} > ${ASPEED_SMC_PATH}unbind
}

mtd_driver_rebind_spi1() {
    # Sometime kernel may complain "vmap allocation for size 268439552 failed: use vmalloc=<size> to increase size"
    # until now the root cause is not clear, still need more investigation
    mtd_driver_unbind_spi1
    echo ${SPI1_DRIVER} > ${ASPEED_SMC_PATH}bind
}

config_spi1_pin_and_path(){
    dev=$1
    case ${dev} in
        "BIOS")
            echo "[Setup] BIOS"
            echo 0x01 > "$SMBCPLD_SYSFS_DIR/spi_1_sel"
        ;;
        "GB_PCIE_FLASH")
            echo "[Setup] GB_PCIE_FLASH"
            echo 0x03 > "$SMBCPLD_SYSFS_DIR/spi_1_sel"
        ;;
        "DOM_FPGA_FLASH1")
            echo "[Setup] DOM_FPGA_FLASH1"
            echo 0x04 > "$SMBCPLD_SYSFS_DIR/spi_1_sel"
        ;;
        "DOM_FPGA_FLASH2")
            echo "[Setup] DOM_FPGA_FLASH2"
            echo 0x05 > "$SMBCPLD_SYSFS_DIR/spi_1_sel"
        ;;
        *)
            echo "Please enter {BIOS, GB_PCIE_FLASH, DOM_FPGA_FLASH1, DOM_FPGA_FLASH2}"
            exit 1
        ;;
    esac
    echo "Config SPI1 Done."
}

operate_spi1_dev(){
    op=$1
    dev=$2
    file=$3
    mtd_driver_rebind_spi1
    ## Operate devices ##
    case ${op} in
        "read")
                read_spi1_dev "$dev" "$file"
        ;;
        "write")
                write_spi1_dev "$dev" "$file"
        ;;
        "erase")
                erase_spi1_dev "$dev"
        ;;
        *)
            echo "Operation $op is not defined."
        ;;
    esac
    mtd_driver_unbind_spi1
}

read_spi2_dev(){
    dev=$1
    file=$2
    file_temp="$file"_temp
    echo -n "Reading ${dev} to $file ... "
    # Sometimes the read back BCM5389_EE image one bit fail.
    # still not found the real root cause
    for n in 1 2 3 4 5
    do
        echo "Reading $file $n times"
        dd if=/sys/bus/spi/devices/spi2.1/eeprom of="$file"
        sleep 1
        dd if=/sys/bus/spi/devices/spi2.1/eeprom of="$file_temp"
        if ! diff $file $file_temp >/dev/null; then
            # read-back file differ
            rm -rf "$file"*
        else
            # read-back file same
            rm -rf $file_temp;
            return;
        fi
    done
    logger -p user.err "Unable to read the correct $file after 5 retries"
    exit 1
}

write_spi2_dev(){
    dev=$1
    file=$2
    echo -n "Writing $file to ${dev} ... "
    dd if="$file" of=/sys/bus/spi/devices/spi2.1/eeprom
    echo "Done"
}

erase_spi2_dev(){
    echo -n "Erasing ${dev} ... "
    echo 1 > /sys/bus/spi/devices/spi2.1/erase
    echo "Done"
}

config_spi2_pin_and_path(){
    dev=$1

    case ${dev} in
        "BCM5389_EE")
            echo "[Setup] BCM5389_EE"
            echo 0x02 > "$SMBCPLD_SYSFS_DIR/spi_1_sel"
        ;;
        *)
            echo "Please enter {BCM5389_EE}"
            exit 1
        ;;
    esac
    echo "Config SPI2 Done."
}

operate_spi2_dev(){
    op=$1
    dev=$2
    file=$3
    ## Operate devices ##
    case ${op} in
        "read")
            read_spi2_dev "$dev" "$file"
        ;;
        "write")
            write_spi2_dev "$dev" "$file"
        ;;
        "erase")
            erase_spi2_dev "$dev"
        ;;
        *)
            echo "Operation $op is not defined."
        ;;
    esac
}

cleanup_spi(){
    if [ $ret -eq 1 ]; then
        exit 1
    fi
    echo 0 > "$SMBCPLD_SYSFS_DIR/spi_1_sel"

    rm -rf /tmp/*_spi*_tmp
    remove_pid_file
}

ui(){
    . /usr/local/bin/openbmc-utils.sh
    op=$1
    spi=$2
    ## Open the path to device ##
    case ${spi} in
        "spi1")
            dev=$3
            file=$4
            config_spi1_pin_and_path "$dev"
            operate_spi1_dev "$op" "$dev" "$file"
        ;;
        "spi2")
            dev=$3
            file=$4
            config_spi2_pin_and_path "$dev"
            operate_spi2_dev "$op" "$dev" "$file"
        ;;
        *)
            echo "No such SPI bus."
            return 1
        ;;
    esac
}

usage(){
    program=$(basename "$0")
    echo "Usage:"
    echo "  $program <op> <spi1> <spi device> <file>"
    echo "    <op>          : read, write, erase"
    echo "    <spi1 device> : BIOS, GB_PCIE_FLASH, DOM_FPGA_FLASH1, DOM_FPGA_FLASH2"
    echo "    <spi2 device> : BCM5389_EE"
    echo ""
    echo "Examples:"
    echo "  $program write spi1 BIOS bios.bin"
    echo ""
}

check_parameter(){
    program=$(basename "$0")
    op=$1
    spi=$2

    # check operations.
    case ${op} in
        "read" | "write" | "erase")
            ;;
        *)
            return 1
            ;;
    esac

    case ${spi} in
        # check spi 1 device and parameters.
        "spi1")
            dev=$3
            file=$4
            case ${dev} in
                "BIOS"|"GB_PCIE_FLASH"|"DOM_FPGA_FLASH1"|"DOM_FPGA_FLASH2")
                    case ${op} in
                        "read" | "write")
                            if [ $# -ne 4 ]; then
                                return 1
                            fi
                            ;;
                    "erase")
                            if [ $# -ne 3 ]; then
                                return 1
                            fi
                            ;;
                    esac
                    ;;
                *)
                    return 1
                    ;;
            esac
            ;;
        # check spi 2 device and parameters.
        "spi2")
            dev=$3
            file=$4
            case ${dev} in
                "BCM5389_EE")
                    case ${op} in
                        "read" | "write")
                            if [ $# -ne 4 ]; then
                                return 1
                            fi
                            ;;
                        "erase")
                            if [ $# -ne 3 ]; then
                                return 1
                            fi
                            ;;
                    esac
                    ;;
                *)
                    return 1
                    ;;
            esac
            ;;
        # check device and parameters fail.
        *)
            return 1
            ;;
    esac

    return 0
}


# check_duplicate_process - to check if there is the duplicate spi util process, will return fail and stop execute command.
check_duplicate_process

# check_parameter - to check command parameters
check_parameter "$@"

is_par_ok=$?

if [ $is_par_ok -eq 0 ]; then
    ui "$@"
    exit 0
else
    usage
    exit 1
fi

