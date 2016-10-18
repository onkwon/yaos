#include <kernel/page.h>
#include <kernel/task.h>
#include <kernel/init.h>
#include <kernel/lock.h>
#include <error.h>
#include <bitops.h>

extern char _ram_start, _ram_size, _ebss;

#ifdef CONFIG_PAGING
static struct buddy buddy;

#define addr2page(addr, node)		(&(node)->mem_map[addr2idx(addr, node)])

static inline unsigned int addr2idx(void *addr, struct buddy *node)
{
	return PAGE_NR((unsigned int)addr -
			(unsigned int)(node->mem_map->addr));
}

void *kmalloc(size_t size)
{
	struct page *page;

	if (!size)
		return NULL;

	int order;
	unsigned int nr_pages;

	nr_pages = ALIGN_PAGE(size) >> PAGE_SHIFT;
	order = log2(nr_pages);
	order = order + !!(nr_pages - (1 << order)); /* round-up to the next
							order */

retry:
	page = alloc_pages(&buddy, order);

	if (page)
		return page->addr;

	debug(MSG_DEBUG, "Low memory");

	if (kill_zombie())
		goto retry;

	debug(MSG_ERROR, "Out of memory");

	return NULL;
}

void kfree(void *addr)
{
	struct page *page;
	struct buddy *node = &buddy;

	if (!addr) return;

	page = addr2page(addr, node);

	if (GET_PAGE_FLAG(page) & PAGE_BUDDY) /* already in buddy list */
		return;

	free_pages(node, page);
}

void __init free_bootmem()
{
	unsigned int idx;
	struct page *page;

	idx = buddy.nr_pages - PAGE_NR(ALIGN_PAGE(STACK_SIZE));
	page = &buddy.mem_map[idx];

	kfree(page->addr);
}

size_t getfree()
{
#ifdef CONFIG_DEBUG
	return show_buddy_all(&buddy) * PAGESIZE;
#else
	return buddy.nr_free * PAGESIZE;
#endif
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
	/* assume flat memory */
	unsigned int nr_pages = PAGE_NR(end) - PAGE_NR(start) + 1;

	struct page *page = (struct page *)ALIGN_PAGE(&_ebss);

	debug(MSG_SYSTEM, "# Initializing memory");
	debug(MSG_SYSTEM, "page size %d bytes", PAGESIZE);
	debug(MSG_SYSTEM, "page struct size %d bytes", sizeof(struct page));
	debug(MSG_SYSTEM, "ram range 0x%08x - 0x%08x", start, end);
	debug(MSG_SYSTEM, "total %d pages", nr_pages);

	buddy.mem_map = page;

	while (start < end) {
		page->flags = 0;
		page->addr  = (void *)start;
		links_init(&page->list);

		start += PAGESIZE;
		page++;
	}

	debug(MSG_SYSTEM, "0x%08x - mem_map first entry, page[%d]",
			(unsigned int)buddy.mem_map,
			addr2idx(buddy.mem_map, &buddy));
	debug(MSG_SYSTEM, "0x%08x - mem_map last entry, page[%d]",
			(unsigned int)page - 1,
			addr2idx((page-1)->addr, &buddy));
	debug(MSG_SYSTEM, "mem_map size : %d bytes",
			(unsigned int)page - (unsigned int)buddy.mem_map);

	buddy_init(&buddy, nr_pages, buddy.mem_map);
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
	links_init(&mem_map->list);
#endif
}

void *sys_brk(size_t size)
{
	return kmalloc(size);
}
