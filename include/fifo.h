#ifndef __YAOS_FIFO_H__
#define __YAOS_FIFO_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

struct fifo {
	void *buf;
	size_t n; /* number of items */
	uintptr_t front, rear;
};

void fifo_init(struct fifo *q, void *queue, size_t n);
void fifo_flush(struct fifo *q);

int fifo_get(struct fifo *q, void *p);
int fifo_put(struct fifo *q, uintptr_t val);

int fifo_getb(struct fifo *q);
int fifo_putb(struct fifo *q, int val);
int fifo_getw(struct fifo *q, void *p);
int fifo_putw(struct fifo *q, uintptr_t val);
bool fifo_empty(struct fifo *q);

#endif /* __YAOS_FIFO_H__ */
