#ifndef __YAOS_SCHED_H__
#define __YAOS_SCHED_H__

#include "arch/hw_context.h"
#include <stdint.h>

#define resched()			hw_raise_sched()

struct scheduler {
	int nr_running;
	//int pri;
	//uint64_t vruntime_base;
	void *rq;

	int (*add)(struct scheduler *self, void *new);
	void *(*next)(struct scheduler *self);
};

void schedule(void);
void sched_init(void);
int sched_yield(void);

int runqueue_add_core(void *task);
int runqueue_del_core(void *task);

#endif /* __YAOS_SCHED_H__ */
