#ifndef __SCHED_H__
#define __SCHED_H__

#include <types.h>

struct sched_t {
	int nr_running;
	int pri;
	uint64_t vruntime_base;
	
	void *rq;
};

void scheduler_init();

void inline update_curr();

#include <foundation.h>
#include <asm/context.h>

#define schedule_prepare() { \
	irq_save(current->primask); \
	cli(); \
	context_save(current->sp); \
}
#define schedule_finish() { \
	context_restore(current->sp); \
	irq_restore(current->primask); \
}

void schedule_core();

#include <asm/clock.h>

#define schedule_on()	systick_on()
#define schedule_off()	systick_off()

#include <kernel/task.h>

void runqueue_add(struct task_t *new);

#include <kernel/cfs.h>
#include <kernel/rts.h>

/* from time.c but used only in the critical region like scheduler. */
extern unsigned long long inline __get_jiffies_64();
extern void inline update_tick(unsigned delta);

#endif /* __SCHED_H__ */
