#include <foundation.h>
#include <kernel/init.h>

static void __init port_init()
{
	SET_PORT_CLOCK(ENABLE, PORTA);
	SET_PORT_CLOCK(ENABLE, PORTB);
	SET_PORT_CLOCK(ENABLE, PORTC);
	SET_PORT_CLOCK(ENABLE, PORTD);
	SET_PORT_CLOCK(ENABLE, PORTE);
	SET_PORT_CLOCK(ENABLE, PORTF);
	SET_PORT_CLOCK(ENABLE, PORTG);

	/* set pins to AIN to reduce power consumption */
	*(volatile unsigned int *)PORTA = 0;
	*(volatile unsigned int *)(PORTA+4) = 0;
	*(volatile unsigned int *)PORTB = 0;
	*(volatile unsigned int *)(PORTB+4) = 0;
	*(volatile unsigned int *)PORTC = 0;
	*(volatile unsigned int *)(PORTC+4) = 0;
	*(volatile unsigned int *)PORTD = 0;
	*(volatile unsigned int *)(PORTD+4) = 0;
	*(volatile unsigned int *)PORTE = 0;
	*(volatile unsigned int *)(PORTE+4) = 0;
	*(volatile unsigned int *)PORTF = 0;
	*(volatile unsigned int *)(PORTF+4) = 0;
	*(volatile unsigned int *)PORTG = 0;
	*(volatile unsigned int *)(PORTG+4) = 0;

	SET_PORT_CLOCK(DISABLE, PORTA);
	SET_PORT_CLOCK(DISABLE, PORTB);
	SET_PORT_CLOCK(DISABLE, PORTC);
	SET_PORT_CLOCK(DISABLE, PORTD);
	SET_PORT_CLOCK(DISABLE, PORTE);
	SET_PORT_CLOCK(DISABLE, PORTF);
	SET_PORT_CLOCK(DISABLE, PORTG);
}

static void __init __attribute__((naked, used)) entry()
{
	cli();

	SCB_SHPR3 |= 0x00f00000; /* PendSV : the lowest priority, 15 */
	SCB_SHPR2 |= 0xf0000000; /* SVCall : the lowest priority, 15 */
	SCB_SHCSR |= 0x00070000; /* enable faults */

	port_init();

	main();
}

extern char _ram_end;

extern void sys_init();
extern void isr_default();
extern void pendsv_handler();
#ifdef CONFIG_SYSCALL
extern void svc_handler();
#endif

static void *isr_vectors[]
__attribute__((section(".vector"), aligned(4))) = {
			/* NUM(IRQ): ADDR - DESC */
			/* -------------------- */
	&_ram_end,	/* 00     :       - Stack pointer */
	entry,		/* 01     : 0x04  - Reset */
	isr_default,	/* 02     : 0x08  - NMI */
	isr_default,	/* 03     : 0x0c  - HardFault */
	isr_default,	/* 04     : 0x10  - MemManage */
	isr_default,	/* 05     : 0x14  - BusFault */
	isr_default,	/* 06     : 0x18  - UsageFault */
	NULL,		/* 07     : 0x1c  - Reserved */
	NULL,		/* 08     : 0x20  - Reserved */
	NULL,		/* 09     : 0x24  - Reserved */
	NULL,		/* 10     : 0x28  - Reserved */
#ifdef CONFIG_SYSCALL
	svc_handler,	/* 11     : 0x2c  - SVCall */
#else
	isr_default,	/* 11     : 0x2c  - SVCall */
#endif
	isr_default,	/* 12     : 0x30  - Debug Monitor */
	NULL,		/* 13     : 0x34  - Reserved */
	pendsv_handler,	/* 14     : 0x38  - PendSV */
	isr_default,	/* 15     : 0x3c  - SysTick */
	isr_default,	/* 16(00) : 0x40  - WWDG */
	isr_default,	/* 17(01) : 0x44  - PVD */
	isr_default,	/* 18(02) : 0x48  - TAMPER */
	isr_default,	/* 19(03) : 0x4c  - RTC */
	isr_default,	/* 20(04) : 0x50  - FLASH */
	isr_default,	/* 21(05) : 0x54  - RCC */
	isr_default,	/* 22(06) : 0x58  - EXTI0 */
	isr_default,	/* 23(07) : 0x5c  - EXTI1 */
	isr_default,	/* 24(08) : 0x60  - EXTI2 */
	isr_default,	/* 25(09) : 0x64  - EXTI3 */
	isr_default,	/* 26(10) : 0x68  - EXTI4 */
	isr_default,	/* 27(11) : 0x6c  - DMA1_Channel1 */
	isr_default,	/* 28(12) : 0x70  - DMA1_Channel2 */
	isr_default,	/* 29(13) : 0x74  - DMA1_Channel3 */
	isr_default,	/* 30(14) : 0x78  - DMA1_Channel4 */
	isr_default,	/* 31(15) : 0x7c  - DMA1_Channel5 */
	isr_default,	/* 32(16) : 0x80  - DMA1_Channel6 */
	isr_default,	/* 33(17) : 0x84  - DMA1_Channel7 */
	isr_default,	/* 34(18) : 0x88  - ADC1 | ADC2 */
	isr_default,	/* 35(19) : 0x8c  - USB High Priority | CAN TX */
	isr_default,	/* 36(20) : 0x90  - USB Low Priority | CAN RX0 */
	isr_default,	/* 37(21) : 0x94  - CAN RX1 */
	isr_default,	/* 38(22) : 0x98  - CAN SCE */
	isr_default,	/* 39(23) : 0x9c  - EXTI[9:5] */
	isr_default,	/* 40(24) : 0xa0  - TIM1 Break */
	isr_default,	/* 41(25) : 0xa4  - TIM1 Update */
	isr_default,	/* 42(26) : 0xa8  - TIM1 Trigger | Communication */
	isr_default,	/* 43(27) : 0xac  - TIM1 Capture Compare */
	isr_default,	/* 44(28) : 0xb0  - TIM2 */
	isr_default,	/* 45(29) : 0xb4  - TIM3 */
	isr_default,	/* 46(30) : 0xb8  - TIM4 */
	isr_default,	/* 47(31) : 0xbc  - I2C1 Event */
	isr_default,	/* 48(32) : 0xc0  - I2C1 Error */
	isr_default,	/* 49(33) : 0xc4  - I2C2 Event */
	isr_default,	/* 50(34) : 0xc8  - I2C2 Error */
	isr_default,	/* 51(35) : 0xcc  - SPI1 */
	isr_default,	/* 52(36) : 0xd0  - SPI2 */
	isr_default,	/* 53(37) : 0xd4  - USART1 */
	isr_default,	/* 54(38) : 0xd8  - USART2 */
	isr_default,	/* 55(39) : 0xdc  - USART3 */
	isr_default,	/* 56(40) : 0xe0  - EXTI[15:10] */
	isr_default,	/* 57(41) : 0xe4  - RTC Alarm */
	isr_default,	/* 58(42) : 0xe8  - USB Wakeup */
	isr_default,	/* 59(43) : 0xec  - TIM8 Break */
	isr_default,	/* 60(44) : 0xf0  - TIM8 Update */
	isr_default,	/* 61(45) : 0xf4  - TIM8 Trigger | Communication */
	isr_default,	/* 62(46) : 0xf8  - TIM8 Capture Compare */
	isr_default,	/* 63(47) : 0xfc  - ADC3 */
	isr_default,	/* 64(48) : 0x100 - FSMC */
	isr_default,	/* 65(49) : 0x104 - SDIO */
	isr_default,	/* 66(50) : 0x108 - TIM5 */
	isr_default,	/* 67(51) : 0x10c - SPI3 */
	isr_default,	/* 68(52) : 0x110 - UART4 */
	isr_default,	/* 69(53) : 0x114 - UART5 */
	isr_default,	/* 70(54) : 0x118 - TIM6 */
	isr_default,	/* 71(55) : 0x11c - TIM7 */
	isr_default,	/* 72(56) : 0x120 - DMA2_Channel1 */
	isr_default,	/* 73(57) : 0x124 - DMA2_Channel2 */
	isr_default,	/* 74(58) : 0x128 - DMA2_Channel3 */
	isr_default,	/* 75(59) : 0x12c - DMA2_Channel4,5 */
	NULL
};

#include <clock.h>

static void __init mem_init()
{
	unsigned int i;

	/* copy interrupt vector table to sram */
	extern char _ram_start;
	for (i = 0; (int)isr_vectors[i] != EOF; i++)
		*((unsigned int *)&_ram_start + i) =
			(unsigned int)isr_vectors[i];

	/* activate vector table in sram */
	SCB_VTOR = (unsigned int)&_ram_start;

	/* copy .data section from flash to sram */
	extern char _etext, _data, _edata;
	for (i = 0; (&_data + i) < &_edata; i++)
		*((char *)&_data + i) = *((char *)&_etext + i);

	/* clear .bss section */
	extern char _bss, _ebss;
	for (i = 0; (&_bss + i) < &_ebss; i++)
		*((char *)&_bss + i) = 0;

	/* reset flag */
	//resetf   = RCC_CSR >> 26;
	RCC_CSR |= 0x01000000;  /* clear reset flags */

	dmb();
}
REGISTER_INIT(mem_init, 1);

#include <kernel/task.h>
#include <context.h>

void set_task_context_hard(struct task *p, void *addr)
{
	int i;

	*(--p->mm.sp) = INIT_PSR;		/* psr */
	*(--p->mm.sp) = (unsigned int)addr;	/* pc */
	for (i = 2; i < NR_CONTEXT_HARD; i++)
		*(--p->mm.sp) = 0;
}

void set_task_context_soft(struct task *p)
{
	int i;

	for (i = 0; i < NR_CONTEXT_SOFT; i++)
		*(--p->mm.sp) = 0;
}

void set_task_context(struct task *p, void *addr)
{
	set_task_context_hard(p, addr);
	set_task_context_soft(p);
}

#include <power.h>

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
	schedule_off();
	__wfi();
	schedule_on();
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

	schedule_off();
	__wfi();
	clock_init();
	schedule_on();

	SCB_SCR &= ~4;
	PWR_CR &= ~1;
	PWRCLK_DISABLE();

	irq_restore(irqflag);

	/* wakeup latency:
	 * HSI RC wakeup time + regulator wakeup time from Low-power mode */
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
	int i, j;
	char **desc[3] = { clkname_ahb, clkname_apb2, clkname_apb1 };
	unsigned int regs[3] = { RCC_AHBENR, RCC_APB2ENR, RCC_APB1ENR };
	unsigned int sysclk, hclk, pclk1, pclk2, adclk;

	sysclk = get_sysclk();
	hclk   = get_hclk  (sysclk);
	pclk1  = get_pclk1 (hclk);
	pclk2  = get_pclk2 (hclk);
	adclk  = get_adclk (pclk2);

	printf("System clock frequency\t %d\n"
		"(hclk   %d, pclk1  %d, pclk2  %d, adclk  %d)\n\n",
		sysclk, hclk, pclk1, pclk2, adclk);
	printf("Enabled peripheral clock:\n");
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 32; j++)  {
			if (regs[i] & (1 << j)) printf(" %s", desc[i][j]);
		}
	}
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
		mdelay(1);			/* delay */
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
