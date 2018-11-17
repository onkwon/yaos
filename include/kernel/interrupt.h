#ifndef __YAOS_INTERRUPT_H__
#define __YAOS_INTERRUPT_H__

#include "arch/interrupt.h"

#define irq_save(val)			__irq_save(val)
#define irq_restore(val)		__irq_restore(val)
#define cli()				__cli()
#define sei()				__sei()
#define local_irq_disable()		cli()
#define local_irq_enable()		sei()
#define get_active_irq()		__get_active_irq()

#define preempt_disable()		cli()
#define preempt_enable()		sei()
#define preempt_count()

#define disable_irq(val)		do {	\
	irq_save(val);				\
	local_irq_disable();			\
} while (0)
#define enable_irq(val)			do {	\
	irq_restore(val);			\
} while (0)
#define ENTER_CRITICAL(val)		disable_irq(val)
#define LEAVE_CRITICAL(val)		enable_irq(val)

#endif /* __YAOS_INTERRUPT_H__ */
