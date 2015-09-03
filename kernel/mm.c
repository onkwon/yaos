#include <kernel/page.h>
#include <kernel/task.h>
#include <kernel/init.h>
#include <kernel/lock.h>
#include <error.h>

extern char _ram_start, _ram_size, _ebss;

#ifdef CONFIG_PAGING
struct page *mem_map;

extern struct buddy buddypool;

void *kmalloc(size_t size)
{
	struct page *page;
	unsigned int irqflag;

	spin_lock_irqsave(buddypool.lock, irqflag);
	page = alloc_pages(&buddypool,
			log2((ALIGN_PAGE(size)-1) >> PAGE_SHIFT));
	spin_unlock_irqrestore(buddypool.lock, irqflag);

	debug("kmalloc : %x %x", page, page->addr);

	if (page)
		return page->addr;

	debug("Out of memory");

	return NULL;
}

void kfree(void *addr)
{
	struct page *page;
	struct buddy *pool = &buddypool;
	unsigned int index;
	unsigned int irqflag;

	if (!addr) return;

	index = PAGE_INDEX(addr);
	if (index >= pool->nr_pages) /* out of range */
		return;

	page = &mem_map[PAGE_INDEX(addr)];

	/* If not allocated by buddy allocator there is no way to free. */
	if (!(GET_PAGE_FLAG(page) & PAGE_BUDDY))
		return;

	debug("kfree : %x", addr);

	spin_lock_irqsave(pool->lock, irqflag);
	free_pages(pool, page);
	spin_unlock_irqrestore(pool->lock, irqflag);

	addr = NULL;
}

void __init free_bootmem()
{
	void *addr;
	unsigned index;
	struct page *page;

	index = buddypool.nr_pages - (ALIGN_PAGE(STACK_SIZE) >> PAGE_SHIFT);
	addr  = mem_map[index].addr;
	page  = &mem_map[index];
	SET_PAGE_FLAG(page, PAGE_BUDDY);

	kfree(addr);
}

#else
#include <lib/firstfit.h>

static struct ff_freelist *mem_map;
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

	if (!addr) return;

	spin_lock_irqsave(mem_lock, irqflag);
	ff_free(&mem_map, addr);
	spin_unlock_irqrestore(mem_lock, irqflag);

	addr = NULL;
}

void __init free_bootmem()
{
	struct ff_freelist *p;
	unsigned int ram_end;

	ram_end = (unsigned int)&_ram_start + (unsigned int)&_ram_size - 1;

	p = (void *)ALIGN_WORD(ram_end -
			(STACK_SIZE + sizeof(struct ff_freelist)));

	kfree(p->addr);
}
#endif

void __init mm_init()
{
	unsigned int start = ALIGN_WORD(&_ram_start);
	unsigned int end   =
		(unsigned int)&_ram_start + (unsigned int)&_ram_size - 1;
#ifdef CONFIG_PAGING
	unsigned int nr_pages = PAGE_NR(end) - PAGE_NR(start) + 1;

	extern struct page *mem_map;
	extern struct buddy buddypool;

	struct page *page = (struct page *)ALIGN_PAGE(&_ebss);

	debug("Memory initiailization");
	debug("page size %d bytes", PAGE_SIZE);
	debug("page struct size %d bytes", sizeof(struct page));
	debug("ram range 0x%08x - 0x%08x", start, end);
	debug("total %d pages", nr_pages);

	mem_map = page;

	while (start < end) {
		page->flags = 0;
		page->addr  = (void *)start;
		list_link_init(&page->link);

		start += PAGE_SIZE;
		page++;
	}

	debug("0x%08x - mem_map first entry, page[%d]", mem_map,
			PAGE_INDEX(mem_map));
	debug("0x%08x - mem_map last entry, page[%d]", page - 1,
			PAGE_INDEX((page-1)->addr));
	debug("mem_map size : %d bytes", page - mem_map);

	buddypool.nr_pages = nr_pages;
	buddypool.nr_free  = 0;
	buddy_init(&buddypool, nr_pages, mem_map);
#else
	struct ff_freelist *p;

	/* preserve initial kernel stack to be free later */
	p = (struct ff_freelist *)ALIGN_WORD(end -
			(STACK_SIZE + sizeof(struct ff_freelist)));
	p->addr = (void *)((unsigned int)p + sizeof(struct ff_freelist));
	p->size = ALIGN_WORD(STACK_SIZE);

	/* mark kernel .data and .bss sections as used */
	mem_map = (struct ff_freelist *)&_ebss;
	mem_map->size = (unsigned int)p -
		((unsigned int)&_ebss + sizeof(struct ff_freelist));
	mem_map->addr = (void *)((unsigned int)mem_map +
			sizeof(struct ff_freelist));
	list_link_init(&mem_map->list);
#endif
}

void *sys_brk(size_t size)
{
	return kmalloc(size);
}
