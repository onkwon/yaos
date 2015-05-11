#include <kernel/page.h>
#include <stdlib.h>

struct page_t *mem_map;
struct zone_t dzone;

#define BITMAP_POS(bitmap, pfn, order) \
	/* (bitmap + ((pfn >> order) / (sizeof(long)*8) / 2)) */ \
	(bitmap + (pfn >> (order + 5 + 1)))
#define BITMAP_OFFSET(pfn, order) \
	(1 << ((pfn >> (order+1)) & (sizeof(long)*8-1)))
#define BITMAP_TOGGLE(bitmap, pfn, order) \
	(*BITMAP_POS(bitmap, pfn, order) ^= BITMAP_OFFSET(pfn, order))
#define BITMAP_MASK(bitmap, pfn, order) \
	(*BITMAP_POS(bitmap, pfn, order) & BITMAP_OFFSET(pfn, order))

void free_pages(struct zone_t *zone, struct page_t *page)
{
	struct page_t *node = mem_map;
	struct page_t *buddy;
	struct free_area_t *area;
	unsigned long order, index;

	order = GET_PAGE_ORDER(page);
	area  = &zone->free_area[order];
	index = PAGE_INDEX(page->addr);

	zone->nr_free += 1 << order;

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

	RESET_PAGE_FLAG(page, PAGE_BUDDY);
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

			zone->nr_free -= 1 << order;

			SET_PAGE_ORDER(page, order);
			SET_PAGE_FLAG(page, PAGE_BUDDY);

			return page;
		}
	}

	return NULL;
}

static inline int log2(int a)
{
	int i;
	for (i = 0; a; i++) a >>= 1;
	return i;
}

void free_area_init(struct zone_t *zone, unsigned long nr_pages,
		struct page_t *array)
{
	unsigned long size, offset;
	int i;

	/* bitmap initialization */
	offset = 0;
	size   = ALIGN_WORD(nr_pages) >> 4; /* nr_pages / 8-bit / 2 */
	size   = ALIGN_WORD(size);
	for (i = 0; i < BUDDY_MAX_ORDER; i++) {
		if (size < sizeof(long))
			size = sizeof(long);

		zone->free_area[i].bitmap = (unsigned long *) 
				(((char *)&array[nr_pages]) + offset);
		memset(zone->free_area[i].bitmap, 0, size);
		LIST_LINK_INIT(&zone->free_area[i].free_list);

		offset += size;
		size >>= 1;
	}

	/* current offset is the first free page */
	offset = ALIGN_PAGE(((char *)&array[nr_pages]) + offset);

	/* buddy list initialization */
	struct free_area_t *area;
	unsigned long order, index, next;

	/* preserve initial kernel stack to be free later */
	nr_pages -= ALIGN_PAGE(DEFAULT_STACK_SIZE) >> PAGE_SHIFT;
	order     = log2((ALIGN_PAGE(DEFAULT_STACK_SIZE)-1) >> PAGE_SHIFT);
	SET_PAGE_ORDER(&array[nr_pages], order);
	/* and mark kernel .data and .bss sections as used.
	 * mem_map array and its bitmap region as well. */
	index = next = PAGE_INDEX(offset);

	order = BUDDY_MAX_ORDER - 1;
	area  = &zone->free_area[order];
	size  = 1 << order;

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
		}

		list_add(&array[index].link, &area->free_list);
		BITMAP_TOGGLE(area->bitmap, index, order);
		index = next;

		zone->nr_free += 1 << order;
	}

done_free_list:
	return;
}

void *kmalloc(unsigned long size)
{
	struct page_t *page;
	unsigned long irq_flag;

	spinlock_irqsave(dzone.lock, irq_flag);
	page = alloc_pages(&dzone, log2((ALIGN_PAGE(size)-1) >> PAGE_SHIFT));
	spinlock_irqrestore(dzone.lock, irq_flag);

	if (page)
		return page->addr;

	return NULL;
}

void free(void *addr)
{
	struct page_t *page;
	struct zone_t *zone = &dzone;
	unsigned long index;
	unsigned long irq_flag;

	if (!addr) return;

	index = PAGE_INDEX(addr);
	if (index >= zone->nr_pages) /* out of range */
		return;

	page = &mem_map[PAGE_INDEX(addr)];

	/* if not allocated by buddy allocator there is no way to free.
	 * Wrong address or where must not be free. */
	if (!(GET_PAGE_FLAG(page) & PAGE_BUDDY))
		return;

	spinlock_irqsave(dzone.lock, irq_flag);
	free_pages(zone, page);
	spinlock_irqrestore(dzone.lock, irq_flag);
}

#ifdef CONFIG_DEBUG
#include <foundation.h>

void show_free_list(void *area)
{
	struct page_t *page;
	struct list_t *p;
	int i, j, size;

	struct zone_t *zone = (struct zone_t *)area;

	if (!zone) zone = &dzone;

	printf("available pages %d\nfree pages %d\n", zone->nr_pages, zone->nr_free);

	for (i = 0; i < BUDDY_MAX_ORDER; i++) {
		printf("============= order %02d =============\n", i);
		p = zone->free_area[i].free_list.next;
		while (p != &zone->free_area[i].free_list) {
			page = get_container_of(p, struct page_t, link); 
			printf("--> 0x%08x(0x%08x) ", page->addr, page);
			p = p->next;
		}

		printf("\n------ bitmap 0x%08x ------\n", zone->free_area[i].bitmap);
		size = ALIGN_WORD(zone->nr_pages) >> (4 + i);
		size = ALIGN_WORD(size);
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
#endif

#include <asm/init.h>
void __init free_bootmem()
{
	void *addr;
	unsigned index;

	index = dzone.nr_pages - (ALIGN_PAGE(DEFAULT_STACK_SIZE) >> PAGE_SHIFT);
	addr  = mem_map[index].addr;

	free(addr);
}
