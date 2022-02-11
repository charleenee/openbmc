
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://elbert.conf \
            file://pim16q.conf \
<<<<<<< HEAD
=======
            file://pim16q2.conf \
>>>>>>> facebook/helium
            file://pim8ddm.conf \
           "

do_install:append() {
    install -d ${D}${sysconfdir}/sensors.d
    install -m 644 ../elbert.conf ${D}${sysconfdir}/sensors.d/elbert.conf
    install -m 644 ../pim16q.conf ${D}${sysconfdir}/sensors.d/.pim16q.conf
<<<<<<< HEAD
=======
    install -m 644 ../pim16q2.conf ${D}${sysconfdir}/sensors.d/.pim16q2.conf
>>>>>>> facebook/helium
    install -m 644 ../pim8ddm.conf ${D}${sysconfdir}/sensors.d/.pim8ddm.conf
}
