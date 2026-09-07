MLX90640_IR_VERSION = 1.0.0
MLX90640_IR_SITE = /home/william/Documents/Embedded_Systems/Embedded_Linux/EmbeddedLinuxSystems/Buildroot/files/mlx90640_ir
MLX90640_IR_SITE_METHOD = local

define MLX90640_IR_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef

define MLX90640_IR_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/mlx90640_ir $(TARGET_DIR)/usr/bin/mlx90640_ir
endef

$(eval $(generic-package))
