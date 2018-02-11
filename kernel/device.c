/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#include <kernel/device.h>
#include <kernel/page.h>
#include <kernel/lock.h>
#include <hash.h>
#include <error.h>

#define HASH_SHIFT			4
#define TABLE_SIZE			(1 << HASH_SHIFT)

static struct links devtab[TABLE_SIZE];
static int nr_device;
static DEFINE_RWLOCK(lock_devtab);

struct device *getdev(dev_t id)
{
	struct device *dev;
	struct links *head, *curr;

	head = &devtab[hash(id, HASH_SHIFT)];
	curr = head;

	do {
		read_lock(&lock_devtab);
		curr = curr->next;
		read_unlock(&lock_devtab);

		if (curr == head) {
			dev = NULL;
			break;
		}

		dev = get_container_of(curr, struct device, list);
	} while (dev->id != id);

	return dev;
}

void linkdev(dev_t id, struct device *new)
{
	struct links *head = &devtab[hash(id, HASH_SHIFT)];

	write_lock(&lock_devtab);
	links_add(&new->list, head);
	write_unlock(&lock_devtab);
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

		nr_device += 1;
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
	dev->refcount = 0;
	dev->op = ops;
	links_init(&dev->list);
	mutex_init(&dev->mutex);

	dev->block_size = 1;
	dev->nr_blocks = 0;
	dev->base_addr = 0;
	dev->buffer = NULL;

	linkdev(dev->id, dev);

	if (name && strcmp(name, "/"))
		sys_mknod(name, FT_DEV, dev->id);

	debug("%s registerd at %d:%d", name, major, minor);

	return dev;

err:
	kfree(dev);
	error("failed to register the device, %s at %d:%d", name, major, minor);

	return NULL;
}

int remove_device(struct device *dev)
{
	(void)dev;
	return 0;
}

#include <kernel/init.h>

void __init driver_init()
{
	extern char _driver_list;
	unsigned int *p = (unsigned int *)&_driver_list;

	while (*p)
		((unsigned int (*)())*p++)();
}

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
		links_init(&devtab[i]);
	}

	nr_device = 0;
}
REGISTER_INIT(devtab_init, 2);

#include <kernel/buffer.h>

void device_sync_all()
{
	struct device *dev;
	struct links *head, *next;
	unsigned int i;

	for (i = 0; i < TABLE_SIZE; i++) {
		head = &devtab[i];
		next = head->next;

		while (next != head) {
			dev = get_container_of(next, struct device, list);

			__sync(dev);

			next = next->next;
		}
	}
}

#ifdef CONFIG_DEBUG
void display_devtab()
{
	struct device *dev;
	struct links *head, *next;
	unsigned int i;

	for (i = 0; i < TABLE_SIZE; i++) {
		head = &devtab[i];
		next = head->next;

		printf("[%02d] ", i);
		while (next != head) {
			dev = get_container_of(next, struct device, list);
			printf("--> %d ", dev->id);

			next = next->next;
		}
		printf("\n");
	}
}
#endif
