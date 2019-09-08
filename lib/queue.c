#include "queue.h"
#include "io.h"

#include <errno.h>
#include <assert.h>
#include <string.h>

int enqueue(queue_t *q, const void * const item)
{
	assert(q && q->data);

	uint16_t pos;
	uint8_t *arr = q->data;

	do {
		pos = atomic_llh(&q->index);

		if (((pos + 1) % q->n) == ACCESS_ONCE(q->outdex))
			return -ENOSPC; /* full */

		memcpy(&arr[pos * q->itemsize], item, q->itemsize);
		pos = (pos + 1) % q->n;
	} while (atomic_sch(pos, &q->index));

	return 0;
}

int dequeue(queue_t *q, void * const buf)
{
	assert(q && q->data && buf);

	uint16_t pos;
	uint8_t *arr;
	
	arr = q->data;

	do {
		pos = atomic_llh(&q->outdex);

		if (pos == ACCESS_ONCE(q->index))
			return -ENOENT; /* empty */

		memcpy(buf, &arr[pos * q->itemsize], q->itemsize);
		pos = (pos + 1) % q->n;
	} while (atomic_sch(pos, &q->outdex));

	return 0;
}

int queue_peek(queue_t *q, void * const buf)
{
	assert(q && q->data && buf);

	uint16_t pos;
	uint8_t *arr;
	
	arr = q->data;

	do {
		pos = atomic_llh(&q->outdex);

		if (pos == ACCESS_ONCE(q->index))
			return -ENOENT; /* empty */

		memcpy(buf, &arr[pos * q->itemsize], q->itemsize);
	} while (atomic_sch(pos, &q->outdex));

	return 0;
}

void queue_flush(queue_t *q)
{
	assert(q);

	q->index = q->outdex = 0;
}

bool queue_empty(const queue_t * const q)
{
	assert(q);

	return q->outdex == q->index;
}

int queue_count(const queue_t * const q)
{
	assert(q);

	int cnt = q->index - q->outdex;

	if (0 > cnt)
		cnt += q->n;

	return cnt;
}

bool queue_is_initialized(const queue_t * const q)
{
	if (!q || !q->data || (0 == q->n) || (0 == q->itemsize))
		return false;

	return true;
}

void queue_init_static(queue_t *q, void *arr, uint16_t n, uint8_t itemsize)
{
	assert(q);

	q->index = q->outdex = 0U;
	q->data = arr;
	q->n = n;
	q->itemsize = itemsize;

	assert(q->data);
	assert(q->n && (q->n <= QUEUE_MAX_ITEM));
	assert(q->itemsize);
}
