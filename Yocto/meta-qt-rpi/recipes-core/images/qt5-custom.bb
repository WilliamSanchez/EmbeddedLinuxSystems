SUMMARY = "Autostart Qt5 demo app"
HOMEPAGE = "http://www.interelectronix.com"
LICENSE = "MIT"

# Base this image on rpi-basic-image -> core-image-base
require recipes-core/images/core-image-base.bb

INHERIT += "populate_sdk_qt5_base"

EXTRA_IMAGE_FEATURES ?= "debug-tweaks ssh-server-openssh"

DISTRO_FEATURES:remove = " x11 wayland"

IMAGE_INSTALL:append = " qtbase packagegroup-qt5"
IMAGE_INSTALL:append = " v4l-utils"
IMAGE_INSTALL:append = " wapp"
