#include <kernel/page.h>
#include <kernel/lock.h>
#include <stdlib.h>
#include <error.h>

#define BITMAP_POS(bitmap, pfn, order) \
	/* (bitmap + ((pfn >> order) / (WORD_SIZE*8) / 2)) */ \
	(bitmap + (pfn >> (order + 5 + 1)))
#define BITMAP_OFFSET(pfn, order) \
	(1 << ((pfn >> (order+1)) & (WORD_SIZE*8-1)))
#define BITMAP_TOGGLE(bitmap, pfn, order) \
	(*BITMAP_POS(bitmap, pfn, order) ^= BITMAP_OFFSET(pfn, order))
#define BITMAP_MASK(bitmap, pfn, order) \
	(*BITMAP_POS(bitmap, pfn, order) & BITMAP_OFFSET(pfn, order))

void free_pages(struct buddy *pool, struct page *page)
{
	struct page *buddy;
	struct buddy_freelist *freelist;
	unsigned int order, index;

	/* debug(MSG_DEBUG, "free page 0x%08x", page->addr); */

	unsigned int irqflag;
	spin_lock_irqsave(&pool->lock, irqflag);

	order    = GET_PAGE_ORDER(page);
	freelist = &pool->free[order];
	index    = PAGE_INDEX(pool->mem_map, page->addr);

	pool->nr_free += 1U << order;

	while (order < BUDDY_MAX_ORDER) {
		BITMAP_TOGGLE(freelist->bitmap, index, order);
		/* If it was 0 before toggling, it means both of buddies
		 * were allocated. So nothing can be merged to upper order
		 * as one of them is still allocated. Otherwise, both of 
		 * them are free now. Therefore merge it to upper order. */
		if (BITMAP_MASK(freelist->bitmap, index, order))
			break;

		/* find buddy and detach it from current order to merge */
		buddy = &pool->mem_map[index ^ (1U << order)];
		list_del(&buddy->link);

		/* grab the first address of merging buddies */
		index &= ~(1U << order);

		order++;
		freelist++;
	}

	list_add(&pool->mem_map[index].link, &freelist->list);

	RESET_PAGE_FLAG(page, PAGE_BUDDY);

	spin_unlock_irqrestore(&pool->lock, irqflag);
}

struct page *alloc_pages(struct buddy *pool, unsigned int order)
{
	struct buddy_freelist *freelist;
	struct list *head, *curr;
	struct page *page;
	unsigned int i;

	unsigned int irqflag;
	spin_lock_irqsave(&pool->lock, irqflag);

	freelist = &pool->free[order];
	page = NULL;

	for (i = order; i < BUDDY_MAX_ORDER; i++, freelist++) {
		head = &freelist->list;
		curr = head->next;

		if (curr != head) {
			page = get_container_of(curr, struct page, link);

			list_del(curr);
			BITMAP_TOGGLE(freelist->bitmap,
					PAGE_INDEX(pool->mem_map, page->addr),
					i);

			/* if allocating from higher order, split in half to add
			 * one of them to current order. */
			while (i > order) {
				freelist--;
				i--;

				list_add(&page->link, &freelist->list);
				BITMAP_TOGGLE(freelist->bitmap,
						PAGE_INDEX(pool->mem_map, page->addr),
						i);

				page += 1 << i;
			}

			pool->nr_free -= 1 << order;

			SET_PAGE_ORDER(page, order);
			SET_PAGE_FLAG(page, PAGE_BUDDY);

			/* debug(MSG_DEBUG, "alloc page 0x%08x (order %d)",
					page->addr, order); */
			break;
		}
	}

	spin_unlock_irqrestore(&pool->lock, irqflag);

	return page;
}

#include <kernel/task.h>

static void buddy_freelist_init(struct buddy *pool,
		unsigned int nr_pages, struct page *array)
{
	size_t size;
	unsigned int offset;
	unsigned int i;

	debug(MSG_DEBUG, "&mem_map[nr_pages] 0x%08x - bitmap start addr",
			&array[nr_pages]);

	/* bitmap initialization */
	offset = 0;
	size   = ALIGN_WORD(nr_pages) >> 4; /* nr_pages / 8-bit / 2 */
	size   = ALIGN_WORD(size);
	for (i = 0; i < BUDDY_MAX_ORDER; i++) {
		if (size < WORD_SIZE)
			size = WORD_SIZE;

		pool->free[i].bitmap = (unsigned int *)
				(((char *)&array[nr_pages]) + offset);
		memset(pool->free[i].bitmap, 0, size);
		debug(MSG_DEBUG, "[%02d] bitmap 0x%08x size %d bytes, order %d"
				, i
				, pool->free[i].bitmap
				, size
				, 1UL << (PAGE_SHIFT+i));
		list_link_init(&pool->free[i].list);

		offset += size;
		size >>= 1;
	}

	/* current offset is the first free page */
	offset = ALIGN_PAGE(((char *)&array[nr_pages]) + offset);
	debug(MSG_DEBUG, "the first free page 0x%08x, page[%d]", offset,
			PAGE_INDEX(pool->mem_map, offset));

	/* buddy list initialization */
	struct buddy_freelist *freelist;
	unsigned int order, index, next;

	/* preserve initial kernel stack to be free later */
	nr_pages -= ALIGN_PAGE(STACK_SIZE) >> PAGE_SHIFT;
	order     = log2((ALIGN_PAGE(STACK_SIZE)-1) >> PAGE_SHIFT);
	SET_PAGE_ORDER(&array[nr_pages], order);
	/* and mark kernel .data and .bss sections as used.
	 * mem_map array and its bitmap region as well. */
	index = next = PAGE_INDEX(pool->mem_map, offset);

	order    = BUDDY_MAX_ORDER - 1;
	freelist = &pool->free[order];
	size     = 1 << order;

	while (1) {
		next = index + size;

		/* split in half if short for current order region */
		while ((int)(nr_pages - next) < 0) {
			if (order == 0)
				goto out;

			order--;
			freelist--;
			size = 1 << order;
			next = index + size;
			debug(MSG_DEBUG, "### order %02d", order);
		}

		list_add(&array[index].link, &freelist->list);
		debug(MSG_DEBUG, "freelist->bitmap %08x bitmap %08x"
				, freelist->bitmap
				, BITMAP_POS(freelist->bitmap, index, order));
		BITMAP_TOGGLE(freelist->bitmap, index, order);
		debug(MSG_DEBUG,
			"index %d, size %d(%02d), addr %08x[%08x], bitmap %08x",
				index, size, order, &array[index],
				array[index].addr,
				*BITMAP_POS(freelist->bitmap, index, order));
		index = next;

		pool->nr_free += 1 << order;
	}

out:
	return;
}

void buddy_init(struct buddy *pool, unsigned int nr_pages, struct page *array)
{
	pool->nr_pages = nr_pages;
	pool->nr_free  = 0;
	lock_init(&pool->lock);

	buddy_freelist_init(pool, nr_pages, array);
}

size_t show_buddy(void *zone)
{
	struct buddy *pool = (struct buddy *)zone;

#ifdef CONFIG_DEBUG_PRINTF
	struct page *page;
	struct list *p;
	size_t size;
	unsigned int i, j;

	printf("available pages %d\nfree pages %d\n",
			pool->nr_pages, pool->nr_free);

	for (i = 0; i < BUDDY_MAX_ORDER; i++) {
		printf("============= order %02d =============\n", i);
		p = pool->free[i].list.next;
		while (p != &pool->free[i].list) {
			page = get_container_of(p, struct page, link);
			printf("--> 0x%08x(0x%08x) ", page->addr, page);
			p = p->next;
		}

		printf("\n------ bitmap 0x%08x ------\n",
				pool->free[i].bitmap);
		size = ALIGN_WORD(pool->nr_pages) >> (4 + i);
		size = ALIGN_WORD(size);
		size = size? size : WORD_SIZE;
		size /= WORD_SIZE;
		for (j = size; j; j--) {
			printf("%08x ", *(pool->free[i].bitmap + j - 1));
			if (!((size-j+1) % 8))
				printf("\n");
		}
		printf("\n");
	}
#endif

	return pool->nr_free;
}
