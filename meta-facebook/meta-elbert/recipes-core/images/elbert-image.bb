# Copyright 2020-present Facebook. All Rights Reserved.

require recipes-core/images/fbobmc-image-meta.inc
require elbert-image-layout.inc

require recipes-core/images/fb-openbmc-image.bb

# Include modules in rootfs
IMAGE_INSTALL += " \
  packagegroup-openbmc-base \
  packagegroup-openbmc-net \
  packagegroup-openbmc-python3 \
  packagegroup-openbmc-rest3 \
  ast-mdio \
  cpldupdate \
  libcpldupdate-dll-ioctl \
  flashrom \
  fscd \
  front-paneld \
  led-controld \
  lldp-util \
  mterm \
  openbmc-utils \
  usb-console \
  wedge-eeprom \
  weutil-dhcp-id \
  kcsd \
  ipmid \
  ipmitool \
  guid-util \
  psu-util \
  sensor-util \
  sensor-mon \
  threshold-util \
  "

# Add vboot-utils for ELBERTVBOOT image
IMAGE_INSTALL += '${@bb.utils.contains("IMAGE_FEATURES", "verified-boot", "vboot-utils", "", d)}'

PROVIDES += "elbertvboot-image"
