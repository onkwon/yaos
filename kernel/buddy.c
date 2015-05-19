#include <kernel/page.h>
#include <stdlib.h>

struct buddypool_t buddypool;

extern struct page_t *mem_map;

#define BITMAP_POS(bitmap, pfn, order) \
	/* (bitmap + ((pfn >> order) / (sizeof(long)*8) / 2)) */ \
	(bitmap + (pfn >> (order + 5 + 1)))
#define BITMAP_OFFSET(pfn, order) \
	(1 << ((pfn >> (order+1)) & (sizeof(long)*8-1)))
#define BITMAP_TOGGLE(bitmap, pfn, order) \
	(*BITMAP_POS(bitmap, pfn, order) ^= BITMAP_OFFSET(pfn, order))
#define BITMAP_MASK(bitmap, pfn, order) \
	(*BITMAP_POS(bitmap, pfn, order) & BITMAP_OFFSET(pfn, order))

void free_pages(struct buddypool_t *pool, struct page_t *page)
{
	struct page_t *buddy;
	struct buddy_freelist_t *freelist;
	unsigned long order, index;

	order    = GET_PAGE_ORDER(page);
	freelist = &pool->free[order];
	index    = PAGE_INDEX(page->addr);

	pool->nr_free += 1 << order;

	while (order < BUDDY_MAX_ORDER) {
		BITMAP_TOGGLE(freelist->bitmap, index, order);
		/* If it was 0 before toggling, it means both of buddies
		 * were allocated. So nothing can be merged to upper order
		 * as one of them is still allocated. Otherwise, both of 
		 * them are free now. Therefore merge it to upper order. */
		if (BITMAP_MASK(freelist->bitmap, index, order))
			break;

		/* find buddy and detach it from current order to merge */
		buddy = &mem_map[index ^ (1<<order)];
		list_del(&buddy->link);

		order++;
		freelist++;

		/* grab the first address of merging buddies */
		index &= ~0 << order;
	}

	list_add(&mem_map[index].link, &freelist->list);

	RESET_PAGE_FLAG(page, PAGE_BUDDY);
}

struct page_t *alloc_pages(struct buddypool_t *pool, unsigned long order)
{
	struct buddy_freelist_t *freelist = &pool->free[order];
	struct list_t *head, *curr;
	struct page_t *page;
	unsigned long i;

	for (i = order; i < BUDDY_MAX_ORDER; i++, freelist++) {
		head = &freelist->list;
		curr = head->next;

		if (curr != head) {
			page = get_container_of(curr, struct page_t, link); 

			list_del(curr);
			BITMAP_TOGGLE(freelist->bitmap,
					PAGE_INDEX(page->addr), i);

			/* if allocating from higher order, split in two to add
			 * one of them to current order. */
			while (i > order) {
				freelist--;
				i--;

				list_add(&page->link, &freelist->list);
				BITMAP_TOGGLE(freelist->bitmap,
						PAGE_INDEX(page->addr), i);

				page += 1 << i;
			}

			pool->nr_free -= 1 << order;

			SET_PAGE_ORDER(page, order);
			SET_PAGE_FLAG(page, PAGE_BUDDY);

			return page;
		}
	}

	return NULL;
}

static void buddy_freelist_init(struct buddypool_t *pool,
		unsigned long nr_pages, struct page_t *array)
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

		pool->free[i].bitmap = (unsigned long *) 
				(((char *)&array[nr_pages]) + offset);
		memset(pool->free[i].bitmap, 0, size);
		LIST_LINK_INIT(&pool->free[i].list);

		offset += size;
		size >>= 1;
	}

	/* current offset is the first free page */
	offset = ALIGN_PAGE(((char *)&array[nr_pages]) + offset);

	/* buddy list initialization */
	struct buddy_freelist_t *freelist;
	unsigned long order, index, next;

	/* preserve initial kernel stack to be free later */
	nr_pages -= ALIGN_PAGE(DEFAULT_STACK_SIZE) >> PAGE_SHIFT;
	order     = log2((ALIGN_PAGE(DEFAULT_STACK_SIZE)-1) >> PAGE_SHIFT);
	SET_PAGE_ORDER(&array[nr_pages], order);
	/* and mark kernel .data and .bss sections as used.
	 * mem_map array and its bitmap region as well. */
	index = next = PAGE_INDEX(offset);

	order    = BUDDY_MAX_ORDER - 1;
	freelist = &pool->free[order];
	size     = 1 << order;

	while (1) {
		next = index + size;

		/* split in half if short for current order region */
		while ((long)(nr_pages - next) < 0) {
			if (order == 0)
				goto freelist_done;

			order--;
			freelist--;
			size = 1 << order;
			next = index + size;
		}

		list_add(&array[index].link, &freelist->list);
		BITMAP_TOGGLE(freelist->bitmap, index, order);
		index = next;

		pool->nr_free += 1 << order;
	}

freelist_done:
	return;
}

void buddy_init(struct buddypool_t *pool, unsigned long nr_pages,
		struct page_t *array)
{
	buddy_freelist_init(pool, nr_pages, array);
	spinlock_init(pool->lock);
}

#ifdef CONFIG_DEBUG
#include <foundation.h>

void show_buddy(void *zone)
{
	struct page_t *page;
	struct list_t *p;
	int i, j, size;

	struct buddypool_t *pool = (struct buddypool_t *)zone;

	if (!pool) pool = &buddypool;

	printk("available pages %d\nfree pages %d\n",
			pool->nr_pages, pool->nr_free);

	for (i = 0; i < BUDDY_MAX_ORDER; i++) {
		printk("============= order %02d =============\n", i);
		p = pool->free[i].list.next;
		while (p != &pool->free[i].list) {
			page = get_container_of(p, struct page_t, link); 
			printk("--> 0x%08x(0x%08x) ", page->addr, page);
			p = p->next;
		}

		printk("\n------ bitmap 0x%08x ------\n",
				pool->free[i].bitmap);
		size = ALIGN_WORD(pool->nr_pages) >> (4 + i);
		size = ALIGN_WORD(size);
		size = size? size : sizeof(long);
		size /= sizeof(long);
		for (j = size; j; j--) {
			printk("%08x ", *(pool->free[i].bitmap + j - 1));
			if (!((size-j+1) % 8))
				printk("\n");
		}
		printk("\n");
	}
}
#endif
