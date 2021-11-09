# Copyright 2017-present Facebook. All Rights Reserved.
FILESEXTRAPATHS:append := "${THISDIR}/files:"
SRC_URI += "file://interface/SoftwareJTAGHandler.c \
           "

LDFLAGS += " -lbic -lipmi -lipmb -lbic -lfby2_gpio -lgpio -lpal"
DEPENDS += "libbic libfby2-gpio libgpio libpal libipmi libipmb"
RDEPENDS:${PN} += "libbic libfby2-gpio libgpio libpal libipmi libipmb"
