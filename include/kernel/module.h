#ifndef __MODULE_H__
#define __MODULE_H__

#include <kernel/device.h>

#define MODULE_INIT(func) \
	static void *module_##func \
	__attribute__((section(".device_list"), used)) = func

#endif /* __MODULE_H__ */
