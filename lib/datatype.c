#include <types.h>
#include <error.h>

/* fifo */

void fifo_init(struct fifo *q, void *queue, size_t size)
{
	q->size  = size;
	q->buf   = queue;
	q->front = q->rear = 0;
}

void fifo_flush(struct fifo *q)
{
	q->front = q->rear = 0;
}

int fifo_get(struct fifo *q, int type_size)
{
	unsigned int sentinel, index;
	unsigned char *p;
	register unsigned int i;
	int v = 0;

	if (!q || !q->buf)
		return EINVAL;

	sentinel = q->size / type_size;
	index = q->front * type_size;
	p = q->buf;

	if (q->front == q->rear) /* empty */
		return ERANGE;

	for (i = 0; i < type_size; i++)
		v = (unsigned int)v | (p[index + i] << (i << 3));

	q->front += 1;
	q->front %= sentinel;

	return v;
}

int fifo_put(struct fifo *q, int value, int type_size)
{
	unsigned int sentinel, index;
	unsigned char *p;
	register unsigned int i;

	if (!q || !q->buf)
		return EINVAL;

	sentinel = q->size / type_size;
	index = q->rear * type_size;
	p = q->buf;

	if (((q->rear+1) % sentinel) == q->front) /* no more room */
		return ERANGE;

	for (i = 0; i < type_size; i++) {
		p[index + i] = (unsigned char)value;
		value        = (unsigned int)value >> 8;
	}

	q->rear += 1;
	q->rear %= sentinel;

	return value;
}
