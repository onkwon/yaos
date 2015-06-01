#ifndef __MODULE_H__
#define __MODULE_H__

#define MAJOR_BITS		16
#define MAJOR_MASK		((1 << MAJOR_BITS) - 1)
#define MAJOR_MAX		MAJOR_MASK
#define MAJOR(id)		((id) & MAJOR_MASK)
#define MINOR(id)		((id) >> MAJOR_BITS)

#include <types.h>

struct dev_interface_t {
	int    (*open) (unsigned int id, int mode);
	size_t (*read) (unsigned int id, void *buf, size_t size);
	size_t (*write)(unsigned int id, void *buf, size_t size);
	int    (*close)(unsigned int id);
};

struct dev_t {
	unsigned int id;
	unsigned int count; /* reference count */
	char *name;
	struct dev_interface_t *ops;

	struct list_t link;
};

#define MODULE_INIT(func) \
	static void *module_##func \
	__attribute__((section(".module_list"), used)) = func

int register_dev(unsigned int id, struct dev_interface_t *ops, char *name);
unsigned int mkdev();
struct dev_t *getdev(unsigned int id);
void linkdev(unsigned int id, struct dev_t *dev);
void driver_init();

#ifdef CONFIG_DEBUG
void display_devtab();
#else
#define display_devtab()
#endif

#endif /* __MODULE_H__ */
