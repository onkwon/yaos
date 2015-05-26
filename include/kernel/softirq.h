#ifndef __SOFTIRQ_H__
#define __SOFTIRQ_H__

#include <kernel/lock.h>
#include <kernel/task.h>

#define SOFTIRQ_MAX		(sizeof(int) * 8)

struct softirq_t {
	unsigned int pending;
	unsigned int bitmap;
	lock_t wlock;

	void (*call[SOFTIRQ_MAX])();
};

struct softirq_t softirq;

static inline void raise_softirq(unsigned int nr)
{
	unsigned int irqflag;
	spin_lock_irqsave(softirq.wlock, irqflag);
	softirq.pending |= (1 << nr);
	spin_unlock_irqrestore(softirq.wlock, irqflag);
}

struct task_t *softirqd;

unsigned int register_softirq(void (*func)());
int softirq_init();

#endif /* __SOFTIRQ_H__ */
