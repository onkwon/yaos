/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#include <lib/fifo.h>
#include <stddef.h>
#include <errno.h>
#ifndef TEST
#include <kernel/lock.h>
#include <bitops.h>
#endif

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
	unsigned int pos;
	char *buf;
	int val;

	if (!q || !q->buf)
		return -EINVAL;

	buf = q->buf;

	do {
		pos = __ldrex(&q->front);

		if (pos == *(volatile typeof(q->rear) *)&q->rear)
			return -ENOENT; /* empty */

		val = (typeof(val))buf[pos];
		pos = (pos + 1) % q->n;
	} while (__strex(pos, &q->front));

	return val;
}

int fifo_putb(struct fifo *q, int val)
{
	unsigned int pos;
	char *buf;

	if (!q || !q->buf)
		return -EINVAL;

	buf = q->buf;

	do {
		pos = __ldrex(&q->rear);
		if (((pos + 1) % q->n) ==
				*(volatile typeof(q->front) *)&q->front)
			return -ENOSPC; /* no more room */

		buf[pos] = (typeof(*buf))val;
		pos = (pos + 1) % q->n;
	} while (__strex(pos, &q->rear));

	return 0;
}

static inline int fifo_getw_core(struct fifo *q, unsigned int *p)
{
	unsigned int pos, *buf;

	if (!q || !q->buf || !p)
		return -EINVAL;

	buf = q->buf;

	do {
		pos = __ldrex(&q->front);

		if (pos == *(volatile typeof(q->rear) *)&q->rear)
			return -ENOENT; /* empty */

		*p = buf[pos];
		pos = (pos + 1) % q->n;
	} while (__strex(pos, &q->front));

	return 0;
}

int fifo_getw(struct fifo *q, void *p)
{
	return fifo_getw_core(q, (unsigned int *)p);
}

static inline int fifo_putw_core(struct fifo *q, unsigned int val)
{
	unsigned int pos, *buf;

	if (!q || !q->buf)
		return -EINVAL;

	buf = q->buf;

	do {
		pos = __ldrex(&q->rear);

		if (((pos + 1) % q->n) == *(volatile typeof(q->front) *)&q->front)
			return -ENOSPC; /* no more room */

		buf[pos] = (typeof(*buf))val;
		pos = (pos + 1) % q->n;
	} while (__strex(pos, &q->rear));

	return 0;
}

int fifo_putw(struct fifo *q, int val)
{
	return fifo_putw_core(q, (unsigned int)val);
}

int fifo_get(struct fifo *q, void *p)
{
	return fifo_getw_core(q, p);
}

int fifo_put(struct fifo *q, void *p)
{
	if (!p)
		return -EINVAL;

	return fifo_putw_core(q, (unsigned int)p);
}

#if 0
int fifo_peek(struct fifo *q, void *p)
{
}
#endif
