# Copyright 2021-present Facebook. All Rights Reserved.
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

SUMMARY = "PEM Sensor Monitoring Daemon"
DESCRIPTION = "Daemon for monitoring the PEM sensors"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://pemd.c;beginline=4;endline=16;md5=219631f7c3c1e390cdedab9df4b744fd"

SRC_URI = "file://Makefile \
           file://pemd.c \
           file://platform_pemd.c \
           file://platform_pemd.h \
          "

S = "${WORKDIR}"

binfiles = "pemd \
           "

CFLAGS += " -llog -lpal -lpem "

DEPENDS += " liblog libpal libpem update-rc.d-native "
RDEPENDS:${PN} += " liblog libpal libpem "

pkgdir = "pemd"
