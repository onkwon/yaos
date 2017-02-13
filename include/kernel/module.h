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
	void __register_##module_##name_##minor() { \
		register_##module(name, minor); \
	} \
	DEVICE_INIT(__register_##module_##name_##minor)

#include <error.h>

/* This function must be called at boot-time only. Otherwise you would need a
 * lock for synchronization of major and minor */
static inline struct device *register_device_core(const char *name,
		int major, int minor, struct file_operations *ops)
{
	struct device *dev;

	if ((dev = mkdev(major, minor, ops, name))) {
		major = MAJOR(dev->id);
		debug("%s device registered at %d:%d", name, major, minor);

		return dev;
	}

	error("failed to register the device, %s at %d:%d",
			name, major, minor);

	return NULL;
}

extern void register_timer(const char *name, int minor);
extern void register_led(const char *name, int minor);
extern void register_gpio(const char *name, int minor);

#endif /* __MODULE_H__ */
