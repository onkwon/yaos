#include "fifo.h"
#include "arch/atomic.h"
#include <errno.h>

void fifo_init(struct fifo *q, void *queue, size_t n)
{
	q->n = n;
	q->buf = queue;
	q->front = q->rear = 0;
}

void fifo_flush(struct fifo *q)
{
	q->front = q->rear = 0;
}

bool fifo_empty(struct fifo *q)
{
	return q->front == q->rear;
}

int fifo_getb(struct fifo *q)
{
	uintptr_t pos;
	uint8_t *buf;
	int val;

	if (!q || !q->buf)
		return -EINVAL;

	buf = q->buf;

	do {
		pos = atomic_ll(&q->front);

		if (pos == *(volatile typeof(q->rear) *)&q->rear)
			return -ENOENT; /* empty */

		val = (typeof(val))buf[pos];
		pos = (pos + 1) % q->n;
	} while (atomic_sc(&q->front, pos));

	return val;
}

int fifo_putb(struct fifo *q, int val)
{
	uintptr_t pos;
	uint8_t *buf;

	if (!q || !q->buf)
		return -EINVAL;

	buf = q->buf;

	do {
		pos = atomic_ll(&q->rear);
		if (((pos + 1) % q->n) ==
				*(volatile typeof(q->front) *)&q->front)
			return -ENOSPC; /* no more room */

		buf[pos] = (typeof(*buf))val;
		pos = (pos + 1) % q->n;
	} while (atomic_sc(&q->rear, pos));

	return 0;
}

static inline int fifo_getw_core(struct fifo *q, uintptr_t *p)
{
	uintptr_t pos, *buf;

	if (!q || !q->buf || !p)
		return -EINVAL;

	buf = q->buf;

	do {
		pos = atomic_ll(&q->front);

		if (pos == *(volatile typeof(q->rear) *)&q->rear)
			return -ENOENT; /* empty */

		*p = buf[pos];
		pos = (pos + 1) % q->n;
	} while (atomic_sc(&q->front, pos));

	return 0;
}

int fifo_getw(struct fifo *q, void *p)
{
	return fifo_getw_core(q, (uintptr_t *)p);
}

static inline int fifo_putw_core(struct fifo *q, uintptr_t val)
{
	uintptr_t pos, *buf;

	if (!q || !q->buf)
		return -EINVAL;

	buf = q->buf;

	do {
		pos = atomic_ll(&q->rear);

		if (((pos + 1) % q->n) == *(volatile typeof(q->front) *)&q->front)
			return -ENOSPC; /* no more room */

		buf[pos] = (typeof(*buf))val;
		pos = (pos + 1) % q->n;
	} while (atomic_sc(&q->rear, pos));

	return 0;
}

int fifo_putw(struct fifo *q, uintptr_t val)
{
	return fifo_putw_core(q, val);
}

int fifo_get(struct fifo *q, void *p)
{
	return fifo_getw_core(q, p);
}

int fifo_put(struct fifo *q, uintptr_t p)
{
	return fifo_putw_core(q, p);
}

#if 0
int fifo_peek(struct fifo *q, void *p)
{
}
#endif
