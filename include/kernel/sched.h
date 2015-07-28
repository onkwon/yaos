#ifndef __SCHED_H__
#define __SCHED_H__

#include <types.h>

struct sched_entity {
	uint64_t vruntime;
	uint64_t exec_start;
	uint64_t sum_exec_runtime;
} __attribute__((packed));

#define INIT_SCHED_ENTITY(name)	((name) = (struct sched_entity){ 0, 0, 0 })

struct scheduler {
	int nr_running;
	int pri;
	uint64_t vruntime_base;
	
	void *rq;
};

#define schedule_prepare() { \
	irq_save(current->irqflag); \
	local_irq_disable(); \
	__context_save(current); \
}
#define schedule_finish() { \
	__context_restore(current); \
	irq_restore(current->irqflag); \
}

void schedule_core();
void scheduler_init();
extern inline void update_curr();
struct task;
extern inline void runqueue_add(struct task *new);
extern inline void runqueue_del(struct task *task);

void sys_yield();

#include <asm/context.h>
#include <kernel/syscall.h>

#ifdef CONFIG_SYSCALL
#define schedule()	syscall(SYSCALL_SCHEDULE)

static inline void yield()
{
	syscall(SYSCALL_YIELD);
}
#else
#define schedule()	sys_schedule()
#define yield()		sys_yield()
#endif

#include <asm/clock.h>

#define schedule_on()	systick_on()
#define schedule_off()	systick_off()

#endif /* __SCHED_H__ */
