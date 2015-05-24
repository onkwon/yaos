#ifndef __SCHED_H__
#define __SCHED_H__

#include <types.h>

struct sched_entity {
	uint64_t vruntime;
	uint64_t exec_start;
	uint64_t sum_exec_runtime;
} __attribute__((packed));

#define INIT_SCHED_ENTITY(name)	((name) = (struct sched_entity){ 0, 0, 0 })

struct sched_t {
	int nr_running;
	int pri;
	uint64_t vruntime_base;
	
	void *rq;
};

void scheduler_init();

inline void update_curr();

#define schedule_prepare() { \
	irq_save(current->irqflag); \
	local_irq_disable(); \
	context_save(current); \
}
#define schedule_finish() { \
	context_restore(current); \
	irq_restore(current->irqflag); \
}

void schedule_core();

#ifdef CONFIG_SYSCALL
#include <kernel/syscall.h>
#define schedule()	syscall(SYSCALL_SCHEDULE)
#else
#include <asm/context.h>
#define schedule()	sys_schedule()
#endif

#include <asm/clock.h>

#define schedule_on()	systick_on()
#define schedule_off()	systick_off()

struct task_t;
extern inline void runqueue_add(struct task_t *new);

#include <kernel/cfs.h>
#ifdef CONFIG_REALTIME
#include <kernel/rts.h>
#endif

/* from time.c but used only in the critical region like scheduler. */
extern inline unsigned long long get_jiffies_64_core();
extern inline void update_tick(unsigned delta);

#endif /* __SCHED_H__ */
