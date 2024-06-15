#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x2dc67ca7, "struct_module" },
	{ 0xc534b4a6, "remap_pfn_range" },
	{ 0x1a1a4f09, "__request_region" },
	{ 0xf1d0cdab, "__check_region" },
	{ 0x1b7d4074, "printk" },
	{ 0x34cc996, "register_chrdev" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0xc192d491, "unregister_chrdev" },
	{ 0xd49501d4, "__release_region" },
	{ 0x865ebccd, "ioport_resource" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "B7521F0CEB6C7230E13DD47");
