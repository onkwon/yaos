#ifndef __FIRSTFIT_H__
#define __FIRSTFIT_H__

#include <types.h>

struct ff_freelist {
	void *addr;
	size_t size;
	struct links list;
};

struct ff_freelist *ff_freelist_init(void *start, void *end);

void *ff_alloc(void *freelist, size_t size);
void ff_free(void *freelist, void *addr);
size_t show_freelist(void *pool);

#endif /* __FIRSTFIT_H__ */
