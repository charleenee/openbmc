FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://facebook-fby35_defconfig.append \
           "

do_copyfile () {
    bbnote "echo do nothing"
}
addtask copyfile after do_patch before do_configure
