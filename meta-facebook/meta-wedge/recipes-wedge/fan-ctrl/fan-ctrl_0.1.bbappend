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

DEPENDS:append = "libwedge-eeprom update-rc.d-native libwatchdog libgpio-ctrl libmisc-utils libobmc-i2c"
RDEPENDS:${PN} += "libwedge-eeprom libwatchdog libgpio-ctrl libmisc-utils libobmc-i2c"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

LOCAL_URI += " \
    file://get_fan_speed.sh \
    file://init_pwm.sh \
    file://pwm_common.sh \
    file://set_fan_speed.sh \
    file://setup-fan.sh \
    file://fand.cpp \
    "

binfiles += "get_fan_speed.sh \
            init_pwm.sh \
            pwm_common.sh \
            set_fan_speed.sh \
           "

LDFLAGS:append = " -lwedge_eeprom -lwatchdog -lgpio-ctrl -lmisc-utils -lobmc-i2c"

pkgdir = "fan_ctrl"

do_install() {
  dst="${D}/usr/local/fbpackages/${pkgdir}"
  bin="${D}/usr/local/bin"
  install -d $dst
  install -d $bin
  for f in ${binfiles}; do
    install -m 755 $f ${dst}/$f
    ln -snf ../fbpackages/${pkgdir}/$f ${bin}/$f
  done
  for f in ${otherfiles}; do
    install -m 644 $f ${dst}/$f
  done
  install -d ${D}${sysconfdir}/init.d
  install -d ${D}${sysconfdir}/rcS.d
  install -m 755 setup-fan.sh ${D}${sysconfdir}/init.d/setup-fan.sh
  update-rc.d -r ${D} setup-fan.sh start 91 S .
}

FBPACKAGEDIR = "${prefix}/local/fbpackages"

FILES:${PN} = "${FBPACKAGEDIR}/fan_ctrl ${prefix}/local/bin ${sysconfdir} "
