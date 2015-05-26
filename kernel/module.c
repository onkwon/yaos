#include <kernel/module.h>
#include <kernel/page.h>
#include <kernel/lock.h>
#include <hash.h>
#include <error.h>

#define HASH_SHIFT	4
#define TABLE_SIZE	(1 << HASH_SHIFT)

static struct list_t devtab[TABLE_SIZE];
static int nr_device;
static DEFINE_RWLOCK(lock_devtab);
static DEFINE_SPINLOCK(nr_lock);

struct dev_t *getdev(int id)
{
	struct dev_t *dev;
	struct list_t *head, *next;

	next = NULL; /* to get rid of compile warning */
	head = &devtab[hash(id, HASH_SHIFT)];

	do {
		read_lock(lock_devtab);
		next = head->next;
		read_unlock(lock_devtab);

		if (next == head) {
			dev = NULL;
			break;
		}

		dev = get_container_of(next, struct dev_t, link);
	} while (dev->id != id);

	return dev;
}

int mkdev()
{
	spin_lock(nr_lock);
	nr_device += 1;
	spin_unlock(nr_lock);

	return nr_device;
}

void linkdev(int id, struct dev_t *new)
{
	struct list_t *head = &devtab[hash(id, HASH_SHIFT)];

	write_lock(lock_devtab);
	list_add(&new->link, head);
	write_unlock(lock_devtab);
}

int register_dev(int id, struct device_interface_t *ops, char *name)
{
	if (id <= 0)
		return -ERR_RANGE;

	struct dev_t *dev = (struct dev_t *)
		kmalloc(sizeof(struct dev_t));

	dev->id = id;
	dev->count = 0;
	dev->ops = ops;
	list_link_init(&dev->link);

	linkdev(dev->id, dev);

	/* make a file interface under /dev directory */
	if (name == NULL)
		;
	/* add count at the end of name if the same name already exists */

	dev->name = name;

	return 0;
}

#include <kernel/init.h>

void __init driver_init()
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
}

#ifdef CONFIG_DEBUG
void display_devtab()
{
	struct dev_t *dev;
	struct list_t *head, *next;
	int i;

	for (i = 0; i < TABLE_SIZE; i++) {
		head = &devtab[i];
		next = head->next;

		printk("[%02d] ", i);
		while (next != head) {
			dev = get_container_of(next, struct dev_t, link);
			printk("--> %d ", dev->id);

			next = next->next;
		}
		printk("\n");
	}
}
#endif
