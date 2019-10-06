#ifndef __YAOS_INTERRUPT_H__
#define __YAOS_INTERRUPT_H__

#include "arch/hw_interrupt.h"

#define irq_save(val)				__irq_save(val)
#define irq_restore(val)			__irq_restore(val)
#define cli()					__cli()
#define sei()					__sei()
#define local_irq_disable()			cli()
#define local_irq_enable()			sei()
#define get_active_irq()			__get_active_irq()

#define preempt_disable()			cli()
#define preempt_enable()			sei()
#define preempt_count()

#define disable_irq(val)	do {		\
	irq_save(val);				\
	local_irq_disable();			\
} while (0)
#define enable_irq(val)		do {		\
	irq_restore(val);			\
} while (0)
#define ENTER_CRITICAL(val)			disable_irq(val)
#define LEAVE_CRITICAL(val)			enable_irq(val)

#if defined(CONFIG_COMMON_IRQ_HANDLER)
#define get_active_irq_from_isr(vector)		vector
#else
#define get_active_irq_from_isr(vector)		get_active_irq()
#endif

/** Never call theses functions below in interrupt context */

/**
 * Register an ISR.
 *
 * @param lvec Logical vector number
 * @param handler Function pointer to a constructor
 * @return Logical vector number on success or negative value in case of
 *         failure, refer to `errno.h`
 */
int register_isr(const int lvec, void (*handler)(const int));
/**
 * Unregister ISR
 *
 * @param lvec Logical vector number
 * @return 0 on success or refer to `errno.h`
 */
int unregister_isr(const int lvec);

void irq_init(void);

#endif /* __YAOS_INTERRUPT_H__ */
