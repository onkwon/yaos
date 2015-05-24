#include <kernel/module.h>
#include <kernel/page.h>
#include <error.h>

#define HASH_SHIFT	4
#define TABLE_SIZE	(1 << HASH_SHIFT)

static struct list_t devtab[TABLE_SIZE];
static int nr_device;

unsigned long hash_long(unsigned long val, unsigned int bits)
{
	unsigned long hash = val * 0x9e370001UL;
	return hash >> (32 - bits);
}

struct device_t *getdev(int id)
{
	struct device_t *dev;
	struct list_t *head, *next;

	head = &devtab[hash_long(id, HASH_SHIFT)];
	next = head->next;

	while (next != head) {
		dev = get_container_of(next, struct device_t, link);
		if (dev->id == id)
			return dev;

		next = next->next;
	}

	return NULL;
}

int dev_get_newid()
{
	return ++nr_device;
}

void link_device(int id, struct device_t *new)
{
	struct list_t *head;

	head = &devtab[hash_long(id, HASH_SHIFT)];
	list_add(&new->link, head);
}

int register_device(int id, struct device_interface_t *ops, char *name)
{
	if (id <= 0)
		return -ERR_RANGE;

	struct device_t *dev = (struct device_t *)
		kmalloc(sizeof(struct device_t));

	dev->id = id;
	dev->count = 0;
	dev->ops = ops;
	list_link_init(&dev->link);

	link_device(dev->id, dev);

	/* make a file interface under /dev directory */
	if (name == NULL)
		;
	/* add count at the end of name if the same name already exists */

	dev->name = name;

	return 0;
}

void driver_init()
{
	int i;

	nr_device = 0;

	for (i = 0; i < TABLE_SIZE; i++) {
		list_link_init(&devtab[i]);
	}

	extern char _module_list;
	int *p = (int *)&_module_list;

	while (*p)
		((int (*)())*p++)();

	/*
	int *p, (*func)();
	extern char _module_list;

	p = (int *)&_module_list;

	while (*p) {
		func = (int (*)())(*p++);
		func();
	}
	*/
}

#ifdef CONFIG_DEBUG
void display_devtab()
{
	struct device_t *dev;
	struct list_t *head, *next;
	int i;

	for (i = 0; i < TABLE_SIZE; i++) {
		head = &devtab[i];
		next = head->next;

		printk("[%02d] ", i);
		while (next != head) {
			dev = get_container_of(next, struct device_t, link);
			printk("--> %d ", dev->id);

			next = next->next;
		}
		printk("\n");
	}
}
#endif
