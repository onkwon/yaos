#ifndef __YAOS_ARMv7M_REGS_H__
#define __YAOS_ARMv7M_REGS_H__

#include "types.h"

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

/* DEBUG
 * Instrumentation Trace Macrocell (ITM)	0xE0000000-0xE0000FFF
 * Data Watchpoint and Trace (DWT) unit		0xE0001000-0xE0001FFF
 * Flash Patch and Breakpoint (FPB) unit	0xE0002000-0xE0002FFF
 * System Control Space (SCS)			0xE000ED00-0xE000EFFF
 *   System Control Block (SCB)			0xE000ED00-0xE000ED8F
 *   Debug Control Block (DCB)			0xE000EDF0-0xE000EEFF
 * Trace Port Interface Unit (TPIU)		0xE0040000-0xE0040FFF
 * Embedded Trace Macrocell (ETM)		0xE0041000-0xE0041FFF
 * IMPLEMENTATION DEFINED			0xE0042000-0xE00FEFFF
 * ROM table					0xE00FF000-0xE00FFFFF
 * */
#define DEBUG_BASE		(0xE000EDF0UL)
#define ITM_BASE		(0xE0000000UL)

#define DFSR			(*(reg_t *)0xE000ED30UL)

#define TPIU_ACPR		(*(reg_t *)0xE0040010UL)
#define TPIU_SPPR		(*(reg_t *)0xE00400F0UL)
#define TPIU_FFCR		(*(reg_t *)0xE0040304UL)

#define DWT_CTRL		(*(reg_t *)0xE0001000UL)

struct COREDEBUG {
	reg_t DHCSR;
	reg_t DCRSR;
	reg_t DCRDR;
	reg_t DEMCR;
};

struct ITM {
	union {
		volatile uint8_t u8;
		volatile uint16_t u16;
		volatile uint32_t u32;
	} PORT[32U];
	reg_t reserved0[864U];
	reg_t TER;
	reg_t reserved1[15U];
	reg_t TPR;
	reg_t reserved2[15U];
	reg_t TCR;
	reg_t reserved3[29U];
	reg_t IWR;
	reg_t IRR;
	reg_t IMCR;
	reg_t reserved4[43U];
	reg_t LAR;
	reg_t LSR;
	reg_t reserved5[6U];
	reg_t PID4;
	reg_t PID5;
	reg_t PID6;
	reg_t PID7;
	reg_t PID0;
	reg_t PID1;
	reg_t PID2;
	reg_t PID3;
	reg_t CID0;
	reg_t CID1;
	reg_t CID2;
	reg_t CID3;
};

#define COREDEBUG		((struct COREDEBUG *)DEBUG_BASE)
#define ITM			((struct ITM *)ITM_BASE)

#endif /* __YAOS_ARMv7M_REGS_H__ */
