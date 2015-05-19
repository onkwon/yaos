#include <kernel/page.h>
#include <stdlib.h>

#include <asm/init.h>

extern char _mem_start, _mem_end, _ebss;

#ifdef CONFIG_PAGING
struct page_t *mem_map;

extern struct buddypool_t buddypool;

void *kmalloc(unsigned long size)
{
	struct page_t *page;
	unsigned long irq_flag;

	spin_lock_irqsave(buddypool.lock, irq_flag);
	page = alloc_pages(&buddypool,
			log2((ALIGN_PAGE(size)-1) >> PAGE_SHIFT));
	spin_unlock_irqrestore(buddypool.lock, irq_flag);

	if (page)
		return page->addr;

	return NULL;
}

void kfree(void *addr)
{
	struct page_t *page;
	struct buddypool_t *pool = &buddypool;
	unsigned long index;
	unsigned long irq_flag;

	if (!addr) return;

	index = PAGE_INDEX(addr);
	if (index >= pool->nr_pages) /* out of range */
		return;

	page = &mem_map[PAGE_INDEX(addr)];

	/* if not allocated by buddy allocator there is no way to free.
	 * Wrong address or where must not be free. */
	if (!(GET_PAGE_FLAG(page) & PAGE_BUDDY))
		return;

	spin_lock_irqsave(pool->lock, irq_flag);
	free_pages(pool, page);
	spin_unlock_irqrestore(pool->lock, irq_flag);
}

void __init free_bootmem()
{
	void *addr;
	unsigned index;

	index = buddypool.nr_pages -
		(ALIGN_PAGE(DEFAULT_STACK_SIZE) >> PAGE_SHIFT);
	addr  = mem_map[index].addr;

	kfree(addr);
}

#else
static struct ff_freelist_t *mem_map;
static spinlock_t mem_lock;

void *kmalloc(unsigned long size)
{
	void *p;
	unsigned long irq_flag;

	spin_lock_irqsave(mem_lock, irq_flag);
	p = ff_alloc(&mem_map, size);
	spin_unlock_irqrestore(mem_lock, irq_flag);

	return p;
}

void kfree(void *addr)
{
	unsigned long irq_flag;

	spin_lock_irqsave(mem_lock, irq_flag);
	ff_free(&mem_map, addr);
	spin_unlock_irqrestore(mem_lock, irq_flag);
}

void __init free_bootmem()
{
	struct ff_freelist_t *p;

	p = (void *)ALIGN_WORD((unsigned long)&_mem_end -
			(DEFAULT_STACK_SIZE + sizeof(struct ff_freelist_t)));

	kfree(p->addr);
}
#endif /* CONFIG_PAGING */

void mm_init()
{
	unsigned long start = ALIGN_WORD(&_mem_start);
	unsigned long end   = (unsigned long)&_mem_end;
#ifdef CONFIG_PAGING
	unsigned long nr_pages = PAGE_NR(end) - PAGE_NR(start) + 1;

	extern struct page_t *mem_map;
	extern struct buddypool_t buddypool;

	struct page_t *page = (struct page_t *)ALIGN_PAGE(&_ebss);

	mem_map = page;

	while (start < end) {
		page->flags = 0;
		page->addr  = (void *)start;
		LIST_LINK_INIT(&page->link);

		start += PAGE_SIZE;
		page++;
	}

	buddypool.nr_pages = nr_pages;
	buddypool.nr_free  = 0;
	buddy_init(&buddypool, nr_pages, mem_map);
#else
	struct ff_freelist_t *p;

	/* preserve initial kernel stack to be free later */
	p = (struct ff_freelist_t *)ALIGN_WORD(end -
			(DEFAULT_STACK_SIZE + sizeof(struct ff_freelist_t)));
	p->addr = (void *)((unsigned long)p + sizeof(struct ff_freelist_t));
	p->size = ALIGN_WORD(DEFAULT_STACK_SIZE);

	/* mark kernel .data and .bss sections as used */
	mem_map = (struct ff_freelist_t *)&_ebss;
	mem_map->size = (unsigned long)p -
		((unsigned long)&_ebss + sizeof(struct ff_freelist_t));
	mem_map->addr = (void *)((unsigned long)mem_map +
			sizeof(struct ff_freelist_t));
	LIST_LINK_INIT(&mem_map->list);

	spinlock_init(mem_lock);
#endif

	/* alloc kernel stack for init */
	unsigned long *kstack = (unsigned long *)
		kmalloc(KERNEL_STACK_SIZE * sizeof(long));

	init.stack.kernel = &kstack[KERNEL_STACK_SIZE-1];
}
