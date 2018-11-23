#ifndef __YAOS_STM32_IO_H__
#define __YAOS_STM32_IO_H__

/* RCC */
#define SET_CLOCK_AHB1(bit, on)		BITBAND(&RCC_AHB1ENR, bit, on)
#define SET_CLOCK_APB2(bit, on)		BITBAND(&RCC_APB2ENR, bit, on)
#define SET_CLOCK_APB1(bit, on)		BITBAND(&RCC_APB1ENR, bit, on)
#define RESET_PERI_APB2(bit) { \
	BITBAND(&RCC_APB2RSTR, bit, 1); \
	BITBAND(&RCC_APB2RSTR, bit, 0); \
}
#define RESET_PERI_APB1(bit) { \
	BITBAND(&RCC_APB1RSTR, bit, 1); \
	BITBAND(&RCC_APB1RSTR, bit, 0); \
}

#ifdef RCC_AHB3ENR
#define SET_CLOCK_AHB3(bit, on)		BITBAND(&RCC_AHB3ENR, bit, on)
#endif

#endif /* __YAOS_STM32_IO_H__ */
