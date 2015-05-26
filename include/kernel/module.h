#ifndef __MODULE_H__
#define __MODULE_H__

#include <types.h>

struct device_interface_t {
	int    (*open) (int id, int mode);
	size_t (*read) (int id, void *buf, size_t size);
	size_t (*write)(int id, void *buf, size_t size);
	int    (*close)(int id);
};

struct dev_t {
	int id;
	int count; /* reference count */
	char *name;
	struct device_interface_t *ops;

	struct list_t link;
};

#define MODULE_INIT(func) \
	static void *module_##func \
	__attribute__((section(".module_list"), used)) = func

int register_dev(int id, struct device_interface_t *ops, char *name);
int mkdev();
struct dev_t *getdev(int id);
void linkdev(int id, struct dev_t *dev);
void driver_init();

#ifdef CONFIG_DEBUG
void display_devtab();
#else
#define display_devtab()
#endif

#endif /* __MODULE_H__ */
