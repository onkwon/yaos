#ifndef __BUDDY_H__
#define __BUDDY_H__

#include <types.h>
#include <kernel/lock.h>

#define BUDDY_MAX_ORDER		10

struct buddy_freelist_t {
	struct list_t list;
	unsigned long *bitmap;
};

struct buddypool_t {
	struct buddy_freelist_t free[BUDDY_MAX_ORDER];

	unsigned long nr_free;
	unsigned long nr_pages;

	spinlock_t lock;
};

#include <kernel/page.h>

struct page_t *alloc_pages(struct buddypool_t *pool, unsigned long order);
void free_pages(struct buddypool_t *pool, struct page_t *page);
void buddy_init(struct buddypool_t *pool, unsigned long nr_pages,
		struct page_t *array);

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
