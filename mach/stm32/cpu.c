#include <foundation.h>
#include <init.h>

static void __attribute__((naked, used)) entry()
{
	cli();

	SCB_SHPR3 |= 0x00f00000; /* PendSV : the lowest priority, 15 */
	SCB_SHPR2 |= 0xf0000000; /* SVCall : the lowest priority, 15 */
	SCB_SHCSR |= 0x00070000; /* enable faults */

	main();
}

extern char _mem_end;

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
	&_mem_end,	/* 00     :       - Stack pointer */
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
	(void *)EOF
};

#include <clock.h>

static void __init mem_init()
{
	unsigned i;

	/* copy interrupt vector table to sram */
	extern char _mem_start;
	for (i = 0; (int)isr_vectors[i] != EOF; i++)
		*((unsigned *)&_mem_start + i) = (unsigned)isr_vectors[i];

	/* activate vector table in sram */
	SCB_VTOR = (unsigned)&_mem_start;

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
REGISTER_INIT_FUNC(mem_init, 1);

#include <context.h>

void set_task_context_core(struct task_t *p)
{
	int i;

	/* initialize task register set */
	*p->stack.sp     = 0x01000000;		/* psr */
	*(--p->stack.sp) = (unsigned)p->addr;	/* pc */
	for (i = 2; i < NR_CONTEXT_HARD; i++)
		*(--p->stack.sp) = 0;
}

void set_task_context(struct task_t *p)
{
	int i;

	set_task_context_core(p);

	for (i = 0; i < NR_CONTEXT_SOFT; i++)
		*(--p->stack.sp) = 0;
}
