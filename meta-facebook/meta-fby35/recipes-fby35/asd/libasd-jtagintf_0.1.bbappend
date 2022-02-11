<<<<<<< HEAD
# Copyright 2017-present Facebook. All Rights Reserved.
FILESEXTRAPATHS:append := "${THISDIR}/files:"
SRC_URI += "file://interface/SoftwareJTAGHandler.c \
           "

LDFLAGS += " -lbic -lipmi -lipmb -lbic -lfby35_gpio -lgpio -lpal"
DEPENDS += "libbic libfby35-gpio libgpio libpal libipmi libipmb"
RDEPENDS:${PN} += "libbic libfby35-gpio libgpio libpal libipmi libipmb"
=======
# Copyright 2021-present Facebook. All Rights Reserved.
FILESEXTRAPATHS:append := "${THISDIR}/files:"

DEPENDS += "libbic libfby35-common"
RDEPENDS:${PN} += "libbic libfby35-common"

LDFLAGS += "-lbic -lfby35_common"
>>>>>>> facebook/helium
