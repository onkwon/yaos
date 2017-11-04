#ifndef __RCC_H__
#define __RCC_H__

#define M_MIN		2
#define M_MAX		63
#define P_MIN		2
#define P_MAX		8
#define P_STEP		2
#define N_MIN		2
#define N_MAX		432
#define Q_MIN		2
#define Q_MAX		15

#define VCO_MIN_MHZ	100
#define VCO_MAX_MHZ	432
#define PLLIN_MIN_KHZ	950
#define PLLIN_MAX_KHZ	2100
#define APB_MAX_MHZ	90
#define LAPB_MAX_MHZ	45

#define PLL48CLK_MHZ	48

struct pll_t {
	unsigned int vco;
	unsigned int m, n, p, q;
};

enum clock_control {
	HSI_ON		= 0,
	HSI_RDY		= 1,
	HSE_ON		= 16,
	HSE_RDY		= 17,
	HSE_BYP		= 18,
	CSS_ON		= 19,
	PLL_ON		= 24,
	PLL_RDY		= 25,
	PLLI2S_ON	= 26,
	PLLI2S_RDY	= 27,
	PLLSAI_ON	= 28,
	PLLSAI_RDY	= 29,
};

enum clock_configuration {
	SW		= 0,
	SWS		= 2,
	AHB_PRE		= 4,
	LAPB_PRE	= 10,
	APB_PRE		= 13,
	RTC_PRE		= 16,
	MCO1		= 21,
	I2S_SRC		= 23,
	MCO1_PRE	= 24,
	MCO2_PRE	= 27,
	MCO2		= 30,
};

enum pll_configuration {
	PLL_M		= 0,
	PLL_N		= 6,
	PLL_P		= 16,
	PLL_SRC		= 22,
	PLL_Q		= 24,
};

#endif /* __RCC_H__ */
