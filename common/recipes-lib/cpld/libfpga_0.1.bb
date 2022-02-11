# Copyright 2015-present Facebook. All Rights Reserved.
SUMMARY = "CPLD Library"
DESCRIPTION = "Library for communicating with CPLD"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://cpld.c;beginline=8;endline=20;md5=da35978751a9d71b73679307c4d296ec"

SRC_URI = "file://cpld.c \
           file://cpld.h \
           file://lattice.c \
<<<<<<< HEAD
           file://lattice.h \
=======
           file://lattice_jtag.c \
           file://lattice_i2c.c \
           file://lattice.h \
           file://lattice_jtag.h \
           file://lattice_i2c.h \
>>>>>>> facebook/helium
           file://altera.c \
           file://altera.h \
           file://meson.build \
          "

<<<<<<< HEAD
DEPENDS = "libast-jtag libpal"
RDEPENDS:${PN} = "libast-jtag libpal"
=======
DEPENDS = "libast-jtag libpal libobmc-i2c"
RDEPENDS:${PN} = "libast-jtag libpal libobmc-i2c"
LDFLAGS += "-lobmc-i2c"
>>>>>>> facebook/helium

S = "${WORKDIR}"

inherit meson
