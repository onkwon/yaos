#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <types.h>
#include <kernel/lock.h>

struct buffer_cache {
	unsigned int nblock;
	char *buf;
	size_t size;
	bool dirty;
	struct list list;
	mutex_t mutex;
};

buf_t *request_buffer(unsigned short int n, unsigned short int block_size);
void release_buffer(buf_t *head);

#include <kernel/device.h>

void *getblk_lock(unsigned int nblock_new, struct device *dev);
void *getblk(unsigned int nblock, struct device *dev);
void putblk_unlock(unsigned int nblock, struct device *dev);
void updateblk(unsigned int nblock, struct device *dev);

int __sync(struct device *dev);

#endif /* __BUFFER_H__ */
