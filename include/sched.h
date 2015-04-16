#ifndef __SCHED_H__
#define __SCHED_H__

#include <task.h>

void runqueue_add(struct task_t *p);
void runqueue_del(struct task_t *p);

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

#endif /* __SCHED_H__ */
