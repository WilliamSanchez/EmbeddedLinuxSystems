SUMMARY = "Autostart Qt5 demo app"
HOMEPAGE = "http://www.interelectronix.com"
LICENSE = "MIT"

# Base this image on rpi-basic-image -> core-image-base
require recipes-core/images/core-image-base.bb

INHERIT += "rm_work"
INHERIT += "populate_sdk_qt5_base"

EXTRA_IMAGE_FEATURES ?= "debug-tweaks ssh-server-openssh"
IMAGE_INSTALL:append = " fontconfig"

INIT_MANAGER = "systemd"

IMAGE_FEATURES += "dev-pkgs"

DISTRO_FEATURES:remove = " x11 wayland"

IMAGE_INSTALL:append = " qtbase packagegroup-qt5"
IMAGE_INSTALL:append = " v4l-utils"
IMAGE_INSTALL:append = " liberation-fonts freetype"
IMAGE_INSTALL:append = " wapp openssh"
IMAGE_INSTALL:append = " qtbase qtbase-dev qtbase-tools"
IMAGE_INSTALL:append = " qt5demond"
IMAGE_INSTALL:append = " packagegroup-core-full-cmdline"
IMAGE_INSTALL:append = "\
    ttf-dejavu-sans \
    ttf-dejavu-sans-mono \
    ttf-dejavu-sans-condensed \
    ttf-dejavu-serif \
    ttf-dejavu-serif-condensed \
    ttf-dejavu-common \
