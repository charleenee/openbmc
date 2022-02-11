<<<<<<< HEAD
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
=======
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
>>>>>>> facebook/helium

SRC_URI += "file://facebook-yosemite_defconfig.append \
           "

do_copyfile () {
    bbnote "copy files"

}
addtask copyfile after do_patch before do_configure
