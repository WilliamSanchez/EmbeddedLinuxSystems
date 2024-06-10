USER_LIDAR_VERSION = 1.0.0
USER_LIDAR_SITE = /home/william/Documents/Embedded_Systems/Embedded_Linux/EmbeddedLinuxSystems/Buildroot/files/user_lidar
USER_LIDAR_SITE_METHOD = local

define USER_LIDAR_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef

define USER_LIDAR_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/user_lidar $(TARGET_DIR)/usr/bin/user_lidar
endef

$(eval $(generic-package))
