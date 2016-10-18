#ifndef __BUDDY_H__
#define __BUDDY_H__

#include <types.h>
#include <kernel/page.h>

#define BUDDY_MAX_ORDER		10

struct buddy_freelist {
	struct links list;
	size_t nr_pageblocks;
};

struct buddy {
	struct buddy_freelist freelist[BUDDY_MAX_ORDER];
	size_t nr_pages;
	size_t nr_free;
	lock_t lock;
	struct page *mem_map;
};

struct page *alloc_pages(struct buddy *node, unsigned int order);
void free_pages(struct buddy *node, struct page *page);
void buddy_init(struct buddy *node, size_t nr_pages, struct page *array);
size_t show_buddy_all(struct buddy *node);

#endif /* __PAGE_H__ */
