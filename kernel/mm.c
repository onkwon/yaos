#include <kernel/page.h>
#include <stdlib.h>

extern char __mem_start, __mem_end, _ebss;

#ifdef CONFIG_PAGING
static void zone_init(struct zone_t *zone, unsigned long nr_pages,
		struct page_t *array)
{
	free_area_init(zone, nr_pages, array);
	spinlock_init(zone->lock);
}
#else
struct free_area_t {
	void *addr;
	unsigned long size;
	struct list_t list;
};

#include <lock.h>

static struct free_area_t *mem_map;
DEFINE_SPINLOCK(mem_lock);

static void *__kmalloc(unsigned long size)
{
	struct free_area_t *p, *new, **head;
	struct list_t *curr;
	unsigned long size_min;

	head = &mem_map;
	curr = &(*head)->list;
	size_min = ALIGN_WORD(size + sizeof(struct free_area_t));
	size = ALIGN_WORD(size);

	do {
		p = get_container_of(curr, struct free_area_t, list);

		if (size == p->size) {
			list_del(curr);
			if (curr == &(*head)->list)
				*head = get_container_of(curr->next,
						struct free_area_t, list);
			return p->addr;
		}

		if (size_min <= p->size) {
			new = p;

			p = (struct free_area_t *)((unsigned long)p + size_min);
			p->addr = (void *)((unsigned long)p +
					sizeof(struct free_area_t));
			p->size = new->size - size_min;
			list_add(&p->list, curr);
			list_del(curr);

			/* as allocating lower address first,
			 * head must be replaced with next address */
			if (curr == &(*head)->list)
				*head = p;

			new->size = size;

			return new->addr;
		}

		curr = curr->next;
	} while (curr != &(*head)->list);

	return NULL;
}

static void __free(void *addr)
{
	struct free_area_t *new, *p, **head;
	struct list_t *curr;

	head = &mem_map;
	curr = &(*head)->list;
	new  = (struct free_area_t *)
		((unsigned long)addr - sizeof(struct free_area_t));

	do {
		if (&new->list < curr) {
			list_add(&new->list, curr->prev);

			/* keep head to have the lowest address */
			if (curr == &(*head)->list)
				*head = new;

			/* merge with the next chunk if possible */
			for (curr = new->list.next; curr != &(*head)->list;
					curr = curr->next) {
__free_merge:
				p = get_container_of(curr,
						struct free_area_t, list);

				if (((unsigned long)new->addr + new->size) ==
						(unsigned long)p) {
					new->size += p->size +
						sizeof(struct free_area_t);
					list_del(curr);
				}
			}

			goto __free_done;
		}

		curr = curr->next;
	} while (curr != &(*head)->list);

	list_add(&new->list, (*head)->list.prev);

	/* merge with the privious chunk if possible */
	curr = &new->list;
	new  = get_container_of(new->list.prev, struct free_area_t, list);
	goto __free_merge;

__free_done:
	return;
}

void *kmalloc(unsigned long size)
{
	void *p;
	unsigned long irq_flag;

	spinlock_irqsave(mem_lock, irq_flag);
	p = __kmalloc(size);
	spinlock_irqrestore(mem_lock, irq_flag);

	return p;
}

void free(void *addr)
{
	unsigned long irq_flag;

	spinlock_irqsave(mem_lock, irq_flag);
	__free(addr);
	spinlock_irqrestore(mem_lock, irq_flag);
}

#ifdef CONFIG_DEBUG
#include <foundation.h>

void show_free_list(void *area)
{
	struct list_t *head, *curr;
	struct free_area_t *p;
	int i = 0;

	head = curr = &mem_map->list;

	do {
		p = get_container_of(curr, struct free_area_t, list);
		printf("[%4d] addr 0x%08x, size %d\n", ++i, p->addr, p->size);

		curr = curr->next;
	} while (curr != head);
}
#endif
#include <asm/init.h>
void __init free_bootmem()
{
	struct free_area_t *p;

	p = (void *)ALIGN_WORD((unsigned long)&__mem_end -
			(DEFAULT_STACK_SIZE + sizeof(struct free_area_t)));

	free(p->addr);
}
#endif /* CONFIG_PAGING */

void mm_init()
{
	unsigned long start = ALIGN_WORD(&__mem_start);
	unsigned long end   = (unsigned long)&__mem_end;
#ifdef CONFIG_PAGING
	unsigned long nr_pages = PAGE_NR(end) - PAGE_NR(start) + 1;

	extern struct page_t *mem_map;
	extern struct zone_t dzone;

	struct page_t *page = (struct page_t *)ALIGN_PAGE(&_ebss);

	mem_map = page;

	while (start < end) {
		page->flags = 0;
		page->addr  = (void *)start;
		LIST_LINK_INIT(&page->link);

		start += PAGE_SIZE;
		page++;
	}

	dzone.nr_pages = nr_pages;
	dzone.nr_free  = 0;
	zone_init(&dzone, nr_pages, mem_map);
#else
	struct free_area_t *p;

	/* preserve initial kernel stack to be free later */
	p = (struct free_area_t *)ALIGN_WORD(end -
			(DEFAULT_STACK_SIZE + sizeof(struct free_area_t)));
	p->addr = (void *)((unsigned long)p + sizeof(struct free_area_t));
	p->size = ALIGN_WORD(DEFAULT_STACK_SIZE);

	mem_map = (struct free_area_t *)&_ebss;
	/* mark kernel .data and .bss sections as used */
	mem_map->size = (unsigned long)p -
		((unsigned long)&_ebss + sizeof(struct free_area_t));
	mem_map->addr = (void *)((unsigned long)mem_map +
			sizeof(struct free_area_t));
	LIST_LINK_INIT(&mem_map->list);
#endif
}
