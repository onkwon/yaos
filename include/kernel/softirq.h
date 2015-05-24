#ifndef __SOFTIRQ_H__
#define __SOFTIRQ_H__

#define SOFTIRQ_MAX	32

struct softirq_t {
	unsigned int pending;
	unsigned int bitmap;

	void *call[SOFTIRQ_MAX];
};

struct softirq_t softirq_pool;

static inline void raise_softirq(unsigned int nr)
{
	softirq_pool.pending |= nr;
}

#include <kernel/task.h>

unsigned int register_softirq(struct task_t *task);
void softirq_init();

#endif /* __SOFTIRQ_H__ */
