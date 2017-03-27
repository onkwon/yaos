#ifndef __SOFTIRQ_H__
#define __SOFTIRQ_H__

#include <kernel/lock.h>
#include <kernel/task.h>
#include <error.h>

#define SOFTIRQ_MAX			(WORD_SIZE * 8)

struct __softirq {
	void (*action)();
	void *args;
	int priority;
	int overrun;
};

struct softirq {
	unsigned int pending;
	unsigned int bitmap;
	lock_t wlock;

	struct __softirq pool[SOFTIRQ_MAX];
};

struct softirq softirq;

static inline bool raise_softirq_atomic(unsigned int nr, void *args)
{
	if (softirq.pending & (1 << nr)) {
		softirq.pool[nr].overrun++;
		return false;
	}

	assert(!is_locked(softirq.wlock));
	lock_atomic(&softirq.wlock);
	softirq.pending |= (1 << nr);
	softirq.pool[nr].args = args;
	unlock_atomic(&softirq.wlock);

	return true;
}

static inline bool raise_softirq(unsigned int nr, void *args)
{
	unsigned int irqflag;
	bool ret;

	spin_lock_irqsave(&softirq.wlock, irqflag);
	ret = raise_softirq_atomic(nr, args);
	spin_unlock_irqrestore(&softirq.wlock, irqflag);

	return ret;
}

struct task *softirqd;

unsigned int request_softirq(void (*func)(), int pri);
int softirq_init();

#endif /* __SOFTIRQ_H__ */
