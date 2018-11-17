#ifndef __YAOS_STM32_RCC_H__
#define __YAOS_STM32_RCC_H__

#include "arch/mach/board/hw.h"
#include "types.h"
#include <stdbool.h>

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

unsigned int get_pllclk(void);
unsigned int get_sysclk_freq(void);
unsigned int get_stkclk(void);
unsigned int get_pclk2(void);
unsigned int get_pclk1(void);
unsigned int get_hclk(void);
unsigned int get_adclk(void);

void __turn_apb1_clock(const unsigned int bit, const bool on);
void __turn_apb2_clock(const unsigned int bit, const bool on);
void __turn_ahb1_clock(const unsigned int bit, const bool on);
void __turn_port_clock(const reg_t * const port, const bool on);
unsigned int __read_apb1_clock(void);
unsigned int __read_apb2_clock(void);
unsigned int __read_ahb1_clock(void);
void __reset_apb1_device(const unsigned int bit);
void __reset_apb2_device(const unsigned int bit);

void clock_init(void);

#endif /* __YAOS_STM32_RCC_H__ */
