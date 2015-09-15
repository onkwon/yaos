#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <asm/interrupt.h>

#define irq_save(flag)			__irq_save(flag)
#define irq_restore(flag)		__irq_restore(flag)
#define cli()				__cli()
#define sei()				__sei()
#define local_irq_disable()		cli()
#define local_irq_enable()		sei()
#define get_active_irq()		__get_active_irq()

#define preempt_disable()		cli()
#define preempt_enable()		sei()
#define preempt_count()

#define dmb()				__dmb()
#define dsb()				__dsb()
#define isb()				__isb()

extern int register_isr(unsigned int nirq, void (*func)());

#define set_user_sp(sp)			__set_usp(sp)
#define set_kernel_sp(sp)		__set_ksp(sp)

#endif /* __INTERRUPT_H__ */
