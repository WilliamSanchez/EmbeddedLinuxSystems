CUSTOMSERVICE_VERSION = 1.0.0
CUSTOMSERVICE_SITE = /home/william/Documents/Embedded_Systems/Embedded_Linux/EmbeddedLinuxSystems/Buildroot/files/customservice
CUSTOMSERVICE_SITE_METHOD = local

define CUSTOMSERVICE_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef

define CUSTOMSERVICE_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/customservice $(TARGET_DIR)/usr/bin/customservice
endef

$(eval $(generic-package))
