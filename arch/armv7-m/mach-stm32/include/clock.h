#ifndef __STM32_RCC_H__
#define __STM32_RCC_H__

#define HSE			8000000 /* 8MHz */
#define HSI			8000000	/* 8MHz */

unsigned int getclk();
unsigned int get_sysclk_freq();
void clock_init();

#endif /* __STM32_RCC_H__ */
