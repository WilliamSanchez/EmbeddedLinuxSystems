#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x92997ed8, "_printk" },
	{ 0x39db21f9, "__serdev_device_driver_register" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x3b6300e, "serdev_device_close" },
	{ 0xe3d9266, "serdev_device_open" },
	{ 0x7929af6b, "serdev_device_set_baudrate" },
	{ 0xe1547d32, "serdev_device_set_flow_control" },
	{ 0xf8c42d6, "serdev_device_set_parity" },
	{ 0x99ea9252, "serdev_device_write_buf" },
	{ 0x4ca0137c, "driver_unregister" },
	{ 0x8884b59d, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Clidar,measure_distance");
MODULE_ALIAS("of:N*T*Clidar,measure_distanceC*");

MODULE_INFO(srcversion, "6B43535707D1D8D5535D7F2");
