FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

PTEST_ENABLED = ""

<<<<<<< HEAD
SRC_URI += "file://less.cfg"
=======
SRC_URI:append:openbmc-fb = " \
    file://less.cfg \
    "
>>>>>>> facebook/helium
