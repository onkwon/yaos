#ifndef __ARMv7M_REGS_H__
#define __ARMv7M_REGS_H__

/* NVIC, Cortex-M3's internal peripherals */
#define NVIC_BASE		(0xE000E100)
#define NVIC_ISER0		(*(reg_t *)0xE000E100)
#define NVIC_ISER1		(*(reg_t *)(NVIC_BASE + 4))
#define NVIC_ISER2		(*(reg_t *)(NVIC_BASE + 8))
#define NVIC_ICER		(*(reg_t *)(NVIC_BASE + 0x80))
#define NVIC_ISPR		(*(reg_t *)(NVIC_BASE + 0x100))
#define NVIC_ICPR		(*(reg_t *)(NVIC_BASE + 0x180))
#define NVIC_IABR		(*(reg_t *)(NVIC_BASE + 0x200))
#define NVIC_IPR		(*(reg_t *)(NVIC_BASE + 0x300))
#define NVIC_STIR		(*(reg_t *)(NVIC_BASE + 0xe00))
		
/* System Control Block(SCB), Cortex-M3's internal peripherals */
#define SCB_BASE		(0xE000ED00)
#define SCB_CPUID		(*(reg_t *)SCB_BASE)
#define SCB_ICSR		(*(reg_t *)(SCB_BASE + 4))
#define SCB_VTOR		(*(reg_t *)(SCB_BASE + 8))
#define SCB_AIRCR		(*(reg_t *)(SCB_BASE + 0xc))
#define SCB_SCR 		(*(reg_t *)(SCB_BASE + 0x10))
#define SCB_CCR 		(*(reg_t *)(SCB_BASE + 0x14))
#define SCB_SHPR1		(*(reg_t *)(SCB_BASE + 0x18))
#define SCB_SHPR2		(*(reg_t *)(SCB_BASE + 0x1c))
#define SCB_SHPR3		(*(reg_t *)(SCB_BASE + 0x20))
#define SCB_SHCSR		(*(reg_t *)(SCB_BASE + 0x24))
#define SCB_CFSR		(*(reg_t *)(SCB_BASE + 0x28))
#define SCB_HFSR		(*(reg_t *)(SCB_BASE + 0x2c))
#define SCB_MMFAR		(*(reg_t *)(SCB_BASE + 0x34))
#define SCB_BFAR		(*(reg_t *)(SCB_BASE + 0x38))

#define SCB_ACTLR		(*(reg_t *)0xE000E008)

/* Systick */
#define SYSTICK_BASE		(0xe000e010)
#define STK_CTRL		(*(reg_t *)SYSTICK_BASE)
#define STK_LOAD		(*(reg_t *)(SYSTICK_BASE + 4))
#define STK_VAL 		(*(reg_t *)(SYSTICK_BASE + 8))
#define STK_CALIB		(*(reg_t *)(SYSTICK_BASE + 0xc))

#endif /* __ARMv7M_REGS_H__ */
