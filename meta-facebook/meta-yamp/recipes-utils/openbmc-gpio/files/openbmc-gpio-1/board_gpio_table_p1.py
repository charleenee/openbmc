# Copyright 2018-present Facebook. All rights reserved.
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

from openbmc_gpio_table import BoardGPIO


# The following table is generated using:
# python minipack_gpio_parser.py data/minipack-BMC-gpio.csv
# DO NOT MODIFY THE TABLE!!!
# Manual modification will be overridden!!!

board_gpio_table_p1 = [
    BoardGPIO("GPIOB0", "TPM_RST_N"),
    BoardGPIO("GPIOB1", "TPM_INTR"),
    BoardGPIO("GPIOB3", "BMC_eMMC_RST_N"),
    BoardGPIO("GPIOL0", "BIOS_SEL"),
    BoardGPIO("GPIOL2", "SUP_PWR_ON"),
    BoardGPIO("GPIOL3", "CPLD_JTAG_SEL_L"),
]
