# Copyright 2021-present Facebook. All Rights Reserved.
SUMMARY = "Bridge IC Utility"
DESCRIPTION = "Util for checking with Bridge IC on FBGC"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://bic-util.c;beginline=4;endline=16;md5=b66b777f082370423b0fa6f12a3dc4db"

SRC_URI = "file://bic-util \
          "
inherit meson

S = "${WORKDIR}/bic-util"

DEPENDS += "libbic libpal libfbgc-common libfbgc-gpio"
RDEPENDS:${PN} += "libbic libpal libfbgc-common libfbgc-gpio"

FILES:${PN} = "${bindir}"
