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

inline void update_curr();

#include <foundation.h>
#include <asm/context.h>

#define schedule_prepare() { \
	irq_save(current->primask); \
	local_irq_disable(); \
	context_save(current); \
}
#define schedule_finish() { \
	context_restore(current); \
	irq_restore(current->primask); \
}

void schedule_core();

#ifdef CONFIG_SYSCALL
#include <syscall.h>
#define schedule()	syscall(SYSCALL_SCHEDULE)
#else
#define schedule()	sys_schedule()
#endif

#include <asm/clock.h>

#define schedule_on()	systick_on()
#define schedule_off()	systick_off()

#include <kernel/task.h>

extern inline void runqueue_add(struct task_t *new);

#include <kernel/cfs.h>
#ifdef CONFIG_REALTIME
#include <kernel/rts.h>
#endif

/* from time.c but used only in the critical region like scheduler. */
extern inline unsigned long long get_jiffies_64_core();
extern inline void update_tick(unsigned delta);

#endif /* __SCHED_H__ */
