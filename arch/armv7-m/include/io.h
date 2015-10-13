#ifndef __ARMv7M_IO_H__
#define __ARMv7M_IO_H__

/* Use these macros where needs atomic operation. */
#define GET_BITBAND_ADDR(base, offset, bit) \
		((base) + ((offset) << 5) + ((bit) << 2))
#define GET_BITBAND(addr, bit) \
		GET_BITBAND_ADDR(((unsigned int)(addr) & 0xf0000000) \
				+ 0x02000000, \
				((unsigned int)(addr) & 0xfffff), bit)
#define BITBAND(addr, bit, v) \
		(*(volatile unsigned int *)GET_BITBAND(addr, bit) = v)

/* NVIC, Cortex-M3's internal peripherals */
#define NVIC_BASE		(0xE000E100)
#define NVIC_ISER0		(*(volatile unsigned int *)0xE000E100)
#define NVIC_ISER1		(*(volatile unsigned int *)(NVIC_BASE + 4))
#define NVIC_ISER2		(*(volatile unsigned int *)(NVIC_BASE + 8))
#define NVIC_ICER		(*(volatile unsigned int *)(NVIC_BASE + 0x80))
#define NVIC_ISPR		(*(volatile unsigned int *)(NVIC_BASE + 0x100))
#define NVIC_ICPR		(*(volatile unsigned int *)(NVIC_BASE + 0x180))
#define NVIC_IABR		(*(volatile unsigned int *)(NVIC_BASE + 0x200))
#define NVIC_IPR		(*(volatile unsigned int *)(NVIC_BASE + 0x300))
#define NVIC_STIR		(*(volatile unsigned int *)(NVIC_BASE + 0xe00))
		
/* System Control Block(SCB), Cortex-M3's internal peripherals */
#define SCB_BASE		(0xE000ED00)
#define SCB_CPUID		(*(volatile unsigned int *)SCB_BASE)
#define SCB_ICSR		(*(volatile unsigned int *)(SCB_BASE + 4))
#define SCB_VTOR		(*(volatile unsigned int *)(SCB_BASE + 8))
#define SCB_AIRCR		(*(volatile unsigned int *)(SCB_BASE + 0xc))
#define SCB_SCR 		(*(volatile unsigned int *)(SCB_BASE + 0x10))
#define SCB_CCR 		(*(volatile unsigned int *)(SCB_BASE + 0x14))
#define SCB_SHPR1		(*(volatile unsigned int *)(SCB_BASE + 0x18))
#define SCB_SHPR2		(*(volatile unsigned int *)(SCB_BASE + 0x1c))
#define SCB_SHPR3		(*(volatile unsigned int *)(SCB_BASE + 0x20))
#define SCB_SHCSR		(*(volatile unsigned int *)(SCB_BASE + 0x24))
#define SCB_CFSR		(*(volatile unsigned int *)(SCB_BASE + 0x28))
#define SCB_HFSR		(*(volatile unsigned int *)(SCB_BASE + 0x2c))
#define SCB_MMFAR		(*(volatile unsigned int *)(SCB_BASE + 0x34))
#define SCB_BFAR		(*(volatile unsigned int *)(SCB_BASE + 0x38))

/* Systick */
#define SYSTICK_BASE		(0xe000e010)
#define STK_CTRL		(*(volatile unsigned int *)SYSTICK_BASE)
#define STK_LOAD		(*(volatile unsigned int *)(SYSTICK_BASE + 4))
#define STK_VAL 		(*(volatile unsigned int *)(SYSTICK_BASE + 8))
#define STK_CALIB		(*(volatile unsigned int *)(SYSTICK_BASE + 0xc))

#undef  INCPATH
#define INCPATH			<asm/mach-MACHINE/SOC.h>
#include INCPATH

#endif /* __ARMv7M_IO_H__ */
