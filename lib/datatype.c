#include "types.h"

/* double linked list */

void LIST_LINK_INIT(struct list_t *list)
{
	list->next = list;
	list->prev = list;
}

void list_add(struct list_t *new, struct list_t *ref)
{
	new->prev = ref;
	new->next = ref->next;
	ref->next->prev = new;
	ref->next = new;
}

void list_del(struct list_t *item)
{
	item->prev->next = item->next;
	item->next->prev = item->prev;
}

/* fifo */

void fifo_init(struct fifo_t *f, char *buf, int size)
{
	f->head = f->tail = 0;
	f->size = size;
	f->buf  = buf;
}

int fifo_get(struct fifo_t *f, void *buf, int size)
{
	char *p = buf;
	int   i;

	for (i = 0; i < size; i++) {
		if (f->tail == f->head) {
			break; /* return number of bytes read */
		} else {
			*p++ = f->buf[f->tail];
			f->tail++;
			if (f->tail >= f->size) /* check for wrap-around */
				f->tail = 0;
		}
	}

	return i;
}

int fifo_put(struct fifo_t *f, void *buf, int size)
{
	const char *p = buf;
	int i;

	for (i = 0; i < size; i++) {
		if ( (f->head + 1 == f->tail) ||
			( (f->head + 1 == f->size) && (f->tail == 0) )) {
			break; /* no more room */
		} else {
			f->buf[f->head] = *p++;
			f->head++;
			if (f->head >= f->size)
				f->head = 0;
		}
	}

	return i;
}
