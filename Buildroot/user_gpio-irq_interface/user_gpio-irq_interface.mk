USER_GPIO_IRQ_INTERFACE_VERSION = 1.0.0
USER_GPIO_IRQ_INTERFACE_SITE = /home/william/Documents/Embedded_Systems/Embedded_Linux/EmbeddedLinuxSystems/Buildroot/files/user_gpio-irq_interface
USER_GPIO_IRQ_INTERFACE_SITE_METHOD = local

define  USER_GPIO_IRQ_INTERFACE
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef

define  USER_GPIO_IRQ_INTERFACE_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/user_gpio-irq_interface $(TARGET_DIR)/usr/bin/user_gpio-irq_interface
endef

$(eval $(generic-package))
