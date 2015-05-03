#ifndef __MM_H__
#define __MM_H__

#include <asm/mm.h>

#ifndef PAGE_SHIFT
#define PAGE_SHIFT		12
#endif
#define PAGE_SIZE		(1UL << PAGE_SHIFT)
#define PAGE_NR(x)		((unsigned long)(x) >> PAGE_SHIFT)
#define PAGE_INDEX(x) \
	(PAGE_NR((unsigned long)(x) - (unsigned long)mem_map->addr))
#define PAGE_ALIGN(x) \
	( ((unsigned long)(x) + PAGE_SIZE-1) & ~(PAGE_SIZE-1) )
#define ALIGN_LONG(x) \
	( ((x) + sizeof(long)-1) & ~(sizeof(long)-1) )

#include <types.h>

struct page_t {
	unsigned long flags; /* the lowest PG_ORDER_BIT for buddy order */
	void *addr;
	struct list_t link;
};

#define BUDDY_MAX_ORDER		10

/* macros related to buddy */
#define PG_ORDER_BIT		4 /* buddy order */
#define GET_PAGE_ORDER(p)	((p)->flags & ((1 << PG_ORDER_BIT) - 1))
#define CLEAR_PAGE_ORDER(p)	((p)->flags &= ~((1 << PG_ORDER_BIT) - 1))
#define SET_PAGE_ORDER(p, n) { \
	CLEAR_PAGE_ORDER(p); \
	(p)->flags |= n; \
}

struct free_area_t {
	struct list_t free_list;
	unsigned long *bitmap;
};

#include <lock.h>

struct zone_t {
	struct free_area_t free_area[BUDDY_MAX_ORDER];
	unsigned long nr_free;
	unsigned long nr_pages;
	spinlock_t lock;
};

void mm_init();

void *kmalloc(unsigned long size);
void free(void *addr);

void show_free_list(struct zone_t *zone);

#endif /* __MM_H__ */
