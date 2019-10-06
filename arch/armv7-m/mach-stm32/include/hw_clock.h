#ifndef __YAOS_HW_CLOCK_H__
#define __YAOS_HW_CLOCK_H__

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
 #error "undefined machine"
 #endif
#endif

unsigned long hw_clock_get_pll(void);
unsigned long hw_clock_get_stk(void);
unsigned long hw_clock_get_pclk2(void);
unsigned long hw_clock_get_pclk1(void);
unsigned long hw_clock_get_hclk(void);
unsigned long hw_clock_get_adc(void);
unsigned long hw_clock_get_wdt(void);
unsigned long hw_clock_get_rtc(void);

void hw_clock_set_apb1(const unsigned long bit, const bool on);
void hw_clock_set_apb2(const unsigned long bit, const bool on);
void hw_clock_set_ahb1(const unsigned long bit, const bool on);
void hw_clock_set_port(const reg_t * const port, const bool on);
unsigned long hw_clock_get_apb1(void);
unsigned long hw_clock_get_apb2(void);
unsigned long hw_clock_get_ahb1(void);
void hw_clock_reset_apb1_conf(const unsigned long bit);
void hw_clock_reset_apb2_conf(const unsigned long bit);

void clock_init(void);

#endif /* __YAOS_HW_CLOCK_H__ */
