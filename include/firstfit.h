#ifndef __FIRSTFIT_H__
#define __FIRSTFIT_H__

#include <types.h>

#include <kernel/page.h>

struct ff_freelist_t {
	void *addr;
	unsigned long size;
	struct list_t list;
};

struct ff_freelist_t *ff_freelist_init(void *start, void *end);

void *ff_alloc(void *freelist, unsigned long size);
void ff_free(void *freelist, void *addr);

#ifdef CONFIG_DEBUG
void show_freelist(void *pool);
#else
#define show_freelist(nul)
#endif

#endif /* __FIRSTFIT_H__ */
