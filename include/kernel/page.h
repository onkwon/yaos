#ifndef __PAGE_H__
#define __PAGE_H__

#include <types.h>

#include <asm/page.h>

#ifndef PAGE_SHIFT
#define PAGE_SHIFT		12 /* 4096 bytes */
#endif

#ifdef CONFIG_PAGING
#define PAGESIZE		(1UL << PAGE_SHIFT)
#else
#define PAGESIZE		1
#endif

struct page {
	unsigned int flags;
	void *addr; /* TODO: remove unnecessary addr field */
	struct links list;
};

#define ALIGN_PAGE(x)		\
	( (ALIGN_WORD(x) + PAGESIZE-1) & ~(PAGESIZE-1) )
#define PAGE_NR(x)		((unsigned int)(x) >> PAGE_SHIFT)

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

#define GET_PAGE_ORDER(p)	\
	(((p)->flags & PAGE_ORDER_MASK) >> PAGE_ORDER_BIT)
#define SET_PAGE_ORDER(p, n)	\
	((p)->flags = ((p)->flags & ~PAGE_ORDER_MASK) | (n << PAGE_ORDER_BIT))

void *kmalloc(size_t size);
void kfree(void *addr);

void free_bootmem();
size_t getfree();
size_t heap_init(void *pool, void *start, void *end);

void *sys_brk(size_t size);

#include <kernel/buddy.h>

#endif /* __PAGE_H__ */
