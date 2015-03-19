#ifndef __SCHED_H__
#define __SCHED_H__

#include "context.h"
#include "task.h"

#define EXC_RETURN_MSPH	0xfffffff1	/* return to HANDLER mode using MSP */
#define EXC_RETURN_MSPT	0xfffffff9	/* return to THREAD  mode using MSP */
#define EXC_RETURN_PSPT	0xfffffffd	/* return to THREAD  mode using PSP */

void runqueue_add(struct task_t *p);
void runqueue_del(struct task_t *p);

#include "foundation.h"

#define schedule()	__asm__ __volatile__("svc 0");
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
