USER_PWM_VERSION = 1.0.0
USER_PWM_SITE = /home/william/Documents/Embedded_Systems/Embedded_Linux/EmbeddedLinuxSystems/Buildroot/files/user_pwm
USER_PWM_SITE_METHOD = local

define USER_PWM_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef

define USER_PWM_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/user_pwm $(TARGET_DIR)/usr/bin/user_pwm
endef

$(eval $(generic-package))
