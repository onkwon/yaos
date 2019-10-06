#ifndef __YAOS_IO_H__
#define __YAOS_IO_H__

#include "compiler.h"
#include "arch/hw_io.h"
#include "arch/atomic.h"

/** IRQ priority */
enum irq_pri {
	__IRQ_PRIORITY_HIGHEST	= 0,
	IRQ_PRIORITY_HIGHEST	= 1,
	IRQ_PRIORITY_DEFAULT	= 4,
	IRQ_PRIORITY_LOWEST	= 7,
};

/** Data memory barrier */
#define dmb()				__dmb()
/** Data synchronization barrier */
#define dsb()				__dsb()
/** Instruction synchronization barrier */
#define isb()				__isb()

#define set_user_sp(val)		__set_usp(val)
#define set_kernel_sp(val)		__set_ksp(val)

/** Test if in interrupt context */
#define in_interrupt()			__in_interrupt()
/** Test if interrupts disabled */
#define is_interrupt_disabled()		__is_interrupt_disabled()
/** Test if in privileged mode */
#define is_privileged()			__is_privileged()

/** Test if it has the right permission */
#define is_honored()			(in_interrupt() || is_privileged())

#endif /* __YAOS_IO_H__ */
