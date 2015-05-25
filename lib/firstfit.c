#include <firstfit.h>

void *ff_alloc(void *freelist, size_t size)
{
	struct ff_freelist_t *p, *new, **head;
	struct list_t *curr;
	size_t size_min;

	head = (struct ff_freelist_t **)freelist;
	curr = &(*head)->list;
	size_min = ALIGN_WORD(size + sizeof(struct ff_freelist_t));
	size = ALIGN_WORD(size);

	do {
		p = get_container_of(curr, struct ff_freelist_t, list);

		if (size == p->size) {
			list_del(curr);
			if (curr == &(*head)->list)
				*head = get_container_of(curr->next,
						struct ff_freelist_t, list);
			return p->addr;
		}

		if (size_min <= p->size) {
			new = p;

			p = (struct ff_freelist_t *)((unsigned int)p + size_min);
			p->addr = (void *)((unsigned int)p +
					sizeof(struct ff_freelist_t));
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

void ff_free(void *freelist, void *addr)
{
	struct ff_freelist_t *new, *p, **head;
	struct list_t *curr;

	head = (struct ff_freelist_t **)freelist;
	curr = &(*head)->list;
	new  = (struct ff_freelist_t *)
		((unsigned int)addr - sizeof(struct ff_freelist_t));

	do {
		if (&new->list < curr) {
			list_add(&new->list, curr->prev);

			/* keep head to have the lowest address */
			if (curr == &(*head)->list)
				*head = new;

			/* merge with the next chunk if possible */
			for (curr = new->list.next; curr != &(*head)->list;
					curr = curr->next) {
ff_merge:
				p = get_container_of(curr,
						struct ff_freelist_t, list);

				if (((unsigned int)new->addr + new->size) ==
						(unsigned int)p) {
					new->size += p->size +
						sizeof(struct ff_freelist_t);
					list_del(curr);
				} else {
					break;
				}
			}

			goto ff_done;
		}

		curr = curr->next;
	} while (curr != &(*head)->list);

	list_add(&new->list, (*head)->list.prev);

	/* merge with the privious chunk if possible */
	curr = &new->list;
	new  = get_container_of(new->list.prev, struct ff_freelist_t, list);
	goto ff_merge;

ff_done:
	return;
}

struct ff_freelist_t *ff_freelist_init(void *start, void *end)
{
	struct ff_freelist_t *head;

	head = (struct ff_freelist_t *)ALIGN_WORD(start);
	head->size = (unsigned int)end -
		((unsigned int)head + sizeof(struct ff_freelist_t));
	head->addr = (void *)((unsigned int)head + sizeof(struct ff_freelist_t));
	list_link_init(&head->list);

	return head;
}

#ifdef CONFIG_DEBUG
#include <foundation.h>

void show_freelist(void *pool)
{
	struct list_t *head, *curr;
	struct ff_freelist_t *p;
	int i = 0;

	head = curr = &(((struct ff_freelist_t *)pool)->list);

	do {
		p = get_container_of(curr, struct ff_freelist_t, list);
		printk("[%4d] addr 0x%08x, size %d\n", ++i, p->addr, p->size);

		curr = curr->next;
	} while (curr != head);
}
#endif
