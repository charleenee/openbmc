FILESEXTRAPATHS:prepend := "${THISDIR}/files/pal:"

LOCAL_URI += " \
    file://plat/meson.build \
    "

DEPENDS += " \
    libbic \
    libsensor-correction \
    libobmc-i2c \
    libmisc-utils \
    liblog \
    "

# These shouldn't be needed but are because we aren't properly versioning the
# shared libraries contained in these recipes.
RDEPENDS:${PN} += " \
    libbic \
    liblog \
    libmisc-utils \
    libobmc-i2c \
    libsensor-correction \
    "
