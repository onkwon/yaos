#include "queue.h"
#include "io.h"
#include <errno.h>
#include <assert.h>
#include <string.h>

int enqueue(queue_t *q, const queue_item_t item)
{
	assert(q && q->data);

	uint16_t pos;
	uint8_t *arr = q->data;

	do {
		pos = atomic_llh(&q->rear);

		if (((pos + 1) % q->n) == ACCESS_ONCE(q->front))
			return -ENOSPC; /* full */

		memcpy(&arr[pos * q->itemsize], &item, q->itemsize);
		pos = (pos + 1) % q->n;
	} while (atomic_sch(pos, &q->rear));

	return 0;
}

int dequeue(queue_t *q, void * const buf)
{
	assert(q && q->data && buf);

	uint16_t pos;
	uint8_t *arr, *p;
	
	arr = q->data;
	p = buf;

	do {
		pos = atomic_llh(&q->front);

		if (pos == ACCESS_ONCE(q->rear))
			return -ENOENT; /* empty */

		memcpy(p, &arr[pos * q->itemsize], q->itemsize);
		pos = (pos + 1) % q->n;
	} while (atomic_sch(pos, &q->front));

	return 0;
}

int queue_peek(queue_t *q, void * const buf)
{
	assert(q && q->data && buf);

	uint16_t pos;
	uint8_t *arr, *p;
	
	arr = q->data;
	p = buf;

	do {
		pos = atomic_llh(&q->front);

		if (pos == ACCESS_ONCE(q->rear))
			return -ENOENT; /* empty */

		memcpy(p, &arr[pos * q->itemsize], q->itemsize);
	} while (atomic_sch(pos, &q->front));

	return 0;
}

void queue_flush(queue_t *q)
{
	assert(q);

	q->front = q->rear = 0;
}

bool queue_empty(const queue_t * const q)
{
	assert(q);

	return q->front == q->rear;
}

int queue_count(const queue_t * const q)
{
	assert(q);

	int cnt = q->rear - q->front;

	if (0 > cnt)
		cnt += q->n;

	return cnt;
}

void queue_init_static(queue_t *q, uint16_t n, uint8_t itemsize, void *arr)
{
	assert(q);

	q->front = q->rear = 0U;
	q->data = arr;
	q->n = n;
	q->itemsize = itemsize;

	assert(q->data);
	assert(q->n && (q->n <= QUEUE_MAX_ITEM));
	assert(q->itemsize);
}
