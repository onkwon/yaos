#ifndef __FIFO_H__
#define __FIFO_H__

#include <stddef.h>
#include <stdbool.h>

struct fifo {
	size_t n; /* number of items */
	unsigned int front, rear;
	void *buf;
};

void fifo_init(struct fifo *q, void *queue, size_t n);
void fifo_flush(struct fifo *q);

int fifo_get(struct fifo *q, void *p);
int fifo_put(struct fifo *q, void *p);

int fifo_getb(struct fifo *q);
int fifo_putb(struct fifo *q, int val);
int fifo_getw(struct fifo *q, void *p);
int fifo_putw(struct fifo *q, int val);
bool fifo_empty(struct fifo *q);

#endif /* __FIFO_H__ */
