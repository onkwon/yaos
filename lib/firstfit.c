#include <firstfit.h>

void *ff_alloc(void *freelist, size_t size)
{
	struct ff_freelist *p, *new, **head;
	struct links *curr;
	size_t size_min;

	head = (struct ff_freelist **)freelist;
	curr = &(*head)->list;
	size_min = ALIGN_WORD(size + sizeof(struct ff_freelist));
	size = ALIGN_WORD(size);

	do {
		p = get_container_of(curr, struct ff_freelist, list);

		if (size == p->size) {
			links_del(curr);
			if (curr == &(*head)->list)
				*head = get_container_of(curr->next,
						struct ff_freelist, list);
			return p->addr;
		}

		if (size_min <= p->size) {
			new = p;

			p = (struct ff_freelist *)((unsigned int)p + size_min);
			p->addr = (void *)((unsigned int)p +
					sizeof(struct ff_freelist));
			p->size = new->size - size_min;
			links_add(&p->list, curr);
			links_del(curr);

			/* as allocating lower address first,
			 * head must be replaced with the next address */
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
	struct ff_freelist *new, *p, **head;
	struct links *curr;

	head = (struct ff_freelist **)freelist;
	curr = &(*head)->list;
	new  = (struct ff_freelist *)
		((unsigned int)addr - sizeof(struct ff_freelist));

	do {
		if (&new->list == curr) /* already in the list */
			goto out;
		else if (&new->list < curr) {
			links_add(&new->list, curr->prev);

			/* keep head to have the lowest address */
			if (curr == &(*head)->list)
				*head = new;

			/* merge with the next chunk if possible */
			for (curr = new->list.next; curr != &(*head)->list;
					curr = curr->next) {
merge:
				p = get_container_of(curr,
						struct ff_freelist, list);

				if (((unsigned int)new->addr + new->size) ==
						(unsigned int)p) {
					new->size += p->size +
						sizeof(struct ff_freelist);
					links_del(curr);
				} else {
					break;
				}
			}

			goto out;
		}

		curr = curr->next;
	} while (curr != &(*head)->list);

	links_add(&new->list, (*head)->list.prev);

	/* merge with the privious chunk if possible */
	curr = &new->list;
	new  = get_container_of(new->list.prev, struct ff_freelist, list);
	goto merge;

out:
	return;
}

struct ff_freelist *ff_freelist_init(void *start, void *end)
{
	struct ff_freelist *head;

	head = (struct ff_freelist *)ALIGN_WORD(start);
	head->size = (unsigned int)end -
		((unsigned int)head + sizeof(struct ff_freelist));
	head->addr = (void *)((unsigned int)head + sizeof(struct ff_freelist));
	links_init(&head->list);

	return head;
}

#ifdef CONFIG_DEBUG
#include <foundation.h>
#endif

size_t show_freelist(void *pool)
{
	struct links *head, *curr;
	struct ff_freelist *p;
	unsigned int size_available;

	head = curr = &(*(struct ff_freelist **)pool)->list;
	size_available = 0;

#ifdef CONFIG_DEBUG
	unsigned int i = 0;
#endif

	do {
		p = get_container_of(curr, struct ff_freelist, list);
		size_available += p->size;
#ifdef CONFIG_DEBUG
		printf("[%4d] addr 0x%08x, size %d\n", ++i, p->addr, p->size);
#endif

		curr = curr->next;
	} while (curr != head);

	return size_available;
}
