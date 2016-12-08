#ifndef __STM32_RCC_H__
#define __STM32_RCC_H__

#ifndef stm32f1
#define stm32f1	1
#define stm32f4	2
#endif

#define HSE			8000000 /* 8MHz */
#if (SOC == stm32f1 || SOC == stm32f3)
#define HSI			8000000	/* 8MHz */
#elif (SOC == stm32f4)
#define HSI			16000000
#endif

unsigned int get_pllclk();
unsigned int get_sysclk_freq();
unsigned int get_stkclk();
unsigned int get_pclk2();
unsigned int get_pclk1();
unsigned int get_hclk();
void clock_init();

#endif /* __STM32_RCC_H__ */
