#include <kernel/page.h>
#include <kernel/lock.h>
#include <stdlib.h>
#include <error.h>
#include <bitops.h>

static inline bool is_buddy_free(struct page *page, struct page *buddy,
		unsigned int order)
{
	if ((GET_PAGE_FLAG(buddy) & PAGE_BUDDY) &&
			(order == GET_PAGE_ORDER(buddy)))
		return true;

	return false;
}

void free_pages(struct buddy *node, struct page *page)
{
	struct page *buddy, *mem_map;
	struct buddy_freelist *freelist;
	unsigned int order, idx, n;
	unsigned int irqflag;

	mem_map = node->mem_map;
	idx = page - mem_map;
	order = GET_PAGE_ORDER(page);
	freelist = &node->freelist[order];
	n = 1U << order;

	spin_lock_irqsave(&node->lock, irqflag);

	node->nr_free += n;

	while (order < (BUDDY_MAX_ORDER - 1)) {
		n = 1U << order;
		buddy = &mem_map[idx ^ n];

		if (!is_buddy_free(page, buddy, order))
			break;

		list_del(&buddy->link);
		freelist->nr_pageblocks--;
		RESET_PAGE_FLAG(buddy, PAGE_BUDDY);

		idx &= ~n; /* grab the head of merging buddies */
		order++;
		freelist++;
	}

	page = &mem_map[idx];
	list_add(&page->link, &freelist->list);
	freelist->nr_pageblocks++;
	SET_PAGE_FLAG(page, PAGE_BUDDY);
	SET_PAGE_ORDER(page, order);

	spin_unlock_irqrestore(&node->lock, irqflag);
}

struct page *alloc_pages(struct buddy *node, unsigned int order)
{
	struct page *page;
	struct buddy_freelist *freelist;
	struct list *head, *curr;
	unsigned int i, n;

	freelist = &node->freelist[order];
	page = NULL;

	unsigned int irqflag;
	spin_lock_irqsave(&node->lock, irqflag);

	for (i = order; i < BUDDY_MAX_ORDER; i++, freelist++) {
		head = &freelist->list;
		curr = head->next;

		if (curr == head)
			continue;

		page = get_container_of(curr, struct page, link);
		RESET_PAGE_FLAG(page, PAGE_BUDDY);
		list_del(curr);
		freelist->nr_pageblocks--;

		/* split in half to add one of them to the current order list
		 * if allocating from a higher order */
		while (i > order) {
			freelist--;
			i--;
			n = 1U << i;

			SET_PAGE_FLAG(&page[n], PAGE_BUDDY);
			SET_PAGE_ORDER(&page[n], i);
			list_add(&page[n].link, &freelist->list);
			freelist->nr_pageblocks++;
		}

		node->nr_free -= 1 << order;

		RESET_PAGE_FLAG(page, PAGE_BUDDY);
		SET_PAGE_ORDER(page, i);
		break;
	}

	spin_unlock_irqrestore(&node->lock, irqflag);

	return page;
}

#include <kernel/task.h>

#define mylog2(x)			(ffs(x) - 1) /* x > 0 */

static void buddy_freelist_init(struct buddy *node, size_t nr_pages,
		struct page *mem_map)
{
	extern unsigned int _ram_start;
	unsigned int order, idx, preserved, nr_free, n;

	/* Preserve the initial kernel stack to be free later, which is located
	 * at the end of memory */
	preserved = nr_pages - PAGE_NR(ALIGN_PAGE(STACK_SIZE));
	order = mylog2(PAGE_NR(ALIGN_PAGE(STACK_SIZE)));
	SET_PAGE_ORDER(&mem_map[preserved], order);

	/* And mark kernel .data and .bss sections as used.
	 * The region of mem_map array as well. */
	idx = PAGE_NR(ALIGN_PAGE(&mem_map[nr_pages]) -
			(unsigned int)&_ram_start);
	nr_free = preserved - idx;
	debug(MSG_DEBUG, "The first free page(idx:%d) - %x",
			idx, &mem_map[nr_pages]);
	debug(MSG_DEBUG, "The number of free pages %d", nr_free);

	struct page *page;

	while (nr_free) {
		order = min(mylog2(idx), BUDDY_MAX_ORDER - 1);
		while ((int)(nr_free - (1U << order)) < 0)
			order--;

		page = &mem_map[idx];
		SET_PAGE_ORDER(page, order);
		SET_PAGE_FLAG(page, PAGE_BUDDY);
		list_add(&page->link, &node->freelist[order].list);

		node->freelist[order].nr_pageblocks++;
		debug(MSG_DEBUG, "%02d %x added to %x", order, &page->link,
				&node->freelist[order].list);

		n = 1U << order;
		debug(MSG_DEBUG, "%04d: idx %d buddy %d head %d",
				n, idx, idx ^ n, idx & ~n);
		idx += n;
		nr_free -= n;
		node->nr_free += n; /* total pages being managed by buddy */
		debug(MSG_DEBUG, "order %d, nr_free %d, next %d\n",
				order, nr_free, idx);
	}
}

void buddy_init(struct buddy *node, size_t nr_pages, struct page *array)
{
	int i;

	for (i = 0; i < BUDDY_MAX_ORDER; i++) {
		node->freelist[i].nr_pageblocks = 0;
		list_link_init(&node->freelist[i].list);
	}

	node->nr_pages = nr_pages;
	node->nr_free = 0;
	lock_init(&node->lock);

	buddy_freelist_init(node, nr_pages, array);

#ifdef CONFIG_DEBUG
	show_buddy_all(node);
#endif
}

size_t show_buddy_all(struct buddy *node)
{
	struct list *p;
	int i;

	printk("The number of pages managed by buddy %d/%d (%d bytes)\n",
			node->nr_free, node->nr_pages,
			node->nr_free * PAGESIZE);

	unsigned int irqflag;
	spin_lock_irqsave(&node->lock, irqflag);

	for (i = 0; i < BUDDY_MAX_ORDER; i++) {
		printk("order %02d, %d page blocks\n",
				i, node->freelist[i].nr_pageblocks);
		p = node->freelist[i].list.next;
		while (p != &node->freelist[i].list) {
			printk(" -> %x", p);
			p = p->next;
		}
		printk("\n");
	}

	spin_unlock_irqrestore(&node->lock, irqflag);

	return node->nr_free;
}
