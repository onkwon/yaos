#ifndef __YAOS_SCHED_H__
#define __YAOS_SCHED_H__

#include "arch/hw_context.h"

#define resched()		hw_raise_sched()

void schedule(void);

#endif /* __YAOS_SCHED_H__ */
