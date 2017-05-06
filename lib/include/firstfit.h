#ifndef __FIRSTFIT_H__
#define __FIRSTFIT_H__

#include <types.h>

#define FF_METASIZE	(sizeof(struct ff_freelist) - sizeof(struct link))
#define FF_DATA_OFFSET	sizeof(size_t)
#define FF_MARK_TAG(p)	\
	(*(struct ff_freelist **) \
	 ((unsigned int)p + p->size + FF_DATA_OFFSET) = p)

#define FF_MARK_ALLOCATED(p)	(p->size |= 1)
#define FF_MARK_FREE(p)		(p->size &= ~1)
#define FF_IS_FREE(p)		(!(p->size & 1))

struct ff_freelist_head {
	struct link list_head;
	void *base;
	void *limit;
};

struct ff_freelist {
	size_t size; /* keep in the first */
	struct link list;
	struct ff_freelist *head; /* keep at the end */
} __attribute__((packed));

size_t ff_freelist_init(struct ff_freelist_head *pool, void *start, void *end);

void *ff_alloc(struct ff_freelist_head *pool, size_t size);
void ff_free(struct ff_freelist_head *pool, void *addr);
size_t show_freelist(struct ff_freelist_head *pool);

#endif /* __FIRSTFIT_H__ */
