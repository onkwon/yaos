#include <kernel/mm.h>
#include <stdlib.h>

struct page_t *mem_map;
struct zone_t dzone;

unsigned long nr_avail_pages, nr_free_pages;

#define BITMAP_POS(bitmap, page_idx, order) \
	(bitmap + (page_idx >> (order + 5 + 1)))
	/* `+5` for dividing by 32
	 * == `(bitmap + ((page_idx >> order) / (sizeof(long)*8)))`
	 * while `+1` for dividing by 2 */
#define BITMAP_OFFSET(page_idx, order) \
	(1 << ((page_idx >> (order+1)) & (sizeof(long)*8-1)))
#define BITMAP_TOGGLE(bitmap, page_idx, order) \
	(*BITMAP_POS(bitmap, page_idx, order) ^= BITMAP_OFFSET(page_idx, order))
#define BITMAP_MASK(bitmap, page_idx, order) \
	(*BITMAP_POS(bitmap, page_idx, order) & BITMAP_OFFSET(page_idx, order))

void free_pages(struct zone_t *zone, struct page_t *page)
{
	struct page_t *node = mem_map;
	struct page_t *buddy;
	struct free_area_t *area;
	unsigned long order, index;

	if (!page) return;

	order = GET_PAGE_ORDER(page);
	area  = &zone->free_area[order];
	index = PAGE_INDEX(page->addr);

	nr_free_pages += 1 << order;

	while (order < BUDDY_MAX_ORDER) {
		BITMAP_TOGGLE(area->bitmap, index, order);
		/* If it was 0 before toggling, it means both of buddies
		 * were allocated. So nothing can be merged to upper order
		 * as one of them is still allocated. Otherwise, both of 
		 * them are free now. Therefore merge it to upper order. */
		if (BITMAP_MASK(area->bitmap, index, order))
			break;

		/* find buddy and detach it from current order to merge */
		buddy = &node[index ^ (1<<order)];
		list_del(&buddy->link);

		order++;
		area++;

		/* grab the first address of merging buddies */
		index &= ~0 << order;
	}

	list_add(&node[index].link, &area->free_list);
}

#include <foundation.h>

void show_free_list(struct zone_t *zone)
{
	struct page_t *page;
	struct list_t *p;
	int i, j, size;

	if (!zone) zone = &dzone;

	printf("available pages %d\nfree pages %d\n", nr_avail_pages, nr_free_pages);

	for (i = 0; i < BUDDY_MAX_ORDER; i++) {
		printf("============= order %02d =============\n", i);
		p = zone->free_area[i].free_list.next;
		while (p != &zone->free_area[i].free_list) {
			page = get_container_of(p, struct page_t, link); 
			printf("--> 0x%08x(0x%08x) ", page->addr, page);
			p = p->next;
		}

		printf("\n------ bitmap 0x%08x ------\n", zone->free_area[i].bitmap);
		size = ALIGN_LONG(nr_avail_pages) >> (4 + i);
		size = ALIGN_LONG(size);
		size = size? size : sizeof(long);
		size /= sizeof(long);
		for (j = size; j; j--) {
			printf("%08x ", *(zone->free_area[i].bitmap + j - 1));
			if (!((size-j+1) % 8))
				printf("\n");
		}
		printf("\n");
	}
}

struct page_t *alloc_pages(struct zone_t *zone, unsigned long order)
{
	struct free_area_t *area = &zone->free_area[order];
	struct list_t *head, *curr;
	struct page_t *page;
	unsigned long i;

	for (i = order; i < BUDDY_MAX_ORDER; i++, area++) {
		head = &area->free_list;
		curr = head->next;

		if (curr != head) {
			page = get_container_of(curr, struct page_t, link); 

			list_del(curr);
			BITMAP_TOGGLE(area->bitmap, PAGE_INDEX(page->addr), i);

			/* if allocating from higher order, split in two to add
			 * one of them to current order. */
			while (i > order) {
				area--;
				i--;

				list_add(&page->link, &area->free_list);
				BITMAP_TOGGLE(area->bitmap, PAGE_INDEX(page->addr), i);

				page += 1 << i;
			}

			nr_free_pages -= 1 << order;

			SET_PAGE_ORDER(page, order);

			return page;
		}
	}

	return NULL;
}

extern char __mem_start, __mem_end, _ebss;

/* It could possibly overlap initial kernel stack. Initial stack region must
 * be reserved first, which is not implemented yet. */
static void free_area_init(struct zone_t *zone, unsigned long nr_pages,
		struct page_t *array)
{
	unsigned long size, offset;
	int i;

	//DEBUG(("&mem_map[nr_pages] 0x%08x - bitmap start addr", &array[nr_pages]));

	offset = 0;

	for (i = 0; i < BUDDY_MAX_ORDER; i++) {
		size = ALIGN_LONG(nr_pages) >> (4 + i);
		size = ALIGN_LONG(size);
		size = size? size : sizeof(long);
		zone->free_area[i].bitmap = (unsigned long *) 
				(((char *)&array[nr_pages]) + offset);
		memset(zone->free_area[i].bitmap, 0, size);
		//DEBUG(("[%02d] bitmap 0x%08x size %d bytes, order %d", i, zone->free_area[i].bitmap, size, 1UL << (PAGE_SHIFT+i)));
		LIST_LINK_INIT(&zone->free_area[i].free_list);

		offset += size;
	}

	offset = PAGE_ALIGN(((char *)&array[nr_pages]) + offset);
	//DEBUG(("the first free page 0x%08x, page[%d]", offset, PAGE_INDEX(offset)));

	struct free_area_t *area;
	unsigned long order, index, next;

	order = BUDDY_MAX_ORDER - 1;
	area  = &zone->free_area[order];
	size  = 1 << order;
	/* mark kernel .data and .bss sections as used */
	index = next = PAGE_INDEX(PAGE_ALIGN(&_ebss));

	while (1) {
		next = index + size;

		/* split in half if short for current order region */
		while ((long)(nr_pages - next) < 0) {
			if (order == 0)
				goto done_free_list;

			order--;
			area--;
			size = 1 << order;
			next = index + size;
			//DEBUG(("### order %02d", order));
		}

		list_add(&array[index].link, &area->free_list);
		//DEBUG(("area->bitmap %08x bitmap %08x", area->bitmap, BITMAP_POS(area->bitmap, index, order)));
		BITMAP_TOGGLE(area->bitmap, index, order);
		//DEBUG(("index %d, size %d(%02d), addr %08x[%08x], bitmap %08x", index, size, order, &array[index], array[index].addr, *BITMAP_POS(area->bitmap, index, order)));
		index = next;

		nr_free_pages += 1 << order;
	}

done_free_list:
	return;
}

static void zone_init(struct zone_t *zone, unsigned long nr_pages,
		struct page_t *array)
{
	free_area_init(zone, nr_pages, array);
	spinlock_init(zone->lock);
}

void mm_init()
{
	unsigned long start    = PAGE_ALIGN(&__mem_start);
	unsigned long end      = (unsigned long)&__mem_end;
	unsigned long nr_pages = PAGE_NR(end) - PAGE_NR(start) + 1;

	struct page_t *page = (struct page_t *)PAGE_ALIGN(&_ebss);

	//DEBUG(("Memory initiailization"));
	//DEBUG(("page size %d bytes", PAGE_SIZE));
	//DEBUG(("page struct size %d bytes", sizeof(struct page_t)));
	//DEBUG(("ram range 0x%08x - 0x%08x", start, end));
	//DEBUG(("total %d pages", nr_pages));

	nr_avail_pages = nr_pages;
	nr_free_pages  = 0;

	mem_map = page;

	while (start < end) {
		page->flags = 0;
		page->addr  = (void *)start;
		LIST_LINK_INIT(&page->link);

		start += PAGE_SIZE;
		page++;
	}

	//DEBUG(("0x%08x - mem_map first entry, page[%d]", mem_map, PAGE_INDEX(mem_map)));
	//DEBUG(("0x%08x - mem_map last entry, page[%d]", page - 1, PAGE_INDEX((page-1)->addr)));
	//DEBUG(("mem_map size : %d bytes", page - mem_map));

	zone_init(&dzone, nr_pages, mem_map);
}

static inline int log2(int a)
{
	int i;
	for (i = 0; a; i++) a >>= 1;
	return i;
}

void *kmalloc(unsigned long size)
{
	struct page_t *page;
	unsigned long irq_flag;

	spinlock_irqsave(dzone.lock, irq_flag);
	page = alloc_pages(&dzone, log2((size-1) >> PAGE_SHIFT));
	spinlock_irqrestore(dzone.lock, irq_flag);

	return page->addr;
}

void free(void *addr)
{
	struct page_t *page;
	unsigned long irq_flag;

	page = &mem_map[PAGE_INDEX(addr)];

	spinlock_irqsave(dzone.lock, irq_flag);
	free_pages(&dzone, page);
	spinlock_irqrestore(dzone.lock, irq_flag);
}
