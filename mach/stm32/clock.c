#include <clock.h>
#include <foundation.h>

#define PLLRDY			25
#define PLLON			24
#define CSSON			19
#define HSEON			16
#define HSERDY			17

#define PLLMUL			18
#define PLLSRC			16
#define ADCPRE			14
#define PPRE2			11
#define PPRE1			8
#define HPRE			4
#define SWS			2
#define SW			0

unsigned int get_sysclk()
{
	unsigned int clk, pllmul;

	switch ((RCC_CFGR >> SWS) & 0x3) {
	case 0x00 :
		clk = HSI;
		break;
	case 0x01 :
		clk = HSE;
		break;
	case 0x02 :
		pllmul = ((RCC_CFGR >> PLLMUL) & 0xf) + 2;
		if ((RCC_CFGR >> PLLSRC) & 1) { /* HSE selected */
			if (RCC_CFGR & 0x20000) /* mask PLLXTPRE[17] */
				clk = (HSE >> 1) * pllmul;
			else
				clk = HSE * pllmul;
		} else {
			clk = (HSI >> 1) * pllmul;
		}
		break;
	default   :
		clk = HSI;
		break;
	}

	return clk;
}

unsigned int get_hclk(unsigned int sysclk)
{
	unsigned int clk, pre;

	pre = (RCC_CFGR >> HPRE) & 0xf; /* mask HPRE[7:4] */
	pre = pre? pre - 7 : 0;         /* get prescaler division factor */
	clk = sysclk >> pre;

	return clk;
}

unsigned int get_pclk1(unsigned int hclk)
{
	unsigned int clk, pre;

	pre = (RCC_CFGR >> PPRE1) & 0x7; /* mask PPRE1[10:8] */
	pre = pre? pre - 3 : 0;
	clk = hclk >> pre;

	return clk;
}

unsigned int get_pclk2(unsigned int hclk)
{
	unsigned int clk, pre;

	pre = (RCC_CFGR >> PPRE2) & 0x7; /* mask PPRE2[13:11] */
	pre = pre? pre - 3 : 0;
	clk = hclk >> pre;

	return clk;
}

unsigned int get_adclk(unsigned int pclk2)
{
	unsigned int clk, pre;

	pre = (RCC_CFGR >> ADCPRE) & 0x3; /* mask PPRE2[15:14] */
	pre = (pre + 1) << 1;             /* get prescaler division factor */
	clk = pclk2 / pre;

	return clk;
}

unsigned int get_stkclk(unsigned int hclk)
{
	unsigned int clk;

	if (STK_CTRL & 4)
		clk = hclk;
	else
		clk = hclk >> 3;

	return clk;
}

#include <kernel/init.h>

void clock_init()
{
	/* flash access time adjustment */
	FLASH_ACR |= 2; /* two wait states for flash access */

	/* 1. Turn on HSE oscillator. */
	BITBAND(&RCC_CR, HSEON, ON);

	/* 2. Wait for HSE to be stable. */
	while (!gbi(RCC_CR, HSERDY));

	/* 3. Set PLL multification factor, and PLL source clock. */
	/* 4. Set prescalers' factors. */
	RCC_CFGR = (7 << PLLMUL) | (4 << PPRE1) | (2 << ADCPRE) | (1 << PLLSRC);

	/* 5. Turn on PLL. */
	BITBAND(&RCC_CR, PLLON, ON);

	/* 6. Wait for PLL to be stable. */
	while (!gbi(RCC_CR, PLLRDY));

	/* 7. Select PLL as system clock. */
	RCC_CFGR |= 2 << SW;

	/* 8. Check if its change is done. */
	while (((RCC_CFGR >> SWS) & 3) != 2);

	//BITBAND(&RCC_CR, CSSON, ON);
}
REGISTER_INIT(clock_init, 0);
