SUMMARY = "Start Qt-Demo application"

LICENSE = "CLOSED"

SRC_URI += "file://qt_demo_start.service \
            file://qt_demo_start.sh"

S = "${WORKDIR}"

inherit systemd

SYSTEMD_PACKAGES="${PN}"

do_install() {
    install -d ${D}${sbindir}
    install -m 0750 qt_demo_start.sh ${D}${sbindir}

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 qt_demo_start.service ${D}${systemd_system_unitdir}
}

FILES:${PN} = "${sbindir} ${systemd_system_unitdir}"

SYSTEMD_SERVICE:${PN} = "qt_demo_start.service"
