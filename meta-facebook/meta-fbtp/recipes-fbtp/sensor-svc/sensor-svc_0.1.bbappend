# Copyright 2017-present Facebook. All Rights Reserved.

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

LOCAL_URI =+ " \
    file://setup-sensor-svcd.sh \
    file://run-sensor-svcd.sh \
    "

LDFLAGS =+ "-lgpio -lsensor-svc-pal -lkv -lipmb -lgpio -lobmc-pal"
DEPENDS =+ "update-rc.d-native libgpio libsensor-svc-pal"
RDEPENDS:${PN} += "libgpio"

do_install:append() {
  install -d ${D}${sysconfdir}/init.d
  install -d ${D}${sysconfdir}/rcS.d
  install -d ${D}${sysconfdir}/sv
  install -d ${D}${sysconfdir}/sv/sensor-svcd
  install -d ${D}${sysconfdir}/sensor-svcd

  install -m 755 setup-sensor-svcd.sh ${D}${sysconfdir}/init.d/setup-sensor-svcd.sh
  install -m 755 run-sensor-svcd.sh ${D}${sysconfdir}/sv/sensor-svcd/run
  update-rc.d -r ${D} setup-sensor-svcd.sh start 91 5 .
}
