#ifndef __YAOS_STM32_RCC_H__
#define __YAOS_STM32_RCC_H__

#include "arch/mach/board/hw.h"
#include "types.h"
#include <stdbool.h>

#ifndef HSE
#define HSE			8000000UL	/* 8MHz */
#endif

#ifndef HSI
  #if defined(stm32f1) || defined(stm32f3)
  #define HSI			8000000UL	/* 8MHz */
  #elif defined(stm32f4)
  #define HSI			16000000UL
  #else
  #error undefined machine
  #endif
#endif

unsigned long get_pllclk(void);
unsigned long get_sysclk_freq(void);
unsigned long get_stkclk(void);
unsigned long get_pclk2(void);
unsigned long get_pclk1(void);
unsigned long get_hclk(void);
unsigned long get_adclk(void);

void __turn_apb1_clock(const unsigned long bit, const bool on);
void __turn_apb2_clock(const unsigned long bit, const bool on);
void __turn_ahb1_clock(const unsigned long bit, const bool on);
void __turn_port_clock(const reg_t * const port, const bool on);
unsigned long __read_apb1_clock(void);
unsigned long __read_apb2_clock(void);
unsigned long __read_ahb1_clock(void);
void __reset_apb1_device(const unsigned long bit);
void __reset_apb2_device(const unsigned long bit);

void clock_init(void);

#endif /* __YAOS_STM32_RCC_H__ */
