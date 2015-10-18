#include <foundation.h>
#include "clock.h"

#ifndef stm32f1
#define stm32f1	1
#define stm32f4	2
#endif

#define PLLRDY			25
#define PLLON			24
#define CSSON			19
#define HSEON			16
#define HSERDY			17

#define SW			0
#define SWS			2
#define HPRE			4
#if (SOC == stm32f1)
#define PLLMUL			18
#define PLLSRC			16
#define ADCPRE			14
#define PPRE2			11
#define PPRE1			8
#elif (SOC == stm32f4)
#define RTCPRE			16
#define PPRE2			13
#define PPRE1			10

#define PLLQ			24
#define PLLSRC			22
#define PLLP			16
#define PLLN			6
#define PLLM			0

#define MHZ			1000000
#endif

/* SYSCLK */
unsigned int get_pllclk()
{
	unsigned int clk, pllm;
#if (SOC == stm32f4)
	unsigned int plln, pllp;
#endif

	switch ((RCC_CFGR >> SWS) & 0x3) {
	case 0x00 :
		clk = HSI;
		break;
	case 0x01 :
		clk = HSE;
		break;
	case 0x02 :
#if (SOC == stm32f1)
		pllm = ((RCC_CFGR >> PLLMUL) & 0xf) + 2;
		if ((RCC_CFGR >> PLLSRC) & 1) { /* HSE selected */
			if (RCC_CFGR & 0x20000) /* mask PLLXTPRE[17] */
				clk = (HSE >> 1) * pllm;
			else
				clk = HSE * pllm;
		} else {
			clk = (HSI >> 1) * pllm;
		}
#elif (SOC == stm32f4)
		pllm = ((RCC_PLLCFGR >> PLLM) & 0x3f) * MHZ;
		plln = (RCC_PLLCFGR >> PLLN) & 0x1ff;
		pllp = (RCC_PLLCFGR >> PLLP) & 3;
		switch (pllp) {
		case 0: pllp = 2;
			break;
		case 1: pllp = 4;
			break;
		case 2: pllp = 6;
			break;
		case 3: pllp = 8;
			break;
		}

		if ((RCC_PLLCFGR >> PLLSRC) & 1) /* HSE selected */
			clk = HSE / pllm;
		else
			clk = HSI / pllm;

		clk = clk * plln / pllp * MHZ;
#endif
		break;
	default   :
		clk = HSI;
		break;
	}

	return clk;
}

/* AHB */
unsigned int get_hclk()
{
	unsigned int clk, pre, pllclk;

	pllclk = get_pllclk();
#if (SOC == stm32f1)
	pre    = (RCC_CFGR >> HPRE) & 0xf; /* mask HPRE[7:4] */
	pre    = pre? pre - 7 : 0;         /* get prescaler division factor */
#elif (SOC == stm32f4)
	pre    = ((RCC_CFGR >> HPRE) & 0x8)? ((RCC_CFGR >> HPRE) & 0x7) + 1 : 0;
#endif
	clk    = pllclk >> pre;

	return clk;
}

/* APB1 */
unsigned int get_pclk1()
{
	unsigned int clk, pre, hclk;

	hclk = get_hclk();
#if (SOC == stm32f1)
	pre  = (RCC_CFGR >> PPRE1) & 0x7; /* mask PPRE1[10:8] */
	pre  = pre? pre - 3 : 0;
#elif (SOC == stm32f4)
	pre  = ((RCC_CFGR >> PPRE1) & 0x4)? ((RCC_CFGR >> PPRE1) & 0x3) + 1 : 0;
#endif
	clk  = hclk >> pre;

	return clk;
}

/* APB2 */
unsigned int get_pclk2()
{
	unsigned int clk, pre, hclk;

	hclk = get_hclk();
#if (SOC == stm32f1)
	pre  = (RCC_CFGR >> PPRE2) & 0x7; /* mask PPRE2[13:11] */
	pre  = pre? pre - 3 : 0;
#elif (SOC == stm32f4)
	pre  = ((RCC_CFGR >> PPRE2) & 0x4)? ((RCC_CFGR >> PPRE2) & 0x3) + 1 : 0;
#endif
	clk  = hclk >> pre;

	return clk;
}

static unsigned int get_adclk(unsigned int pclk2)
{
#if (SOC == stm32f1)
	unsigned int clk, pre;

	pre = (RCC_CFGR >> ADCPRE) & 0x3; /* mask PPRE2[15:14] */
	pre = (pre + 1) << 1;             /* get prescaler division factor */
	clk = pclk2 / pre;

	return clk;
#elif (SOC == stm32f4)
	return 0;
#endif
}

unsigned int get_stkclk()
{
	unsigned int clk, hclk;

	hclk = get_hclk();

	if (STK_CTRL & 4)
		clk = hclk;
	else
		clk = hclk >> 3;

	return clk;
}

unsigned int get_sysclk_freq()
{
	return get_stkclk();
}

#include <kernel/init.h>

void clock_init()
{
#if (SOC == stm32f1)	/* 72MHz */
	/* flash access time adjustment */
	FLASH_ACR |= 2; /* two wait states for flash access */
	/* prefetch buffer gets activated from reset. disable to avoid extra
	 * flash access that consumes 20mA for 128-bit line fetching */
	while ((FLASH_ACR & 7) != 2);
#elif (SOC == stm32f4)	/* 168MHz */
	FLASH_ACR |= 5; /* five wait states */
	/* enable data cache, instruction cache and prefetch */
	FLASH_ACR |= 0x700;
	while ((FLASH_ACR & 7) != 5);
#endif

	/* 1. Turn on HSE oscillator. */
	BITBAND(&RCC_CR, HSEON, ON);

	/* 2. Wait for HSE to be stable. */
	while (!gbi(RCC_CR, HSERDY));

	/* 3. Set PLL multification factor, and PLL source clock. */
	/* 4. Set prescalers' factors. */
#if (SOC == stm32f1)
	/* APB1 <= 36MHz <= APB2 <= 72MHz <= AHB <= 72MHz
	 * ADC <= 14MHz
	 * USB = 48MHz */
	RCC_CFGR = (7 << PLLMUL) | (4 << PPRE1) | (2 << ADCPRE) | (1 << PLLSRC);
#elif (SOC == stm32f4)
	/* APB1 <= 42MHz <= APB2 <= 84MHz <= AHB <= 168MHz */
	RCC_CFGR = (8 << RTCPRE) | (4 << PPRE2) | (5 << PPRE1);
	RCC_PLLCFGR = (7 << PLLQ) | (1 << PLLSRC) | (336 << PLLN) | (8 << PLLM);
#endif

	/* 5. Turn on PLL. */
	BITBAND(&RCC_CR, PLLON, ON);

	/* 6. Wait for PLL to be stable. */
	while (!gbi(RCC_CR, PLLRDY));

	/* 7. Select PLL as system clock. */
	RCC_CFGR |= 2 << SW;

	/* 8. Check if its change is done. */
	while (((RCC_CFGR >> SWS) & 3) != 2);

#if (SOC == stm32f4)
	/* 9. Turn off HSI */
	RCC_CR &= ~1;
#endif
	//BITBAND(&RCC_CR, CSSON, ON);
}
REGISTER_INIT(clock_init, 0);
