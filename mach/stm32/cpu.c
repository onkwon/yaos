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

	dsb();

	port_init();
	main();
}

extern char _ram_end;

extern void sys_init();
extern void isr_fault();
extern void __schedule();
#ifdef CONFIG_SYSCALL
extern void svc_handler();
#endif
void isr_null() { debug("ISR is not registered yet"); }

static void *isr_vectors[]
__attribute__((section(".vector"), aligned(4))) = {
			/* NUM(IRQ): ADDR - DESC */
			/* -------------------- */
	&_ram_end,	/* 00     :       - Stack pointer */
	entry,		/* 01     : 0x04  - Reset */
	isr_fault,	/* 02     : 0x08  - NMI */
	isr_fault,	/* 03     : 0x0c  - HardFault */
	isr_fault,	/* 04     : 0x10  - MemManage */
	isr_fault,	/* 05     : 0x14  - BusFault */
	isr_fault,	/* 06     : 0x18  - UsageFault */
	NULL,		/* 07     : 0x1c  - Reserved */
	NULL,		/* 08     : 0x20  - Reserved */
	NULL,		/* 09     : 0x24  - Reserved */
	NULL,		/* 10     : 0x28  - Reserved */
#ifdef CONFIG_SYSCALL
	svc_handler,	/* 11     : 0x2c  - SVCall */
#else
	isr_null,	/* 11     : 0x2c  - SVCall */
#endif
	isr_null,	/* 12     : 0x30  - Debug Monitor */
	NULL,		/* 13     : 0x34  - Reserved */
	__schedule,	/* 14     : 0x38  - PendSV */
	isr_null,	/* 15     : 0x3c  - SysTick */
	isr_null,	/* 16(00) : 0x40  - WWDG */
	isr_null,	/* 17(01) : 0x44  - PVD */
	isr_null,	/* 18(02) : 0x48  - TAMPER */
	isr_null,	/* 19(03) : 0x4c  - RTC */
	isr_null,	/* 20(04) : 0x50  - FLASH */
	isr_null,	/* 21(05) : 0x54  - RCC */
	isr_null,	/* 22(06) : 0x58  - EXTI0 */
	isr_null,	/* 23(07) : 0x5c  - EXTI1 */
	isr_null,	/* 24(08) : 0x60  - EXTI2 */
	isr_null,	/* 25(09) : 0x64  - EXTI3 */
	isr_null,	/* 26(10) : 0x68  - EXTI4 */
	isr_null,	/* 27(11) : 0x6c  - DMA1_Channel1 */
	isr_null,	/* 28(12) : 0x70  - DMA1_Channel2 */
	isr_null,	/* 29(13) : 0x74  - DMA1_Channel3 */
	isr_null,	/* 30(14) : 0x78  - DMA1_Channel4 */
	isr_null,	/* 31(15) : 0x7c  - DMA1_Channel5 */
	isr_null,	/* 32(16) : 0x80  - DMA1_Channel6 */
	isr_null,	/* 33(17) : 0x84  - DMA1_Channel7 */
	isr_null,	/* 34(18) : 0x88  - ADC1 | ADC2 */
	isr_null,	/* 35(19) : 0x8c  - USB High Priority | CAN TX */
	isr_null,	/* 36(20) : 0x90  - USB Low Priority | CAN RX0 */
	isr_null,	/* 37(21) : 0x94  - CAN RX1 */
	isr_null,	/* 38(22) : 0x98  - CAN SCE */
	isr_null,	/* 39(23) : 0x9c  - EXTI[9:5] */
	isr_null,	/* 40(24) : 0xa0  - TIM1 Break */
	isr_null,	/* 41(25) : 0xa4  - TIM1 Update */
	isr_null,	/* 42(26) : 0xa8  - TIM1 Trigger | Communication */
	isr_null,	/* 43(27) : 0xac  - TIM1 Capture Compare */
	isr_null,	/* 44(28) : 0xb0  - TIM2 */
	isr_null,	/* 45(29) : 0xb4  - TIM3 */
	isr_null,	/* 46(30) : 0xb8  - TIM4 */
	isr_null,	/* 47(31) : 0xbc  - I2C1 Event */
	isr_null,	/* 48(32) : 0xc0  - I2C1 Error */
	isr_null,	/* 49(33) : 0xc4  - I2C2 Event */
	isr_null,	/* 50(34) : 0xc8  - I2C2 Error */
	isr_null,	/* 51(35) : 0xcc  - SPI1 */
	isr_null,	/* 52(36) : 0xd0  - SPI2 */
	isr_null,	/* 53(37) : 0xd4  - USART1 */
	isr_null,	/* 54(38) : 0xd8  - USART2 */
	isr_null,	/* 55(39) : 0xdc  - USART3 */
	isr_null,	/* 56(40) : 0xe0  - EXTI[15:10] */
	isr_null,	/* 57(41) : 0xe4  - RTC Alarm */
	isr_null,	/* 58(42) : 0xe8  - USB Wakeup */
	isr_null,	/* 59(43) : 0xec  - TIM8 Break */
	isr_null,	/* 60(44) : 0xf0  - TIM8 Update */
	isr_null,	/* 61(45) : 0xf4  - TIM8 Trigger | Communication */
	isr_null,	/* 62(46) : 0xf8  - TIM8 Capture Compare */
	isr_null,	/* 63(47) : 0xfc  - ADC3 */
	isr_null,	/* 64(48) : 0x100 - FSMC */
	isr_null,	/* 65(49) : 0x104 - SDIO */
	isr_null,	/* 66(50) : 0x108 - TIM5 */
	isr_null,	/* 67(51) : 0x10c - SPI3 */
	isr_null,	/* 68(52) : 0x110 - UART4 */
	isr_null,	/* 69(53) : 0x114 - UART5 */
	isr_null,	/* 70(54) : 0x118 - TIM6 */
	isr_null,	/* 71(55) : 0x11c - TIM7 */
	isr_null,	/* 72(56) : 0x120 - DMA2_Channel1 */
	isr_null,	/* 73(57) : 0x124 - DMA2_Channel2 */
	isr_null,	/* 74(58) : 0x128 - DMA2_Channel3 */
	isr_null,	/* 75(59) : 0x12c - DMA2_Channel4,5 */
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

	dsb();
	isb();

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
	unsigned int i;

	*(--p->mm.sp) = INIT_PSR;		/* psr */
	*(--p->mm.sp) = (unsigned int)addr;	/* pc */
	for (i = 2; i < NR_CONTEXT_HARD; i++)
		*(--p->mm.sp) = 0;
}

void set_task_context_soft(struct task *p)
{
	unsigned int i;

	for (i = 0; i < NR_CONTEXT_SOFT; i++)
		*(--p->mm.sp) = 0;
}

void set_task_context(struct task *p, void *addr)
{
	set_task_context_hard(p, addr);
	set_task_context_soft(p);
}
