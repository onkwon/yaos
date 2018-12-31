#ifndef __YAOS_QUEUE_H__
#define __YAOS_QUEUE_H__

#include <stdint.h>
#include <stdbool.h>

#define QUEUE_MAX_ITEM			((1UL << 31) - 1)

/** A FIFO queue. It can holds (2^31 - 1) items at most.
 * The maximum item size is up to 2^8. */
typedef struct queue {
	void *data;
	uintptr_t front;
	uintptr_t rear;
	uintptr_t n; /** Number of items */
	uint8_t itemsize;
} queue_t;

typedef struct queue_item {
	union {
		uint8_t _d8;
		uint16_t _d16;
		uint32_t _d32;
		uint8_t _d[1];
	} s;
} queue_item_t;

/** Make a parameter to the :c:data:`queue_item_t` type. */
#define QUEUE_ITEM(x)			(*(queue_item_t *)&(x))

/** Enqueue an item in a queue
 *
 * @param q A pointer to a queue
 * @param item An item to store in a queue, use the :c:data:`QUEUE_ITEM()` macro
 * @return 0 on success or negative errno, `-ENOSPC` when full
 */
int enqueue(queue_t *q, const queue_item_t item);
/** Dequeue an item from a queue
 *
 * @param q A pointer to a queue
 * @param buf A buffer to save an item from a queue
 * @return 0 on success or negative errno, `-ENOENT` when empty
 */
int dequeue(queue_t *q, void * const buf);

int queue_init(queue_t *q, uintptr_t n, uint8_t itemsize);
/** Initialize a queue statically
 *
 * @param q A pointer to a queue
 * @param n Number of items the queue holds up to
 * @param itemsize Number of items the queue holds up to
 * @param arr An array to store data in the queue.
 *            It must be big enough to save :c:data:`n` items
 */
void queue_init_static(queue_t *q, uintptr_t n, uint8_t itemsize, void *arr);
/** Flush a queue
 *
 * @param q A pointer to a queue
 */
void queue_flush(queue_t *q);
/** Test a queue if empty
 *
 * @param q A pointer to a queue
 * @return true if empty or false
 */
bool queue_empty(const queue_t * const q);
/** Count a number of items stored in a queue
 *
 * @param q A pointer to a queue
 * @return Number of items stored at the moment
 */
int queue_count(const queue_t * const q);
/** Peek an oldest item in a queue
 *
 * @param q A pointer to a queue
 * @param buf A buffer to save an item from a queue
 * @return 0 on success or negative errno
 */
int queue_peek(queue_t *q, void * const buf);

#endif /* __YAOS_QUEUE_H__ */
