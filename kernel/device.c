#include <kernel/device.h>
#include <kernel/page.h>
#include <kernel/lock.h>
#include <hash.h>
#include <error.h>

#define HASH_SHIFT			4
#define TABLE_SIZE			(1 << HASH_SHIFT)

static struct list devtab[TABLE_SIZE];
static int nr_device;
static DEFINE_RWLOCK(lock_devtab);
static DEFINE_SPINLOCK(nr_lock);

struct device *getdev(dev_t id)
{
	struct device *dev;
	struct list *head, *curr;

	head = &devtab[hash(id, HASH_SHIFT)];
	curr = head;

	do {
		read_lock(lock_devtab);
		curr = curr->next;
		read_unlock(lock_devtab);

		if (curr == head) {
			dev = NULL;
			break;
		}

		dev = get_container_of(curr, struct device, link);
	} while (dev->id != id);

	return dev;
}

void linkdev(dev_t id, struct device *new)
{
	struct list *head = &devtab[hash(id, HASH_SHIFT)];

	write_lock(lock_devtab);
	list_add(&new->link, head);
	write_unlock(lock_devtab);
}

#include <string.h>

struct device *mkdev(unsigned int major, unsigned int minor,
		struct file_operations *ops, const char *name)
{
	struct device *dev;
	dev_t id;

	if ((dev = kmalloc(sizeof(struct device))) == NULL)
		return NULL;

	if (!major) {
		if (nr_device >= MAJOR_MAX)
			goto err;

		unsigned int irqflag;
		spin_lock_irqsave(nr_lock, irqflag);
		nr_device += 1;
		spin_unlock_irqrestore(nr_lock, irqflag);

		major = nr_device;
	}

	if (!major || (major > nr_device))
		goto err;

	/* get a unique minor number
	 * checking if the same number registered already */
	do {
		id = SET_DEVID(major, minor);
		if (!getdev(id))
			break;

		minor++;
	} while (minor <= MINOR_MAX);

	if (minor > MINOR_MAX)
		goto err;

	dev->id = id;
	dev->count = 0;
	dev->op = ops;
	list_link_init(&dev->link);
	INIT_MUTEX(dev->lock);

	dev->block_size = 1;
	dev->nr_blocks = 0;
	dev->base_addr = 0;
	dev->buffer = NULL;

	linkdev(dev->id, dev);

	if (name && strcmp(name, "/"))
		sys_mknod(name, FT_DEV, dev->id);

	return dev;

err:
	kfree(dev);
	return NULL;
}

int remove_device(struct device *dev)
{
	return 0;
}

#include <kernel/init.h>

void __init device_init()
{
	extern char _device_list;
	unsigned int *p = (unsigned int *)&_device_list;

	while (*p)
		((unsigned int (*)())*p++)();
}

static void devtab_init()
{
	unsigned int i;
	for (i = 0; i < TABLE_SIZE; i++) {
		list_link_init(&devtab[i]);
	}

	nr_device = 0;
}
REGISTER_INIT(devtab_init, 10);

#ifdef CONFIG_DEBUG
void display_devtab()
{
	struct device *dev;
	struct list *head, *next;
	unsigned int i;

	for (i = 0; i < TABLE_SIZE; i++) {
		head = &devtab[i];
		next = head->next;

		printf("[%02d] ", i);
		while (next != head) {
			dev = get_container_of(next, struct device, link);
			printf("--> %d ", dev->id);

			next = next->next;
		}
		printf("\n");
	}
}
#endif
