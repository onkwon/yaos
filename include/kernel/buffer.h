#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <types.h>
#include <kernel/lock.h>

struct buffer_cache {
	unsigned int nblock;
	char *buf;
	bool dirty;
	struct list list;
	mutex_t lock;
};

buf_t *request_buffer(unsigned short int n, unsigned short int block_size);
void release_buffer(buf_t *head);

#include <kernel/device.h>

void *getblk_lock(unsigned int nblock_new, const struct device *dev);
void *getblk(unsigned int nblock, const struct device *dev);
void putblk_unlock(unsigned int nblock, const struct device *dev);
void updateblk(unsigned int nblock, const struct device *dev);

#endif /* __BUFFER_H__ */
