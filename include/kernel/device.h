#ifndef __DEVICE_H__
#define __DEVICE_H__

/* It must start from 1, not 0 to check sanity */
#define USART		1

#include <types.h>

struct driver_operations {
	int    (*open) (int id, int mode);
	size_t (*read) (int id, void *buf, size_t size);
	size_t (*write)(int id, void *buf, size_t size);
	int    (*close)(int id);
};

struct device_t {
	int id;
	int count; /* reference count */
	struct driver_operations *ops;

	struct list_t link;
};

struct device_t *getdev(int id);
void link_device(int id, struct device_t *dev);
void __devman_init();

#define REGISTER_DEVICE(n, operations) \
	static struct device_t dev_##n \
	__attribute__((section(".device_list"), used)) = { \
		.id    = n, \
		.count = 0, \
		.ops   = operations, \
		.link  = {NULL, NULL}, \
	}

#ifdef CONFIG_DEBUG
void display_devtab();
#else
#define display_devtab()
#endif

#endif /* __DEVICE_H__ */
