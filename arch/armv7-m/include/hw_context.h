#ifndef __YAOS_HW_CONTEXT_H__
#define __YAOS_HW_CONTEXT_H__

#include "regs.h"

/* raise pendsv for scheduling */
#define hw_raise_sched()	(SCB_ICSR |= 1UL << 28)

#endif /* __YAOS_HW_CONTEXT_H__ */
