#ifndef __TYPES_H__
#define __TYPES_H__

#define NULL			((void *)0)
#define EOF			(-1)

#define HIGH			1
#define LOW			0
#define ON			1
#define OFF			0
#define ENABLE			1
#define DISABLE			0

typedef enum {FALSE = 0, TRUE = 1} bool;
typedef unsigned size_t;

#define get_container_of(ptr, type, member) \
		((type *)((char *)ptr - (char *)&((type *)0)->member))

struct list_t {
	struct list_t *next;
	struct list_t *prev;
};

#define LIST_HEAD_INIT(name) 	{ &(name), &(name) }
#define LIST_HEAD(name) \
		struct list_t name = LIST_HEAD_INIT(name)

void LIST_LINK_INIT(struct list_t *list);
extern inline void list_add(struct list_t *new, struct list_t *ref);
extern inline void list_del(struct list_t *item);

struct fifo_t {
	char *buf;
	int  size;
	int  head, tail;
};

void fifo_init(struct fifo_t *f, char *buf, int size);
extern inline int fifo_get(struct fifo_t *f, void *buf, int size);
extern inline int fifo_put(struct fifo_t *f, void *buf, int size);

#define SWAP_WORD(word)	\
		((word >> 24) | (word << 24) | ((word >> 8) & 0xff00) | ((word << 8) & 0xff0000))

#endif /* __TYPES_H__ */
