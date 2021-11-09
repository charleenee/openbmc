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

DEPENDS += " libipmi libfruid update-rc.d-native libsdr libfby35-fruid libncsi libfby35-common"
RDEPENDS:${PN} += "libipmi libfruid libsdr libfby35-fruid libbic libncsi libfby35-common"

CFLAGS += "-DCONFIG_FBY35"
LDFLAGS += " -lipmb -lbic -lncsi -lfby35_fruid -lfby35_common"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += "file://fruid.c \
            file://usb-dbg-conf.c \
           "
