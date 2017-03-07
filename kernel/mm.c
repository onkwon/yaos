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

	nr_pages = (size - 1) >> PAGE_SHIFT;
	order = fls(nr_pages);

retry:
	page = alloc_pages(&buddy, order);

	if (page)
		return page->addr;

	warn("Low memory");

	if (kill_zombie())
		goto retry;

	error("Out of memory");

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

static heap_t mem_map;
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
		warn("Low memory");

		if (kill_zombie())
			goto retry;

		error("Out of memory");
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

	p = (void *)ALIGN_WORD(ram_end - (STACK_SIZE + FF_METASIZE));

	kfree((char *)p + FF_DATA_OFFSET);
}

size_t getfree()
{
	return show_freelist(&mem_map);
}
#endif

void __init mm_init()
{
	unsigned int end =
		(unsigned int)&_ram_start + (unsigned int)&_ram_size - 1;
#ifdef CONFIG_PAGING
	unsigned int start = ALIGN_WORD(&_ram_start);

	/* assume flat memory */
	unsigned int nr_pages = PAGE_NR(end) - PAGE_NR(start) + 1;

	struct page *page = (struct page *)ALIGN_PAGE(&_ebss);

	notice("# Initializing memory\n"
	       "page size %d bytes\n"
	       "page struct size %d bytes\n"
	       "ram range 0x%08x - 0x%08x\n"
	       "total %d pages",
	       PAGESIZE,
	       sizeof(struct page),
	       start, end,
	       nr_pages);

	buddy.mem_map = page;

	while (start < end) {
		page->flags = 0;
		page->addr  = (void *)start;
		links_init(&page->list);

		start += PAGESIZE;
		page++;
	}

	notice("0x%08x - mem_map first entry, page[%d]\n"
	       "0x%08x - mem_map last entry, page[%d]\n"
	       "mem_map size : %d bytes",
	       (unsigned int)buddy.mem_map, addr2idx(buddy.mem_map, &buddy),
	       (unsigned int)page - 1, addr2idx((page-1)->addr, &buddy),
	       (unsigned int)page - (unsigned int)buddy.mem_map);

	buddy_init(&buddy, nr_pages, buddy.mem_map);
#else
	struct ff_freelist *p;

	/* preserve initial kernel stack to be free later */
	p = (struct ff_freelist *)ALIGN_WORD(end - (STACK_SIZE + FF_METASIZE));
	p->size = ALIGN_WORD(STACK_SIZE);
	FF_LINK_HEAD(p);
	FF_MARK_ALLOCATED(p);

	/* mark kernel .data and .bss sections as used */
	heap_init(&mem_map, &_ebss, p);
#endif
}

int heap_init(void *pool, void *start, void *end)
{
	return ff_freelist_init(pool, start, end);
}

void *sys_brk(size_t size)
{
	return kmalloc(size);
}
