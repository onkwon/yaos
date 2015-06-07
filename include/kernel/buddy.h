#ifndef __BUDDY_H__
#define __BUDDY_H__

#include <types.h>

#define BUDDY_MAX_ORDER		10

struct buddy_freelist {
	struct list list;
	unsigned int *bitmap;
};

struct buddy {
	struct buddy_freelist free[BUDDY_MAX_ORDER];

	unsigned int nr_free;
	unsigned int nr_pages;

	lock_t lock;
};

#include <kernel/page.h>

struct page *alloc_pages(struct buddy *pool, unsigned int order);
void free_pages(struct buddy *pool, struct page *page);
void buddy_init(struct buddy *pool, unsigned int nr_pages,
		struct page *array);

static inline unsigned int log2(unsigned int v)
{
	unsigned int i;
	for (i = 0; v; i++) v >>= 1;
	return i;
}

#ifdef CONFIG_DEBUG
void show_buddy(void *zone);
#else
#define show_buddy(nul)
#endif

#endif /* __PAGE_H__ */
