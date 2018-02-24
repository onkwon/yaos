#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define NULL				((void *)0)
#define EOF				(-1)
#define INF				(-1)

#define UNLOCKED			1

enum {
	LOW	= false,
	OFF	= false,
	DISABLE	= false,
	disable	= false,
	HIGH	= true,
	ON	= true,
	ENABLE	= true,
	enable	= true,
};

typedef volatile int lock_t;
typedef unsigned int dev_t;
struct links;
typedef struct links buf_t;
typedef volatile unsigned int reg_t;
typedef struct ff_freelist_head heap_t;
typedef enum {
	SLEEP_NAP	= 1,
	SLEEP_DEEP	= 2,
	SLEEP_BLACKOUT	= 3,
} sleep_t;

#define WORD_SIZE			sizeof(int)
#define WORD_BITS			(WORD_SIZE << 3)

#define max(a, b)			({ \
		__typeof__(a) _a = (a); \
		__typeof__(b) _b = (b); \
		_a > _b ? _a : _b; \
})
#define min(a, b)			({ \
		__typeof__(a) _a = (a); \
		__typeof__(b) _b = (b); \
		_a < _b ? _a : _b; \
})

#define BASE_ALIGN(x, a)		((x) & ~((typeof(x))(a) - 1UL))
#define ALIGN(x, a)			\
	BASE_ALIGN((x) + ((typeof(x))(a) - 1UL), a)

#define BASE_WORD(x)			BASE_ALIGN(x, WORD_SIZE)
#define ALIGN_WORD(x)			ALIGN(x, WORD_SIZE)
#define BASE_DWORD(x)			BASE_ALIGN(x, WORD_SIZE << 1)
#define ALIGN_DWORD(x)			ALIGN(x, WORD_SIZE << 1)

#define get_container_of(ptr, type, member) \
	((type *)((char *)(ptr) - (char *)&((type *)0)->member))

static inline bool is_pow2(unsigned int x)
{
	return !(x & (x - 1));
}

#define stringify(x)		#x
#define def2str(x)		stringify(x)

/* doubly-linked list */
struct links {
	struct links *next, *prev;
};

#define INIT_LINKS_HEAD(name) 		{ &(name), &(name) }
#define DEFINE_LINKS_HEAD(name)		\
	struct links name = INIT_LINKS_HEAD(name)

static inline void links_init(struct links *node)
{
	node->next = node;
	node->prev = node;
}

static inline void links_add(struct links *new, struct links *ref)
{
	new->prev = ref;
	new->next = ref->next;
	ref->next->prev = new;
	ref->next = new;
}

static inline void links_del(struct links *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

static inline bool links_empty(const struct links *node)
{
	return (node->next == node) && (node->prev == node);
}

/* singly-linked list */
struct link {
	struct link *next;
};

#define DEFINE_LINK_HEAD(name)		struct link name = { NULL }

static inline void link_init(struct link *node)
{
	node->next = NULL;
}

static inline void link_add(struct link *new, struct link *ref)
{
	new->next = ref->next;
	ref->next = new;
}

static inline void link_add_tail(struct link *new, struct link *head)
{
	struct link **curr = &head;

	while ((*curr) && (*curr)->next)
		curr = &(*curr)->next;

	new->next = (*curr)->next;
	(*curr)->next = new;
}

static inline void link_del(struct link *node, struct link *ref)
{
	struct link **curr = &ref;

	while (*curr && *curr != node)
		curr = &(*curr)->next;

	*curr = node->next;
}

static inline bool link_empty(const struct link *node)
{
	return node->next == NULL;
}

#define SWAP_WORD(word)	\
		((word >> 24) | (word << 24) | ((word >> 8) & 0xff00) | \
		 ((word << 8) & 0xff0000))

#endif /* __TYPES_H__ */
