#ifndef __YAOS_ARMv7M_IO_H__
#define __YAOS_ARMv7M_IO_H__

#include <stdint.h>

#if !defined(TEST)
/* Use these macros where needs atomic operation. */
#define GET_BITBAND_ADDR(base, offset, bit) \
	((base) + ((offset) << 5) + ((bit) << 2))
#define GET_BITBAND(addr, bit) \
	GET_BITBAND_ADDR(((uintptr_t)(addr) & 0xf0000000UL) \
			+ 0x02000000UL, \
			((uintptr_t)(addr) & 0xfffffUL), bit)
#define BITBAND(addr, bit, on) \
	(*(reg_t *)GET_BITBAND(addr, bit) = on)

#define __nop()			__asm__ __volatile__("nop")

#define __dmb()			__asm__ __volatile__("dmb" ::: "memory")
#define __dsb()			__asm__ __volatile__("dsb" ::: "memory")
#define __isb()			__asm__ __volatile__("isb" ::: "memory")

#define __ret()			__asm__ __volatile__("bx lr")
#define __ret_from_exc(offset)

#define __get_pc() __extension__ ({					\
	unsigned int __pc;						\
	__asm__ __volatile__("mov %0, pc" : "=r"(__pc));		\
	__pc;								\
})
#define __get_sp() __extension__ ({					\
	unsigned int __sp;						\
	__asm__ __volatile__("mov %0, sp" : "=r"(__sp));		\
	__sp;								\
})
#define __get_ksp() __extension__ ({					\
	unsigned int __ksp;						\
	__asm__ __volatile__("mrs %0, msp" : "=r"(__ksp));		\
	__ksp;								\
})
#define __get_usp() __extension__ ({					\
	unsigned int __usp;						\
	__asm__ __volatile__("mrs %0, psp" : "=r"(__usp));		\
	__usp;								\
})
#define __get_psr() __extension__ ({					\
	unsigned int __psr;						\
	__asm__ __volatile__("mrs %0, psr" : "=r"(__psr));		\
	__psr;								\
})
#define __get_lr() __extension__ ({					\
	unsigned int __lr;						\
	__asm__ __volatile__("mov %0, lr" : "=r"(__lr));		\
	__lr;								\
})
#define __get_primask() __extension__ ({				\
	unsigned int __primask;						\
	__asm__ __volatile__("mrs %0, primask" : "=r"(__primask));	\
	__primask;							\
})
#define __get_cntl() __extension__ ({					\
	unsigned int __control;						\
	__asm__ __volatile__("mrs %0, control" : "=r"(__control));	\
	__control;							\
})
#define __set_sp(sp)							\
	__asm__ __volatile__("mov sp, %0" :: "r"(sp))
#define __set_ksp(sp)							\
	__asm__ __volatile__("msr msp, %0" :: "r"(sp))
#define __set_usp(sp)							\
	__asm__ __volatile__("msr psp, %0" :: "r"(sp))
#endif /* !defined(TEST) */

#define __get_ret_addr()	__get_lr()

#define __get_active_irq()	(__get_psr() & 0x1ff)
#define __in_interrupt()	__get_active_irq()

#include "arch/mach/io.h" 

#endif /* __YAOS_ARMv7M_IO_H__ */
