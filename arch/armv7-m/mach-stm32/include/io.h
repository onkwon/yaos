#ifndef __STM32_IO_H__
#define __STM32_IO_H__

/* RCC */
#define SET_CLOCK_AHB1(on, pin)		BITBAND(&RCC_AHB1ENR, pin, on)
#define SET_CLOCK_APB2(on, pin)		BITBAND(&RCC_APB2ENR, pin, on)
#define SET_CLOCK_APB1(on, pin)		BITBAND(&RCC_APB1ENR, pin, on)
#define RESET_PERI_APB2(pin) { \
	BITBAND(&RCC_APB2RSTR, pin, ON); \
	BITBAND(&RCC_APB2RSTR, pin, OFF); \
}
#define RESET_PERI_APB1(pin) { \
	BITBAND(&RCC_APB1RSTR, pin, ON); \
	BITBAND(&RCC_APB1RSTR, pin, OFF); \
}

#undef  INCPATH
#define INCPATH			<asm/mach-MACHINE/SOC.h>
#include INCPATH

#ifndef stm32f1
#define stm32f1	1
#define stm32f3	3
#define stm32f4	4
#endif

#endif /* __STM32_IO_H__ */
