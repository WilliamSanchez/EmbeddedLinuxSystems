SUMMARY = "QT William Example"
DESCRIPTION = "Recipe created by bitbake-layers"
LICENSE = "CLOSED"

DEPENDS += " qtbase packagegroup-qt5"
DEPENDS += "qtbase \
	qtdeclarative \
	qtquickcontrols"

SRC_URI = "file://mainwindow.h \
	   file://mainwindow.cpp \
           file://myQTApp.cpp \
		   file://mainwindow.ui \
		   file://qml.qrc \
		   file://map.qml \
	   file://myQTApp.pro \
	   "

S = "${WORKDIR}"

do_install:append(){
	install -d ${D}${bindir}
	install -m 0775 myQTApp ${D}${bindir}/
}

FILES_${PN} += "${bindir}/myQTApp"

inherit qmake5
