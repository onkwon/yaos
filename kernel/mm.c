#include <kernel/page.h>
#include <kernel/task.h>
#include <kernel/init.h>

extern char _mem_start, _mem_end, _ebss;

#ifdef CONFIG_PAGING
struct page_t *mem_map;

extern struct buddy_t buddypool;

void *kmalloc(size_t size)
{
	struct page_t *page;
	unsigned int irqflag;

	spin_lock_irqsave(buddypool.lock, irqflag);
	page = alloc_pages(&buddypool,
			log2((ALIGN_PAGE(size)-1) >> PAGE_SHIFT));
	spin_unlock_irqrestore(buddypool.lock, irqflag);

	if (page)
		return page->addr;

	return NULL;
}

void kfree(void *addr)
{
	struct page_t *page;
	struct buddy_t *pool = &buddypool;
	unsigned int index;
	unsigned int irqflag;

	if (!addr) return;

	index = PAGE_INDEX(addr);
	if (index >= pool->nr_pages) /* out of range */
		return;

	page = &mem_map[PAGE_INDEX(addr)];

	/* if not allocated by buddy allocator there is no way to free.
	 * Wrong address or where must not be free. */
	if (!(GET_PAGE_FLAG(page) & PAGE_BUDDY))
		return;

	spin_lock_irqsave(pool->lock, irqflag);
	free_pages(pool, page);
	spin_unlock_irqrestore(pool->lock, irqflag);
}

void __init free_bootmem()
{
	void *addr;
	unsigned index;

	index = buddypool.nr_pages - (ALIGN_PAGE(STACK_SIZE) >> PAGE_SHIFT);
	addr  = mem_map[index].addr;

	kfree(addr);
}

#else
#include <lib/firstfit.h>

static struct ff_freelist_t *mem_map;
static DEFINE_SPINLOCK(mem_lock);

void *kmalloc(size_t size)
{
	void *p;
	unsigned int irqflag;

	spin_lock_irqsave(mem_lock, irqflag);
	p = ff_alloc(&mem_map, size);
	spin_unlock_irqrestore(mem_lock, irqflag);

	return p;
}

void kfree(void *addr)
{
	unsigned int irqflag;

	spin_lock_irqsave(mem_lock, irqflag);
	ff_free(&mem_map, addr);
	spin_unlock_irqrestore(mem_lock, irqflag);
}

void __init free_bootmem()
{
	struct ff_freelist_t *p;

	p = (void *)ALIGN_WORD((unsigned int)&_mem_end -
			(STACK_SIZE + sizeof(struct ff_freelist_t)));

	kfree(p->addr);
}
#endif /* CONFIG_PAGING */

void __init mm_init()
{
	unsigned int start = ALIGN_WORD(&_mem_start);
	unsigned int end   = (unsigned int)&_mem_end;
#ifdef CONFIG_PAGING
	unsigned int nr_pages = PAGE_NR(end) - PAGE_NR(start) + 1;

	extern struct page_t *mem_map;
	extern struct buddy_t buddypool;

	struct page_t *page = (struct page_t *)ALIGN_PAGE(&_ebss);

	mem_map = page;

	while (start < end) {
		page->flags = 0;
		page->addr  = (void *)start;
		list_link_init(&page->link);

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
			(STACK_SIZE + sizeof(struct ff_freelist_t)));
	p->addr = (void *)((unsigned int)p + sizeof(struct ff_freelist_t));
	p->size = ALIGN_WORD(STACK_SIZE);

	/* mark kernel .data and .bss sections as used */
	mem_map = (struct ff_freelist_t *)&_ebss;
	mem_map->size = (unsigned int)p -
		((unsigned int)&_ebss + sizeof(struct ff_freelist_t));
	mem_map->addr = (void *)((unsigned int)mem_map +
			sizeof(struct ff_freelist_t));
	list_link_init(&mem_map->list);
#endif
}
