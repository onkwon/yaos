#ifndef __STM32_CLOCK_H__
#define __STM32_CLOCK_H__

#define HSE			8000000 /* 8MHz */
#define HSI			8000000	/* 8MHz */

#include <io.h>

/* Systick */
#define SYSTICK_INT		2
#define SYSTICK_FLAG()		(STK_CTRL & 0x10000)
#define SYSTICK_MAX		((1 << 24) - 1) /* 24-bit timer */

#define reset_sysclk()		(STK_VAL = 0)
#define set_sysclk(v)		(STK_LOAD = v)

#define stop_sysclk()		(STK_CTRL &= ~3)
#define run_sysclk()		(STK_CTRL = (STK_CTRL & ~3) | SYSTICK_INT | 1)

#define get_sysclk()		(STK_VAL)
#define get_sysclk_max()	(STK_LOAD + 1)
unsigned int getclk();
unsigned int get_sysclk_freq();

int sysclk_init();
void clock_init();

#endif /* __STM32_CLOCK_H__ */
