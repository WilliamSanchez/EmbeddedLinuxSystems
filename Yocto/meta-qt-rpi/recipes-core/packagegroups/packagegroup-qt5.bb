DESCRIPTION = "RaspberryPi Qt5 Packagegroup"
LICENSE = "CLOSED"

inherit packagegroup

COMPATIBLE_MACHINE = "^rpi$"

RDEPENDS_${PN} = "\
    qtbase \
    qtbase-dev \
    qtbase-mkspecs \
    qtbase-plugins \
    qtbase-tools \
    qt3d \
    qt3d-dev \
    qt3d-mkspecs \
    qt3d-plugins \
    qt3d-qmlplugins \
    qtcharts \
    qtcharts-dev \
    qtcharts-mkspecs \
    qtconnectivity-dev \
    qtconnectivity-mkspecs \
    qtconnectivity-tools \
    qtconnectivity-qmlplugins \
    qtimageformats-plugins \
    qtlocation \
    qtlocation-dev \
    qtlocation-mkspecs \
    qtlocation-plugins \
    qtlocation-tools \
    qtmultimedia \
    qtmultimedia-dev \
    qtmultimedia-mkspecs \
    qtmultimedia-plugins \
    qtmultimedia-qmlplugins \
    qtquickcontrols \
    qtquickcontrols-dev \
    qtquickcontrols-mkspecs \
    qtquickcontrols-qmlplugins \
    qtquickcontrols2 \
    qtquickcontrols2-dev \
    qtquickcontrols2-mkspecs \
    qtquickcontrols2-qmlplugins \
    qtdeclarative \
    qtdeclarative-dev \
    qtdeclarative-plugins \
    qtdeclarative-mkspecs \
    qtdeclarative-tools \
    qtdeclarative-qmlplugins \
    qtgraphicaleffects \
    qtgraphicaleffects-dev \
    qtgraphicaleffects-qmlplugins \
    qsiv \
    qtsensors \
    qtsensors-dev \
    qtsensors-mkspecs \
    qtbase-fonts \
    qtbase-fonts-pfa \
    qtbase-fonts-pfb \
    qtbase-fonts-qpf \
    qtbase-fonts-ttf-dejavu \
    qtbase-fonts-ttf-vera \
"
