#ifndef __STM32_RCC_H__
#define __STM32_RCC_H__

#include <asm/hw.h>

#ifndef HSE
#define HSE			8000000 /* 8MHz */
#endif

#ifndef HSI
  #if defined(stm32f1) || defined(stm32f3)
  #define HSI			8000000	/* 8MHz */
  #elif defined(stm32f4)
  #define HSI			16000000
  #else
  #error undefined machine
  #endif
#endif

unsigned int get_pllclk();
unsigned int get_sysclk_freq();
unsigned int get_stkclk();
unsigned int get_pclk2();
unsigned int get_pclk1();
unsigned int get_hclk();
unsigned int get_adclk();

void __turn_apb1_clock(unsigned int nbit, bool on);
void __turn_apb2_clock(unsigned int nbit, bool on);
void __turn_ahb1_clock(unsigned int nbit, bool on);
void __turn_port_clock(reg_t *port, bool on);
unsigned int __read_apb1_clock();
unsigned int __read_apb2_clock();
unsigned int __read_ahb1_clock();
void __reset_apb1_device(unsigned int nbit);
void __reset_apb2_device(unsigned int nbit);

void clock_init();

#endif /* __STM32_RCC_H__ */
