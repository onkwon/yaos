#include <power.h>
#include <foundation.h>

#define PWRCLK_ENABLE()		{ \
	SET_CLOCK_APB1(ENABLE, 28); /* PWR */ \
	SET_CLOCK_APB1(ENABLE, 27); /* BKP */ \
}
#define PWRCLK_DISABLE()		{ \
	SET_CLOCK_APB1(DISABLE, 28); /* PWR */ \
	SET_CLOCK_APB1(DISABLE, 27); /* BKP */ \
}

void __enter_sleep_mode()
{
	PWRCLK_ENABLE();
	dsb();
	__wfi();
	PWRCLK_DISABLE();
}

/* all clocks in the 1.8 V domain are stopped,
 * the PLL, the HSI and the HSE RC oscillators are disabled.
 * SRAM and register contents are preserved.
 * all I/O pins keep the same state as in the Run mode.
 * ADC, DAC, WDG, RTC, LSI_RC, and LSE_OSC can consume power. */
void __enter_stop_mode()
{
	unsigned int irqflag;

	irq_save(irqflag);
	local_irq_disable();

	PWRCLK_ENABLE();
	SCB_SCR |= 4; /* Set SLEEPDEEP bit */
	/* Clear PDDS bit in Power Control register (PWR_CR) */
	PWR_CR |= 1; /* configure LPDS bit in PWR_CR */

	stop_scheduler();
	dsb();
	__wfi();
	clock_init();
	run_scheduler();

	SCB_SCR &= ~4;
	PWR_CR &= ~1;
	PWRCLK_DISABLE();

	irq_restore(irqflag);

	/* wakeup latency:
	 * HSI RC wakeup time + regulator wakeup time from Low-power mode */

	/* Note from reference manual:
	 * If the application needs to disable the external clock before
	 * entering Stop mode, the HSEON bit must first be disabled and the
	 * system clock switched to HSI. Otherwise, if the HSEON bit remains
	 * enabled and the external clock (external oscillator) is removed when
	 * entering Stop mode, the clock security system (CSS) feature must be
	 * enabled to detect any external oscillator failure and avoid a
	 * malfunction behavior when entering stop mode. */
	//BITBAND(&RCC_CR, CSSON, ON);
}

/* the voltage regulator disabled.
 * The 1.8 V domain is consequently powered off.
 * The PLL, the HSI oscillator and the HSE oscillator are also switched off.
 * SRAM and register contents are lost
 * except for registersin the Backup domain and Standby circuitry */
void __enter_standby_mode()
{
	SCB_SCR |= 4; /* Set SLEEPDEEP bit */
	PWR_CR |= 2; /* Set PDDS bit in Power Control register (PWR_CR) */
	PWR_CR |= 4; /* Clear WUF bit in Power Control register (PWR_CSR) */

	__wfi();

	/* wakeup latency: reset phase */
}

void __sleep_on_exit()
{
	SCB_SCR |= 2;
}

#ifdef CONFIG_DEBUG
static void disp_clkinfo()
{
	/*
	char *clkname_ahb[32]  = {
		"DMA1" , "DMA2"  , "SRAM"  , NULL   , "FLITF", NULL    , NULL    , NULL   ,
		"FSMC" , NULL    , "SDIO"  , NULL   , NULL   , NULL    , NULL    , NULL   ,
		NULL   , NULL    , NULL    , NULL   , NULL   , NULL    , NULL    , NULL   ,
		NULL   , NULL    , NULL    , NULL   , NULL   , NULL    , NULL    , NULL   };
	char *clkname_apb2[32] = {
		"AFIO" , NULL    , "IOPA"  , "IOPB" , "IOPC" , "IOPD"  , "IOPE"  , "IOPF" ,
		"IOPG" , "ADC1"  , "ADC2"  , "TIM1" , "SPI1" , "TIM8"  , "USART1", "ADC3" ,
		NULL   , NULL    , NULL    , "TIM11", "TIM10", "TIM9"  , NULL    , NULL   ,
		NULL   , NULL    , NULL    , NULL   , NULL   , NULL    , NULL    , NULL   };
	char *clkname_apb1[32] = {
		"TIM2" , "TIM3"  , "TIM4"  , "TIM5" , "TIM6" , "TIM7"  , "TIM12" , "TIM13",
		"TIM14", NULL    , NULL    , "WWD"  , NULL   , NULL    , "SPI2"  , "SPI3 ",
		NULL   , "USART2", "USART3", "UART4", "UART5", "I2C1"  , "I2C2"  , "USB"  ,
		NULL   , "CAN"   , NULL    , "BKP"  , "PWR"  , "DAC"   , NULL    , NULL   };
	char **desc[3] = { clkname_ahb, clkname_apb2, clkname_apb1 };
	unsigned int regs[3] = { RCC_AHBENR, RCC_APB2ENR, RCC_APB1ENR };
	unsigned int i, j;
	*/

	unsigned int sysclk, hclk, pclk1, pclk2, adclk;

	/*
	sysclk = getclk();
	hclk   = get_hclk  (sysclk);
	pclk1  = get_pclk1 (hclk);
	pclk2  = get_pclk2 (hclk);
	adclk  = get_adclk (pclk2);

	printf("System clock frequency\t %d\n"
		"(hclk   %d, pclk1  %d, pclk2  %d, adclk  %d)\n\n",
		sysclk, hclk, pclk1, pclk2, adclk);
	*/
	printf("Enabled peripheral clock:\n");
	printf("AHB  %08x\n", RCC_AHBENR);
	printf("APB2 %08x\n", RCC_APB2ENR);
	printf("APB1 %08x\n", RCC_APB1ENR);
	/*
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 32; j++)  {
			if (regs[i] & (1 << j)) printf(" %s", desc[i][j]);
		}
	}
	*/
	printf("\n");

	printf("PORTA %08x %08x\n", *(volatile unsigned int *)PORTA, *(volatile unsigned int *)(PORTA+4));
	printf("PORTB %08x %08x\n", *(volatile unsigned int *)PORTB, *(volatile unsigned int *)(PORTB+4));
	printf("PORTC %08x %08x\n", *(volatile unsigned int *)PORTC, *(volatile unsigned int *)(PORTC+4));
	printf("PORTD %08x %08x\n", *(volatile unsigned int *)PORTD, *(volatile unsigned int *)(PORTD+4));
	printf("PORTE %08x %08x\n", *(volatile unsigned int *)PORTE, *(volatile unsigned int *)(PORTE+4));
	printf("PORTF %08x %08x\n", *(volatile unsigned int *)PORTF, *(volatile unsigned int *)(PORTF+4));
	printf("PORTG %08x %08x\n", *(volatile unsigned int *)PORTG, *(volatile unsigned int *)(PORTG+4));
	printf("\n");
}

#include <kernel/timer.h>

#define TEMP_V25	1775	/* 1.43V */
#define TEMP_SLOPE	43
#define TEMP_OFFSET	25
#define AD2VOL(ad)	(ad * 8 / 10000)
#define AD2VOL_FAC(ad)	(ad * 8 / 100 % 100)

/* to get internal reference voltage and temperature */
static int getadc(int ch)
{
	int v;

	if (!(RCC_APB2ENR & (1 << 9))) {
		SET_CLOCK_APB2(ENABLE, 9);	/* ADC1 clock enable */
		ADC1_CR2   = 1 | 0x800000;	/* ADC1 and TSVREFE power on */
		udelay(1000);			/* delay */
		ADC1_CR2  |= 4;			/* calibration */
		while (ADC1_CR2 & 4);		/* wait until calibration done */
		printf("ADC calibration offset\t\t %d\n", ADC1_DR);
		ADC1_SMPR1 = 0x1c0000;		/* 239.5 sample cycles */
	}

	ADC1_SQR3 = ch;				/* channel selection */
	//INIT_TIMEOUT(adc, 1000);		/* a second timeout */
	ADC1_CR2 |= 1;				/* start conversion */
	while (!(ADC1_SR & 2));			/* EOC, until end of conversion */
		//if (IS_TIMEOUT(adc)) return -1;
	v = ADC1_DR;				/* get the result */

	//ADC1_CR2  &= ~1;			/* ADC1 power down */
	//SET_CLOCK_APB2(DISABLE, 9);		/* ADC1 clock disable */

	return v;
}

void disp_sysinfo()
{
	printf("Temperature\t\t\t %d(%d)C\n",
		(TEMP_V25 - getadc(16)) / TEMP_SLOPE / 10 + TEMP_OFFSET,
		getadc(16));
	printf("Internal reference voltage\t %d.%d(%d)V\n",
		AD2VOL(getadc(17)), AD2VOL_FAC(getadc(17)),
		getadc(17));
	printf("\n");

	disp_clkinfo();
}
#endif
