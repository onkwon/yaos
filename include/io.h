#ifndef __YAOS_IO_H__
#define __YAOS_IO_H__

#include "arch/io.h"

/** IRQ priority */
enum irq_pri {
	__IRQ_PRIORITY_HIGHEST	= 0,
	IRQ_PRIORITY_HIGHEST	= 1,
	IRQ_PRIORITY_DEFAULT	= 4,
	IRQ_PRIORITY_LOWEST	= 7,
};

/** Compiler barrier */
#define barrier()			__asm__ __volatile__("" ::: "memory")

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
#define is_interrupt_disabled()		(__get_primask() & 1UL)

/** Test if in privileged mode */
#define is_honored()			(in_interrupt() || !(__get_cntl() & 1UL))

#endif /* __YAOS_IO_H__ */
