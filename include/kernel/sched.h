#ifndef __SCHED_H__
#define __SCHED_H__

#include <types.h>

struct sched_entity {
	uint64_t vruntime;
	uint64_t exec_start;
	uint64_t sum_exec_runtime;
};

#define INIT_SCHED_ENTITY(name)		\
	((name) = (struct sched_entity){ 0, 0, 0 })

struct scheduler {
	int nr_running;
	int pri;
	uint64_t vruntime_base;
	
	void *rq;
};

void schedule_core();
void scheduler_init();

#include <kernel/task.h>
struct task;
void runqueue_add_core(struct task *new);
void runqueue_del_core(struct task *task);
void runqueue_add(struct task *new);
void runqueue_del(struct task *task);
void sum_curr_stat(struct task *to);
unsigned int nr_running();

void sys_yield();

#include <asm/context.h>
#include <kernel/syscall.h>

extern void resched();
extern void sys_schedule();
extern void set_task_context(struct task *p, void *addr);
extern void set_task_context_soft(struct task *p);
extern void set_task_context_hard(struct task *p, void *addr);

#ifdef CONFIG_SYSCALL
#define schedule()			syscall(SYSCALL_SCHEDULE)

static inline void yield()
{
	syscall(SYSCALL_YIELD);
}
#else
#define schedule()			sys_schedule()
#define yield()				sys_yield()
#endif

#include <asm/sysclk.h>

void run_scheduler(bool run);

#endif /* __SCHED_H__ */
