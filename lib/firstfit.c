#include "firstfit.h"
#include "list.h"
#include "types.h"
#include "syslog.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#define ALIGNMENT			sizeof(uintptr_t)
#define MIN_SIZE			ALIGN(16UL, ALIGNMENT) // bytes

struct freelist {
	size_t size;
	struct list node;
} __attribute__((packed, aligned(ALIGNMENT)));

// TODO: Implement coalesce()
static inline void coalesce(void *head)
{
	sysinfo("coalescing");
	(void)head;
}

int firstfit_init(void *head, void *addr, size_t size)
{
	struct freelist *new;
	struct list *list;

	if (!addr || !head || 0 >= (ssize_t)size)
		return -EINVAL;

	list = head;
	new = (struct freelist *)ALIGN((uintptr_t)addr, ALIGNMENT);
	size = size - ((uintptr_t)new - (uintptr_t)addr);

	if (size < (MIN_SIZE + sizeof(*new)))
		return -ENOSPC;

	new->size = size - sizeof(*new)
		+ sizeof(new->node); // only size info left when allocated

	list_add(&new->node, list);

	debug("%zu bytes from %p added", size, new);

	return 0;
}

void *firstfit_alloc(void *head, size_t size)
{
	struct list *list;
	struct freelist *p, *allocated;
	bool coalesced;

	allocated = NULL;
	coalesced = false;

	if (!head)
		goto out;

	size = ALIGN(max(size, MIN_SIZE), ALIGNMENT);

retry:
	for (list = ((struct list *)head)->next; list; list = list->next) {
		p = container_of(list, struct freelist, node);
		if (p->size >= size)
			break;
	}

	if (!list) {
		if (coalesced) {
			alert("out of memory");
			goto out;
		}

		coalesce(head);
		coalesced = true;
		goto retry;
	}

	if (p->size > (size + MIN_SIZE + sizeof(p->size))) {
		p->size -= size + sizeof(p->size);
		allocated = (struct freelist *)
			((uintptr_t)p + p->size + sizeof(p->size));
		allocated->size = size;
	} else {
		allocated = p;
		list_del(&p->node, head);
	}

	allocated = (void *)((uintptr_t)allocated + sizeof(p->size));
out:
	return allocated;
}

/** recently-used order */
static inline void firstfit_free_mru(void *head, void *addr)
{
	struct freelist *p;

	p = (struct freelist *)((uintptr_t)addr - sizeof(p->size));
	list_add(&p->node, head);
}

/** address order */
static inline void firstfit_free_addr(void *head, void *addr)
{
	struct freelist *new, *p;
	struct list **list;

	new = (struct freelist *)((uintptr_t)addr - sizeof(new->size));
	list = (struct list **)&head;

	while (*list && (*list)->next) {
		p = container_of((*list)->next, struct freelist, node);

		if ((uintptr_t)new < (uintptr_t)p) {
			uintptr_t next =
				(uintptr_t)new + new->size + sizeof(new->size);

			if ((uintptr_t)p == next) { // merge next
				new->size += p->size + sizeof(p->size);
				list_del(&p->node, *list);
			}
			break;
		}

		list = (struct list **)&(*list)->next;
	}

	if (*list != head) {
		p = container_of(*list, struct freelist, node);
		uintptr_t next = (uintptr_t)p + p->size + sizeof(p->size);
		if (next == (uintptr_t)new) { // merge previous
			p->size += new->size + sizeof(new->size);
			return;
		}
	}

	list_add(&new->node, *list);
}

void firstfit_free(void *head, void *addr)
{
	if (!head || !addr)
		return;

#if defined(CONFIG_MALLOC_FF_MRU)
	firstfit_free_mru(head, addr);
#else
	firstfit_free_addr(head, addr);
#endif
}

void firstfit_coalesce(void *head)
{
	if (!head)
		return;

	coalesce(head);
}

size_t firstfit_left(void *head, size_t *largest, int *nr)
{
	struct list *list;
	struct freelist *p;
	size_t sum, bmax;
	int n;

	if (!largest)
		largest = &bmax;
	if (!nr)
		nr = &n;

	sum = *largest = 0;
	*nr = 0;

	if (!head)
		goto out;

	for (list = ((struct list *)head)->next; list; list = list->next) {
		p = container_of(list, struct freelist, node);
		sum += p->size;
		*largest = max(*largest, p->size);
		(*nr)++;
	}

out:
	return sum;
}

int firstfit_fragmentation(void *head)
{
	size_t total, largest;

	total = firstfit_left(head, &largest, NULL);

	return (int)(100 - largest * 100 / total);
}
