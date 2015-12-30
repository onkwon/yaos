#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <types.h>
#include <fs/fs.h>

#define MINOR_BITS		16
#define MAJOR_MASK		((1 << MINOR_BITS) - 1)
#define MAJOR_MAX		MAJOR_MASK
#define MINOR_MAX		((1 << (WORD_BITS - MINOR_BITS)) - 1)

#define MAJOR(id)		((id) & MAJOR_MASK)
#define MINOR(id)		((id) >> MINOR_BITS)
#define SET_MINOR(mi)		((mi) << MINOR_BITS)
#define SET_DEVID(ma, mi)	(MAJOR(ma) | SET_MINOR(mi))

struct device {
	dev_t id; /* major & minor */
	refcnt_t count;

	mutex_t lock; /* to synchronize calls to its driver */

	struct file_operations *op; /* remove this!! */

	struct list link; /* for device hash list */

	/* block device only */
	unsigned short int block_size;
	unsigned int nr_blocks;
	unsigned int base_addr; /* or base block number */
	buf_t *buffer;
};

int remove_device(struct device *dev);
struct device *mkdev(unsigned int major, unsigned int minor,
		struct file_operations *ops, const char *name);
struct device *getdev(dev_t id);
void linkdev(dev_t id, struct device *dev);
void device_init();
void device_sync_all();

#ifdef CONFIG_DEBUG
void display_devtab();
#else
#define display_devtab()
#endif

#endif /* __DEVICE_H__ */
