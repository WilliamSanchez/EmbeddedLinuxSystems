BLUETOOTH_SERIAL_VERSION = 1.0.0
BLUETOOTH_SERIAL_SITE = /home/william/Documents/Embedded_Systems/Embedded_Linux/EmbeddedLinuxSystems/Buildroot/files/bluetooth_serial
BLUETOOTH_SERIAL_SITE_METHOD = local

define BLUETOOTH_SERIAL_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef

define BLUETOOTH_SERIAL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/bluetooth_serial $(TARGET_DIR)/usr/bin/bluetooth_serial
endef

$(eval $(generic-package))
