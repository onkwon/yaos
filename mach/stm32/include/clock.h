#ifndef __STM32_CLOCK_H__
#define __STM32_CLOCK_H__

#define HSE		8000000 /* 8MHz */
#define HSI		8000000	/* 8MHz */

#include <io.h>

unsigned get_sysclk();
unsigned get_hclk  (unsigned sysclk);
unsigned get_pclk1 (unsigned hclk);
unsigned get_pclk2 (unsigned hclk);
unsigned get_adclk (unsigned pclk2);
unsigned get_stkclk(unsigned hclk);

/* Systick */
#define SYSTIMER(on)	/* ON or OFF [| SYSTIMER_INT] */ \
		STK_CTRL = (STK_CTRL & ~3) | (on)
#define SYSTIMER_FLAG()		(STK_CTRL & 0x10000)
#define SET_SYSTIMER(v)		(STK_LOAD = v)
#define GET_SYSTIMER()		(STK_VAL)
#define RESET_SYSTIMER()	(STK_VAL  = 0)
#define SYSTIMER_INT		2
#define SYSTIMER_MAX		((1 << 24) - 1) /* 24-bit timer */

void systick_init();

#define systick_off()		SYSTIMER(OFF)
#define systick_on()		SYSTIMER(ON | SYSTIMER_INT)

#endif /* __STM32_CLOCK_H__ */
