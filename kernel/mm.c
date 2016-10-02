#include <kernel/page.h>
#include <kernel/task.h>
#include <kernel/init.h>
#include <kernel/lock.h>
#include <error.h>

extern char _ram_start, _ram_size, _ebss;

#ifdef CONFIG_DEBUG
unsigned int alloc_fail_count;
#endif

#ifdef CONFIG_PAGING
static struct buddy buddypool;

void *kmalloc(size_t size)
{
	struct page *page;

retry:
	page = alloc_pages(&buddypool,
			log2((ALIGN_PAGE(size)-1) >> PAGE_SHIFT));

	if (page)
		return page->addr;

	debug(MSG_DEBUG, "Low memory");

	if (kill_zombie())
		goto retry;

	debug(MSG_ERROR, "Out of memory");
#ifdef CONFIG_DEBUG
	alloc_fail_count++;
#endif

	return NULL;
}

void kfree(void *addr)
{
	struct page *page;
	struct buddy *pool = &buddypool;
	unsigned int index;

	if (!addr) return;

	index = PAGE_INDEX(buddypool.mem_map, addr);
	if (index >= pool->nr_pages) /* out of range */
		return;

	page = &pool->mem_map[PAGE_INDEX(buddypool.mem_map, addr)];

	/* already freed or no way to free if not allocated by buddy
	 * allocator */
	if (!(GET_PAGE_FLAG(page) & PAGE_BUDDY))
		return;

	free_pages(pool, page);
}

void __init free_bootmem()
{
	void *addr;
	unsigned index;
	struct page *page;

	index = buddypool.nr_pages - (ALIGN_PAGE(STACK_SIZE) >> PAGE_SHIFT);
	addr  = buddypool.mem_map[index].addr;
	page  = &buddypool.mem_map[index];
	SET_PAGE_FLAG(page, PAGE_BUDDY);

	kfree(addr);
}

size_t getfree()
{
	return show_buddy(&buddypool);
}

#else
#include <lib/firstfit.h>

static struct ff_freelist *mem_map;
static DEFINE_SPINLOCK(mem_lock);

void *kmalloc(size_t size)
{
	void *p;
	unsigned int irqflag;

retry:
	spin_lock_irqsave(&mem_lock, irqflag);
	p = ff_alloc(&mem_map, size);
	spin_unlock_irqrestore(&mem_lock, irqflag);

	if (p == NULL) {
		debug(MSG_DEBUG, "Low memory");

		if (kill_zombie())
			goto retry;

		debug(MSG_ERROR, "Out of memory");
	}

	return p;
}

void kfree(void *addr)
{
	unsigned int irqflag;

	if (!addr) return;

	spin_lock_irqsave(&mem_lock, irqflag);
	ff_free(&mem_map, addr);
	spin_unlock_irqrestore(&mem_lock, irqflag);
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

size_t getfree()
{
	return show_freelist(&mem_map);
}
#endif

void __init mm_init()
{
	unsigned int start = ALIGN_WORD(&_ram_start);
	unsigned int end   =
		(unsigned int)&_ram_start + (unsigned int)&_ram_size - 1;
#ifdef CONFIG_PAGING
	unsigned int nr_pages = PAGE_NR(end) - PAGE_NR(start) + 1;

	struct page *page = (struct page *)ALIGN_PAGE(&_ebss);

	debug(MSG_SYSTEM, "# Initializing memory");
	debug(MSG_SYSTEM, "page size %d bytes", PAGESIZE);
	debug(MSG_SYSTEM, "page struct size %d bytes", sizeof(struct page));
	debug(MSG_SYSTEM, "ram range 0x%08x - 0x%08x", start, end);
	debug(MSG_SYSTEM, "total %d pages", nr_pages);

	buddypool.mem_map = page;

	while (start < end) {
		page->flags = 0;
		page->addr  = (void *)start;
		list_link_init(&page->link);

		start += PAGESIZE;
		page++;
	}

	debug(MSG_SYSTEM, "0x%08x - mem_map first entry, page[%d]",
			(unsigned int)buddypool.mem_map,
			PAGE_INDEX(buddypool.mem_map, buddypool.mem_map));
	debug(MSG_SYSTEM, "0x%08x - mem_map last entry, page[%d]",
			(unsigned int)page - 1,
			PAGE_INDEX(buddypool.mem_map, (page-1)->addr));
	debug(MSG_SYSTEM, "mem_map size : %d bytes",
			(unsigned int)page - (unsigned int)buddypool.mem_map);

	buddy_init(&buddypool, nr_pages, buddypool.mem_map);

	debug(MSG_SYSTEM, "%d pages, %d bytes free\n",
			getfree(), getfree() * PAGESIZE);
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
