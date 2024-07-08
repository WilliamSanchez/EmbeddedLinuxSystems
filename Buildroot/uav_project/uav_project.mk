UAV_PROJECT_VERSION = 1.0.0
UAV_PROJECT_SITE = /home/william/Documents/Embedded_Systems/Embedded_Linux/EmbeddedLinuxSystems/Buildroot/files/uav_project
UAV_PROJECT_SITE_METHOD = local

define UAV_PROJECT_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef

define UAV_PROJECT_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/uav_project $(TARGET_DIR)/usr/bin/uav_project
endef

$(eval $(generic-package))
