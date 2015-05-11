#ifndef __PAGE_H__
#define __PAGE_H__

#ifdef CONFIG_PAGING
#include <asm/page.h>

#ifndef PAGE_SHIFT
#define PAGE_SHIFT		12
#endif
#define PAGE_SIZE		(1UL << PAGE_SHIFT)
#define PAGE_NR(x)		((unsigned long)(x) >> PAGE_SHIFT)
#define PAGE_INDEX(x)		\
	(PAGE_NR((unsigned long)(x) - (unsigned long)mem_map->addr))
#define ALIGN_PAGE(x)		\
	( ((unsigned long)(x) + PAGE_SIZE-1) & ~(PAGE_SIZE-1) )

#define PAGE_FLAG_BIT		0
#define PAGE_ORDER_BIT		12
#define PAGE_RSVD_BIT		16
#define PAGE_FLAG_MASK		((1 << PAGE_ORDER_BIT) - 1)
#define PAGE_ORDER_MASK		(((1 << PAGE_RSVD_BIT) - 1) & ~PAGE_FLAG_MASK)

/* page flags */
#define PAGE_BUDDY		1	

#define PAGE_FLAG_POS(bit)	((bit) << PAGE_FLAG_BIT)
#define GET_PAGE_FLAG(p)	(((p)->flags & PAGE_FLAG_MASK) >> PAGE_FLAG_BIT)
#define SET_PAGE_FLAG(p, bit)	((p)->flags |= PAGE_FLAG_POS(bit))
#define RESET_PAGE_FLAG(p, bit)	((p)->flags &= ~PAGE_FLAG_POS(bit))

#include <types.h>

struct page_t {
	unsigned long flags;
	void *addr;
	struct list_t link;
} __attribute__((packed));

/* buddy order */
#define BUDDY_MAX_ORDER		10

#define GET_PAGE_ORDER(p)	\
	(((p)->flags & PAGE_ORDER_MASK) >> PAGE_ORDER_BIT)
#define SET_PAGE_ORDER(p, n)	\
	((p)->flags = ((p)->flags & ~PAGE_ORDER_MASK) | (n << PAGE_ORDER_BIT))

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

void free_area_init(struct zone_t *zone, unsigned long nr_pages,
		struct page_t *array);

void *kmalloc(unsigned long size);
void free(void *addr);
#endif /* CONFIG_PAGING */

void mm_init();

#ifdef CONFIG_DEBUG
void show_free_list(void *area);
#else
#define show_free_list(nul)
#endif

void free_bootmem();

#endif /* __PAGE_H__ */
