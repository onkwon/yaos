/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#include <asm/power.h>
#include <asm/sysclk.h>
#include <io.h>

#ifndef stm32f1
#define stm32f1	1
#define stm32f4	2
#endif

enum system_control_bits {
	SLEEPONEXIT	= 1,
	SLEEPDEEP	= 2,
	SEVONPEND	= 4,
};

enum power_control_bits {
	LPDS		= 0,
	PDDS		= 1,
	CWUF		= 2,
	FPDS		= 9,
	LPUDS		= 10,
	MRUDS		= 11,
	VOS		= 14,
	ODEN		= 16,
	ODSWEN		= 17,
	UDEN		= 18,
};

enum power_status_bits {
	ODRDY		= 16,
	ODSWRDY		= 17,
};

void __set_power_regulator(bool on, int scalemode, bool overdrive)
{
	unsigned int tmp;

#if (SOC == stm32f1)
	__turn_apb1_clock(27, on); /* BKP */
#endif
	__turn_apb1_clock(28, on); /* PWR */

	tmp = PWR_CR;
	if (on)
		tmp |= (3 << UDEN) | (1 << LPUDS) | (1 << LPDS);

	tmp &= ~(3 << VOS);
	tmp |= (4 - scalemode) << VOS;
	tmp |= (1 * overdrive) << ODEN;

	PWR_CR = tmp;

	if (overdrive) {
		while (!(PWR_CSR & (1 << ODRDY)));
		PWR_CR |= 1 << ODSWEN;
		while (!(PWR_CSR & (1 << ODSWRDY)));
	}
}

sleep_t get_sleep_type()
{
	if (PWR_CR & (1 << PDDS))
		return SLEEP_BLACKOUT;
	else if (SCB_SCR & (1 << SLEEPDEEP))
		return SLEEP_DEEP;

	return SLEEP_NAP;
}

void __enter_sleep_mode()
{
	SCB_SCR &= ~(1 << SLEEPDEEP);
	/*  drain any pending memory activity before suspending execution */
	dsb();
	__wfi();
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

	SCB_SCR |= 1 << SLEEPDEEP;
	PWR_CR &= ~(1 << PDDS);

	stop_sysclk();

	/*  drain any pending memory activity before suspending execution */
	dsb();
	__wfi();

	clock_init();
	run_sysclk();

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
	SCB_SCR |= 1 << SLEEPDEEP; /* Set SLEEPDEEP bit */
	PWR_CR |= 1 << PDDS;
	PWR_CR |= 4; /* Clear WUF bit in Power Control register (PWR_CSR) */
	PWR_CSR |= 0x100; /* EWUP */

	__wfi();

	__reboot();
	/* wakeup latency: reset phase */
}

void __sleep_on_exit()
{
	SCB_SCR |= 1 << SLEEPONEXIT;
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

	/*
	unsigned int pllclk, hclk, pclk1, pclk2, adclk;

	pllclk = get_pllclk();
	hclk   = get_hclk();
	pclk1  = get_pclk1();
	pclk2  = get_pclk2();
	adclk  = get_adclk();

	printk("System clock frequency\t %d\n"
		"(hclk   %d, pclk1  %d, pclk2  %d, adclk  %d)\n\n",
		pllclk, hclk, pclk1, pclk2, adclk);
	*/
	printk("Enabled peripheral clock:\n");
#if (SOC == stm32f1)
	printk("AHB  %08x\n", RCC_AHB1ENR);
#elif (SOC == stm32f4)
	printk("AHB1 %08x\n", RCC_AHB1ENR);
	printk("AHB2 %08x\n", RCC_AHB2ENR);
	printk("AHB3 %08x\n", RCC_AHB3ENR);
#endif
	printk("APB2 %08x\n", RCC_APB2ENR);
	printk("APB1 %08x\n", RCC_APB1ENR);
	/*
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 32; j++)  {
			if (regs[i] & (1 << j)) printk(" %s", desc[i][j]);
		}
	}
	*/
	printk("\n");

	printk("PORTA %08x %08x\n", *(reg_t *)PORTA, *(reg_t *)(PORTA+4));
	printk("PORTB %08x %08x\n", *(reg_t *)PORTB, *(reg_t *)(PORTB+4));
	printk("PORTC %08x %08x\n", *(reg_t *)PORTC, *(reg_t *)(PORTC+4));
	printk("PORTD %08x %08x\n", *(reg_t *)PORTD, *(reg_t *)(PORTD+4));
	printk("PORTE %08x %08x\n", *(reg_t *)PORTE, *(reg_t *)(PORTE+4));
	printk("PORTF %08x %08x\n", *(reg_t *)PORTF, *(reg_t *)(PORTF+4));
	printk("PORTG %08x %08x\n", *(reg_t *)PORTG, *(reg_t *)(PORTG+4));
	printk("\n");
}

#include <kernel/timer.h>

#define TEMP_V25	1775	/* 1.43V */
#define TEMP_SLOPE	43
#define TEMP_OFFSET	25
#define AD2VOL(ad)	(ad * 8 / 10000)
#define AD2VOL_FAC(ad)	(ad * 8 / 100 % 100)

/* to get internal reference voltage and temperature */
static __attribute__((unused)) int getadc(int ch)
{
	int v;

	if (!(RCC_APB2ENR & (1 << 9))) {
		__turn_apb2_clock(9, ENABLE);	/* ADC1 */
		ADC1_CR2   = 1 | 0x800000;	/* ADC1 and TSVREFE power on */
		udelay(1000);			/* delay */
		ADC1_CR2  |= 4;			/* calibration */
		while (ADC1_CR2 & 4);		/* wait until calibration done */
		printk("ADC calibration offset\t\t %d\n", ADC1_DR);
		ADC1_SMPR1 = 0x1c0000;		/* 239.5 sample cycles */
	}

	ADC1_SQR3 = ch;				/* channel selection */
	//INIT_TIMEOUT(adc, 1000);		/* a second timeout */
	ADC1_CR2 |= 1;				/* start conversion */
	while (!(ADC1_SR & 2));			/* EOC, until end of conversion */
		//if (IS_TIMEOUT(adc)) return -1;
	v = ADC1_DR;				/* get the result */

	//ADC1_CR2  &= ~1;			/* ADC1 power down */
	//SET_CLOCK_APB2(9, DISABLE);		/* ADC1 clock disable */

	return v;
}

void disp_sysinfo()
{
#if 0
	printk("Temperature\t\t\t %d(%d)C\n",
		(TEMP_V25 - getadc(16)) / TEMP_SLOPE / 10 + TEMP_OFFSET,
		getadc(16));
	printk("Internal reference voltage\t %d.%d(%d)V\n",
		AD2VOL(getadc(17)), AD2VOL_FAC(getadc(17)),
		getadc(17));
	printk("\n");
#endif

	disp_clkinfo();
}
#endif

unsigned int __read_reset_source()
{
	unsigned int ret;

	ret = RCC_CSR;
	RCC_CSR |= 1 << 24;

	return ret >> 24;
}
