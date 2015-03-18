#ifndef __SCHED_H__
#define __SCHED_H__

#include "context.h"
#include "task.h"

#define EXC_RETURN_MSPH	0xfffffff1	/* return to HANDLER mode using MSP */
#define EXC_RETURN_MSPT	0xfffffff9	/* return to THREAD  mode using MSP */
#define EXC_RETURN_PSPT	0xfffffffd	/* return to THREAD  mode using PSP */

void runqueue_add(struct task_t *p);
void runqueue_del(struct task_t *p);

#define schedule()	__asm__ __volatile__("svc 0");

#endif /* __SCHED_H__ */
