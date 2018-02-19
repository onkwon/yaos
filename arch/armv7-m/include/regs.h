#ifndef __ARMv7M_REGS_H__
#define __ARMv7M_REGS_H__

/* System Control Block(SCB), Cortex-M3's internal peripherals */
#define SCB_BASE		(0xE000E000)
#define SCB_ACTLR		(*(reg_t *)(SCB_BASE + 0x8))
#define SCB_CPUID		(*(reg_t *)(SCB_BASE + 0xD00))
#define SCB_ICSR		(*(reg_t *)(SCB_BASE + 0xD04))
#define SCB_VTOR		(*(reg_t *)(SCB_BASE + 0xD08))
#define SCB_AIRCR		(*(reg_t *)(SCB_BASE + 0xD0C))
#define SCB_SCR 		(*(reg_t *)(SCB_BASE + 0xD10))
#define SCB_CCR 		(*(reg_t *)(SCB_BASE + 0xD14))
#define SCB_SHPR		(SCB_BASE + 0xD18)
#define SCB_SHPR1		(*(reg_t *)(SCB_BASE + 0xD18))
#define SCB_SHPR2		(*(reg_t *)(SCB_BASE + 0xD1C))
#define SCB_SHPR3		(*(reg_t *)(SCB_BASE + 0xD20))
#define SCB_SHCSR		(*(reg_t *)(SCB_BASE + 0xD24))
#define SCB_CFSR		(*(reg_t *)(SCB_BASE + 0xD28))
#define SCB_HFSR		(*(reg_t *)(SCB_BASE + 0xD2C))
#define SCB_MMFAR		(*(reg_t *)(SCB_BASE + 0xD34))
#define SCB_BFAR		(*(reg_t *)(SCB_BASE + 0xD38))
#define SCB_AFRR		(*(reg_t *)(SCB_BASE + 0xD3C))
#define SCB_CPACR		(*(reg_t *)(SCB_BASE + 0xD88))
#define SCB_STIR		(*(reg_t *)(SCB_BASE + 0xF00))

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

/* Systick */
#define SYSTICK_BASE		(0xE000E010)
#define STK_CTRL		(*(reg_t *)SYSTICK_BASE)
#define STK_LOAD		(*(reg_t *)(SYSTICK_BASE + 4))
#define STK_VAL 		(*(reg_t *)(SYSTICK_BASE + 8))
#define STK_CALIB		(*(reg_t *)(SYSTICK_BASE + 0xc))

/* FPU */
#define FPCCR			(*(reg_t *)0xE000EF34)
#define FPCAR			(*(reg_t *)0xE000EF38)
#define FPDSCR			(*(reg_t *)0xE000EF3C)
#define MVFR0			(*(reg_t *)0xE000EF40)
#define MVFR1			(*(reg_t *)0xE000EF44)

#endif /* __ARMv7M_REGS_H__ */
