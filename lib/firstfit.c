/*
 * 8-byte meta data per an allocated block is required.
 *
 * The latest freed object gets added to the head. And new object gets from
 * the head for cache efficiency.
 *
 * It splits an object in two if the requested block size is less than the
 * object size found. But if the remainder size is less than MIN_SIZE, it
 * doesn't split to reduce fragmentation, there is unlikely a request of that
 * much small size anyway.
 *
 * Figure 1. split P into C1 and C2:
 *
 *  ---------------------------------------
 * | meta | data space | meta | data space |
 *  ---------------------------------------
 * |      |            |      |<- C2size ->|
 * |      |            |<------- C2 ------>|
 * |      |<- C1size ->|
 * |<------- C1 ------>|
 * |      |<------------- P_size --------->|
 * |<----------------- P ----------------->|
 *
 * An allocated block looks like:
 *
 * Figure 2. free list
 *
 *  ------------------------------      ------------------------------
 * | size|A| *next | //// | *head | -> | size|A| *next | //// | *head |
 *  ------------------------------      ------------------------------
 *
 * Figure 3. alloced block
 *   _____________
 *  /             \
 * v               \
 *  ----------------------
 * | size|A| //// | *head |
 *  ----------------------
 *         ^
 *         return address
 *
 * 	A: the lowest bit of size
 * 	   reset if in freelist or set, meaning allocated
 */

#include <lib/firstfit.h>
#include <kernel/lock.h>
#include <error.h>

#define MIN_SIZE		(16 - 1) /* bytes */
#define HEAD_LOCKED		1

void *ff_alloc(struct ff_freelist_head *pool, size_t size)
{
	struct ff_freelist *p, *n;
	struct link *head, *curr, *next, *prev;
	unsigned int snap;

	if (!pool || !size)
		return NULL;

	size = ALIGN_WORD(size);

retry:
	n = NULL;
	next = NULL;
	head = prev = (struct link *)&pool->list_head;

	/* TODO: remove an embarrassing lock mechanism below but implement real
	 * lock-free.
	 *
	 * checking if locked first would be better before ldrex to avoid
	 * contention, since it does ldrex every time in the loop causing contention. */
	do {
		snap = __ldrex(&head->next);
		if (snap == HEAD_LOCKED)
			goto retry;
	} while (__strex(HEAD_LOCKED, &head->next));

	curr = (struct link *)snap;

	while (curr) {
		p = get_container_of(curr, typeof(*p), list);

		if (p->size < size) {
			prev = curr;
			curr = curr->next;
			continue;
		}

		next = curr->next;

		/* split if bigger than requested */
		if (p->size > size + MIN_SIZE + sizeof(*p)) {
			n = (struct ff_freelist *)
				((unsigned int)p + size + FF_METASIZE);
			n->size = p->size - size - FF_METASIZE;
			n->list.next = curr->next;
			n->head = n;
			FF_MARK_TAG(n);
			FF_MARK_FREE(n);

			next = &n->list;
		}

		break;
	}

	if (!curr) {
		head->next = (struct link *)snap;
		return NULL;
	}

	prev->next = next;
	if (&prev->next != &head->next)
		head->next = (struct link *)snap;

	if (n)
		p->size = p->size - n->size - FF_METASIZE;

	FF_MARK_TAG(p);
	FF_MARK_ALLOCATED(p);

	return (void *)((unsigned int)p + FF_DATA_OFFSET);
}

void ff_free(struct ff_freelist_head *pool, void *addr)
{
	struct ff_freelist *p, *prev, *next;
	struct link *head, tmp;
	unsigned int snap;

	if (!pool || !addr || addr < pool->base)
		return;

retry:
	head = (struct link *)&pool->list_head;

	do {
		snap = __ldrex(&head->next);
		if (snap == HEAD_LOCKED)
			goto retry;
	} while (__strex(HEAD_LOCKED, &head->next));

	tmp.next = (struct link *)snap;

	prev = next = NULL;
	p = (struct ff_freelist *)((unsigned int)addr - FF_DATA_OFFSET);
	FF_MARK_FREE(p);

	if ((void *)p > pool->base)
		prev = (struct ff_freelist *)
			*((char **)((unsigned int)p - WORD_SIZE));

	next = (struct ff_freelist *)((unsigned int)p + p->size + FF_METASIZE);

	if (prev && FF_IS_FREE(prev)) {
		link_del(&prev->list, &tmp);

		prev->size += p->size + FF_METASIZE;
		p = prev;
	}

	if ((void *)next < pool->limit && FF_IS_FREE(next)) {
		p->size += next->size + FF_METASIZE;

		link_del(&next->list, &tmp);
	}

	p->head = p;
	FF_MARK_TAG(p);
	p->list.next = tmp.next;

	head->next = &p->list;
}

size_t ff_freelist_init(struct ff_freelist_head *pool, void *start, void *end)
{
	struct ff_freelist *first;

	first = (struct ff_freelist *)ALIGN_WORD((unsigned int)start);
	first->size = (unsigned int)end - (unsigned int)first - FF_METASIZE;
	first->size = BASE_WORD(first->size);
	first->head = first;
	FF_MARK_TAG(first);
	link_init(&first->list);
	FF_MARK_FREE(first);

	pool->base = first;
	pool->limit = end;
	pool->list_head.next = &first->list;

	return first->size;
}

#ifdef CONFIG_DEBUG
#include <foundation.h>
#endif

size_t show_freelist(struct ff_freelist_head *pool)
{
	struct link *curr;
	struct ff_freelist *p;
	size_t nr_free;
	int i;

	nr_free = 0;
	i = 0;

	do {
		curr = (struct link *)&pool->list_head;
	} while (curr->next == (struct link *)HEAD_LOCKED);

	debug("  NR     ADDR     SIZE    NEXT");

	for (curr = curr->next; curr; curr = curr->next) {
		p = get_container_of(curr, typeof(*p), list);
		nr_free += p->size;

		debug("[%4d] %08x %8d %x", ++i, p, p->size, p->list.next);
		(void)i;
	}

	debug("Memory pool base: 0x%x, list head %x",
			pool->base, &pool->list_head);

	return nr_free;
}
