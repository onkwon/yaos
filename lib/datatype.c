#include <types.h>

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
	unsigned int sentinel = q->size / type_size;
	unsigned int index    = q->front * type_size;
	unsigned char *p      = q->buf;
	int v = 0;
	register unsigned int i;

	if (q->front == q->rear) /* empty */
		return -1;

	for (i = 0; i < type_size; i++)
		v = (unsigned int)v | (p[index + i] << (i << 3));

	q->front += 1;
	q->front %= sentinel;

	return v;
}

int fifo_put(struct fifo *q, int value, int type_size)
{
	unsigned int sentinel = q->size / type_size;
	unsigned int index    = q->rear * type_size;
	unsigned char *p      = q->buf;
	register unsigned int i;

	if (((q->rear+1) % sentinel) == q->front) /* no more room */
		return -1;

	for (i = 0; i < type_size; i++) {
		p[index + i] = (unsigned char)value;
		value        = (unsigned int)value >> 8;
	}

	q->rear += 1;
	q->rear %= sentinel;

	return value;
}
