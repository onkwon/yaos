#ifndef __MODULE_H__
#define __MODULE_H__

#include <types.h>

struct device_interface_t {
	int    (*open) (int id, int mode);
	size_t (*read) (int id, void *buf, size_t size);
	size_t (*write)(int id, void *buf, size_t size);
	int    (*close)(int id);
};

struct device_t {
	int id;
	int count; /* reference count */
	char *name;
	struct device_interface_t *ops;

	struct list_t link;
};

#define module_init(func) \
	static void *module_##func \
	__attribute__((section(".module_list"), used)) = func

int register_device(int id, struct device_interface_t *ops, char *name);
int dev_get_newid();
struct device_t *getdev(int id);
void link_device(int id, struct device_t *dev);
void driver_init();

#ifdef CONFIG_DEBUG
void display_devtab();
#else
#define display_devtab()
#endif

#endif /* __MODULE_H__ */
