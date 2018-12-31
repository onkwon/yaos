#ifndef __YAOS_FIFO_H__
#define __YAOS_FIFO_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

struct fifo {
	void *data;
	size_t n; /* number of items */
	uintptr_t front, rear;
};

/** Initialize a fifo
 *
 * @param q A pointer to :c:data:`struct fifo`
 * @param arr A pointer to an array to store data
 * @param n Number of items that can be stored in :c:data:`arr`
 */
void fifo_init(struct fifo *q, void *arr, size_t n);
void fifo_flush(struct fifo *q);

int fifo_get(struct fifo *q, void *p);
int fifo_put(struct fifo *q, uintptr_t val);

int fifo_getb(struct fifo *q);
int fifo_putb(struct fifo *q, int val);
int fifo_getw(struct fifo *q, void *p);
int fifo_putw(struct fifo *q, uintptr_t val);
bool fifo_empty(struct fifo *q);

#endif /* __YAOS_FIFO_H__ */
