#ifndef __ARMv7M_IO_H__
#define __ARMv7M_IO_H__

/* Use these macros where needs atomic operation. */
#define GET_BITBAND_ADDR(base, offset, bit) \
	((base) + ((offset) << 5) + ((bit) << 2))
#define GET_BITBAND(addr, bit) \
	GET_BITBAND_ADDR(((unsigned int)(addr) & 0xf0000000) \
			+ 0x02000000, \
			((unsigned int)(addr) & 0xfffff), bit)
#define BITBAND(addr, bit, on) \
	(*(reg_t *)GET_BITBAND(addr, bit) = on)

#define __nop()			__asm__ __volatile__("nop")

#define __ret()			__asm__ __volatile__("bx lr")
#define __ret_from_exc(offset)

#define __get_pc() ({							\
	unsigned int __pc;						\
	__asm__ __volatile__("mov %0, pc" : "=r"(__pc));		\
	__pc;								\
})
#define __get_sp() ({							\
	unsigned int __sp;						\
	__asm__ __volatile__("mov %0, sp" : "=r"(__sp));		\
	__sp;								\
})
#define __get_ksp() ({							\
	unsigned int __ksp;						\
	__asm__ __volatile__("mrs %0, msp" : "=r"(__ksp));		\
	__ksp;								\
})
#define __get_usp() ({							\
	unsigned int __usp;						\
	__asm__ __volatile__("mrs %0, psp" : "=r"(__usp));		\
	__usp;								\
})
#define __get_psr() ({							\
	unsigned int __psr;						\
	__asm__ __volatile__("mrs %0, psr" : "=r"(__psr));		\
	__psr;								\
})
#define __get_lr() ({							\
	unsigned int __lr;						\
	__asm__ __volatile__("mov %0, lr" : "=r"(__lr));		\
	__lr;								\
})
#define __get_primask() ({						\
	unsigned int __primask;						\
	__asm__ __volatile__("mrs %0, primask" : "=r"(__primask));	\
	__primask;							\
})
#define __get_cntl() ({							\
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
#define __get_ret_addr()	__get_lr()

#include "regs.h"

#undef  INCPATH
#define INCPATH			<asm/mach-MACHINE/io.h>
#include INCPATH

#endif /* __ARMv7M_IO_H__ */
