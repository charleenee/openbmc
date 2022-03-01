# Copyright 2015-present Facebook. All Rights Reserved.

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
LOCAL_URI += " \
    file://setup-ipmbd.sh \
    file://run-ipmbd_2.sh \
    file://run-ipmbd_4.sh \
    "
DEPENDS:append = " plat-utils "

CFLAGS:prepend = " -DCONFIG_MINILAKETB"

do_install() {
  dst="${D}/usr/local/fbpackages/${pkgdir}"
  bin="${D}/usr/local/bin"
  install -d $dst
  install -d $bin
  install -m 755 ipmbd ${dst}/ipmbd
  ln -snf ../fbpackages/${pkgdir}/ipmbd ${bin}/ipmbd
  install -d ${D}${sysconfdir}/init.d
  install -d ${D}${sysconfdir}/rcS.d
  install -d ${D}${sysconfdir}/sv
  install -d ${D}${sysconfdir}/sv/ipmbd_2
  install -d ${D}${sysconfdir}/sv/ipmbd_4
  install -d ${D}${sysconfdir}/ipmbd_2
  install -d ${D}${sysconfdir}/ipmbd_4
  install -m 755 setup-ipmbd.sh ${D}${sysconfdir}/init.d/setup-ipmbd.sh
  install -m 755 run-ipmbd_2.sh ${D}${sysconfdir}/sv/ipmbd_2/run
  install -m 755 run-ipmbd_4.sh ${D}${sysconfdir}/sv/ipmbd_4/run
  update-rc.d -r ${D} setup-ipmbd.sh start 65 5 .
}

FBPACKAGEDIR = "${prefix}/local/fbpackages"

FILES:${PN} = "${FBPACKAGEDIR}/ipmbd ${prefix}/local/bin ${sysconfdir} "
