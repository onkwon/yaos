#ifndef __STM32_CLOCK_H__
#define __STM32_CLOCK_H__

#define HSE		8000000 /* 8MHz */
#define HSI		8000000	/* 8MHz */

#include <io.h>

unsigned int get_sysclk();
unsigned int get_hclk  (unsigned int sysclk);
unsigned int get_pclk1 (unsigned int hclk);
unsigned int get_pclk2 (unsigned int hclk);
unsigned int get_adclk (unsigned int pclk2);
unsigned int get_stkclk(unsigned int hclk);
unsigned int get_sysclk_hz();

void clock_init();

/* Systick */
#define SYSTIMER(on)	/* ON or OFF [| SYSTIMER_INT] */ \
		STK_CTRL = (STK_CTRL & ~3) | (on)
#define SYSTIMER_FLAG()		(STK_CTRL & 0x10000)
#define SET_SYSTIMER(v)		(STK_LOAD = v)
#define GET_SYSTIMER()		(STK_VAL)
#define RESET_SYSTIMER()	(STK_VAL  = 0)
#define SYSTIMER_INT		2
#define SYSTIMER_MAX		((1 << 24) - 1) /* 24-bit timer */

#define get_systick()		GET_SYSTIMER()
#define get_systick_max()	(STK_LOAD + 1)
unsigned int get_sysclk_hz();

#define reset_sysclk()		RESET_SYSTIMER()
#define set_sysclk(v)		SET_SYSTIMER(v)

void sysclk_init();

#define stop_sysclk()		SYSTIMER(OFF)
#define run_sysclk()		SYSTIMER(ON | SYSTIMER_INT)

#endif /* __STM32_CLOCK_H__ */
