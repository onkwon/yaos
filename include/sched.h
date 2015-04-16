#ifndef __SCHED_H__
#define __SCHED_H__

#include <task.h>

void runqueue_add(struct task_t *p);
void runqueue_del(struct task_t *p);

#include <io.h>
#include <asm/context.h>

#define schedule_prepare() { \
	irq_save(current->primask); \
	cli(); \
	context_save(current->stack); \
}
#define schedule_finish() { \
	context_restore(current->stack); \
	irq_restore(current->primask); \
}

void schedule_core();

#endif /* __SCHED_H__ */
