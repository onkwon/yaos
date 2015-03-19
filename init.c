#include "foundation.h"
#include "io.h"

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

#include "sched.h"

static void __attribute__((naked, used)) svc_handler(unsigned *sp)
{
	/* we have return address(pc) stacked in sp[6].
	 * svc parameter can be gotten by accessing the prior code of return
	 * address, pc[-2] while pc[-1] of 'df'(svc mnemonic).
	 *
	 * e.g.
	 *  [-2][-1]
	 *   `df 40      	svc	64`
	 *   `f3 ef 83 03 	mrs	r3, PSR` # code of return address
	 *    [0][1][2][3]
	 */

	switch ( ((char *)sp[6])[-2] ) {
	case 0:
		schedule_prepare();
		schedule_core();
		schedule_finish();
		break;
	default:
		__asm__ __volatile__("push {lr}");
		DBUG(("no handler!\n"));
		__asm__ __volatile__("pop {lr}");
		break;
	}

	__asm__ __volatile__("bx lr");
}

static void __attribute__((naked)) __svc_handler()
{
	__asm__ __volatile__(
		"mov	r0, sp		\n\t"
		"b	svc_handler	\n\t"
		/* never reach out up to the code below */
		"bx	lr		\n\t"
	);
}

static void __attribute__((naked)) isr_default()
{
	unsigned sp, lr, psr;

	sp  = GET_SP ();
	psr = GET_PSR();
	lr  = GET_LR ();

	/* EXC_RETURN:
	 * 0xFFFFFFF1 - MSP, return to handler mode
	 * 0xFFFFFFF9 - MSP, return to thread mode
	 * 0xFFFFFFFD - PSP, return to thread mode */
	DBUG(("\nStacked PSR    0x%08x\n"
		"Stacked PC     0x%08x\n"
		"Stacked LR     0x%08x\n"
		"Current LR     0x%08x\n"
		"Current PSR    0x%08x(vector number:%d)\n",
		*(unsigned *)(sp + 28),
		*(unsigned *)(sp + 24),
		*(unsigned *)(sp + 20),
		lr, psr, psr & 0x1ff));

	/* led for debugging */
	SET_PORT_CLOCK(ENABLE, PORTD);
	SET_PORT_PIN(PORTD, 2, PIN_OUTPUT_50MHZ);
	while (1) {
		PUT_PORT(PORTD, GET_PORT(PORTD) ^ 4);
		mdelay(100);
	}
}

extern char _sram_end;

static void __init();

static void *isr_vectors[] 
__attribute__((section(".vector"), aligned(4))) = {
			/* NUM(IRQ): ADDR - DESC */
			/* -------------------- */
	&_sram_end,	/* 00     :       - Stack pointer */
	__init,		/* 01     : 0x04  - Reset */
	isr_default,	/* 02     : 0x08  - NMI */
	isr_default,	/* 03     : 0x0c  - HardFault */
	isr_default,	/* 04     : 0x10  - MemManage */
	isr_default,	/* 05     : 0x14  - BusFault */
	isr_default,	/* 06     : 0x18  - UsageFault */
	NULL,		/* 07     : 0x1c  - Reserved */
	NULL,		/* 08     : 0x20  - Reserved */
	NULL,		/* 09     : 0x24  - Reserved */
	NULL,		/* 10     : 0x28  - Reserved */
	__svc_handler,	/* 11     : 0x2c  - SVCall */
	isr_default,	/* 12     : 0x30  - Debug Monitor */
	NULL,		/* 13     : 0x34  - Reserved */
	isr_default,	/* 14     : 0x38  - PendSV */
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
	(void *)EOF
};

unsigned get_sysclk()
{
	unsigned clk, pllmul;

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

unsigned get_hclk(unsigned sysclk)
{
	unsigned clk, pre;

	pre = (RCC_CFGR >> HPRE) & 0xf; /* mask HPRE[7:4] */
	pre = pre? pre - 7 : 0;         /* get prescaler division factor */
	clk = sysclk >> pre;

	return clk;
}

unsigned get_pclk1(unsigned hclk)
{
	unsigned clk, pre;

	pre = (RCC_CFGR >> PPRE1) & 0x7; /* mask PPRE1[10:8] */
	pre = pre? pre - 3 : 0;
	clk = hclk >> pre;

	return clk;
}

unsigned get_pclk2(unsigned hclk)
{
	unsigned clk, pre;

	pre = (RCC_CFGR >> PPRE2) & 0x7; /* mask PPRE2[13:11] */
	pre = pre? pre - 3 : 0;
	clk = hclk >> pre;

	return clk;
}

unsigned get_adclk(unsigned pclk2)
{
	unsigned clk, pre;

	pre = (RCC_CFGR >> ADCPRE) & 0x3; /* mask PPRE2[15:14] */
	pre = (pre + 1) << 1;             /* get prescaler division factor */
	clk = pclk2 / pre;

	return clk;
}

unsigned get_stkclk(unsigned hclk)
{
	unsigned clk;

	if (STK_CTRL & 4)
		clk = hclk;
	else
		clk = hclk >> 3;

	return clk;
}

static void clock_init()
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

static int resetf;

__attribute__((used)) static void __init()
{
	unsigned i;

	cli();

	clock_init();

	/* copy interrupt vector table to sram */
	extern char _sram_start;
	for (i = 0; (int)isr_vectors[i] != EOF; i++)
		*((unsigned *)&_sram_start + i) = (unsigned)isr_vectors[i];

	__asm__ __volatile__("dsb");

	/* activate vector table in sram */
	SCB_VTOR = (unsigned)&_sram_start;

	/* copy .data section from flash to sram */
	extern char _etext, _data, _edata;
	for (i = 0; (&_data + i) < &_edata; i++)
		*((char *)&_data + i) = *((char *)&_etext + i);

	/* clear .bss section */
	extern char _bss, _ebss;
	for (i = 0; (&_bss + i) < &_ebss; i++)
		*((char *)&_bss + i) = 0;

	resetf   = RCC_CSR >> 26;
	RCC_CSR |= 0x01000000;  /* clear reset flags */

	sei();

	extern int main();
	main();
}

int get_resetf()
{
	return resetf;
}
