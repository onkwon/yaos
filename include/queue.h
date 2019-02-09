#ifndef __YAOS_QUEUE_H__
#define __YAOS_QUEUE_H__

#include <stdint.h>
#include <stdbool.h>

#define QUEUE_MAX_ITEM			((1UL << 16) - 1)

/** A FIFO queue. It can holds (2^16 - 1) items at most.
 * The maximum item size is up to (2^8 - 1). */
typedef struct queue {
	void *data;
	uint16_t index;
	uint16_t outdex;
	uint16_t n; /** Number of items */
	uint8_t itemsize;
} queue_t;

/** Enqueue an item in a queue
 *
 * @param q A pointer to a queue
 * @param item A pointer to an item to store in a queue
 * @return 0 on success or negative errno, `-ENOSPC` when full
 */
int enqueue(queue_t *q, const void * const item);
/** Dequeue an item from a queue
 *
 * @param q A pointer to a queue
 * @param buf A buffer to move an item to
 * @return 0 on success or negative errno, `-ENOENT` when empty
 */
int dequeue(queue_t *q, void * const buf);

int queue_init(queue_t *q, uint16_t n, uint8_t itemsize);
/** Initialize a queue statically
 *
 * @param q A pointer to a queue
 * @param arr An array to store data.
 *            It must be big enough to save :c:data:`n` * itemsize
 * @param n Number of items the queue holds up to
 * @param itemsize Size of an item
 */
void queue_init_static(queue_t *q, void *arr, uint16_t n, uint8_t itemsize);
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
 * @return Number of items stored in the queue at the moment
 */
int queue_count(const queue_t * const q);
/** Peek an oldest item in a queue
 *
 * @param q A pointer to a queue
 * @param buf A buffer to move an item to
 * @return 0 on success or negative errno
 */
int queue_peek(queue_t *q, void * const buf);
/** Check if initialized
 *
 * @param q A pointer to a queue
 */
bool queue_is_initialized(const queue_t * const q);

#endif /* __YAOS_QUEUE_H__ */
