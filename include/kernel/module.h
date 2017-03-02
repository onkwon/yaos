#ifndef __MODULE_H__
#define __MODULE_H__

#include <kernel/device.h>

#define MODULE_INIT(func) \
	static void *module_##func \
	__attribute__((section(".driver_list"), used)) = func

#define DEVICE_INIT(func) \
	static void *device_##func \
	__attribute__((section(".device_list"), used)) = func

#define REGISTER_DEVICE(module, name, minor) \
	void register_##module##minor() { \
		register_##module(name, minor); \
	} \
	DEVICE_INIT(register_##module##minor)

/* This function must be called at boot-time only. Otherwise you would need a
 * lock for synchronization of major and minor */
#define macro_register_device(name, major, minor, ops) do { \
	struct device *dev; \
	dev = mkdev(major, minor, ops, name); \
	major = MAJOR(dev->id); \
} while (0); \

extern void register_timer(const char *name, int minor);
extern void register_led(const char *name, int minor);
extern void register_gpio(const char *name, int minor);

#endif /* __MODULE_H__ */
