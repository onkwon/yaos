#ifndef __STM32_IO_H__
#define __STM32_IO_H__

/* RCC */
#define SET_CLOCK_AHB1(bit, on)		BITBAND(&RCC_AHB1ENR, bit, on)
#define SET_CLOCK_APB2(bit, on)		BITBAND(&RCC_APB2ENR, bit, on)
#define SET_CLOCK_APB1(bit, on)		BITBAND(&RCC_APB1ENR, bit, on)
#define RESET_PERI_APB2(bit) { \
	BITBAND(&RCC_APB2RSTR, bit, ON); \
	BITBAND(&RCC_APB2RSTR, bit, OFF); \
}
#define RESET_PERI_APB1(bit) { \
	BITBAND(&RCC_APB1RSTR, bit, ON); \
	BITBAND(&RCC_APB1RSTR, bit, OFF); \
}

#include <asm/mach/reg.h>

#ifndef stm32f1
#define stm32f1	1
#define stm32f3	3
#define stm32f4	4
#endif

#if (SOC == stm32f4)
#define SET_CLOCK_AHB3(bit, on)		BITBAND(&RCC_AHB3ENR, bit, on)
#endif

#endif /* __STM32_IO_H__ */
