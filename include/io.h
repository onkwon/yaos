#ifndef __YAOS_IO_H__
#define __YAOS_IO_H__

#include "arch/io.h"

enum {
	__IRQ_PRIORITY_HIGHEST	= 0,
	IRQ_PRIORITY_HIGHEST	= 1,
	IRQ_PRIORITY_DEFAULT	= 4,
	IRQ_PRIORITY_LOWEST	= 7,
};

#define barrier()			__asm__ __volatile__("" ::: "memory")

#define dmb()				__dmb()
#define dsb()				__dsb()
#define isb()				__isb()

#define set_user_sp(val)		__set_usp(val)
#define set_kernel_sp(val)		__set_ksp(val)

#define in_interrupt()			__in_interrupt()
#define is_interrupt_disabled()		(__get_primask() & 1UL)

#define is_honored()			(in_interrupt() || !(__get_cntl() & 1UL))

#endif /* __YAOS_IO_H__ */
