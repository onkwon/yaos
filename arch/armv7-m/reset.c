#include <io.h>
#include <kernel/init.h>
#include <kernel/task.h>

#define NVIC_DEFAULT_PRIORITY		10

extern char _ram_end;

extern void sys_init();
extern void isr_fault();
extern void __schedule();
extern void irq_handler();
#ifdef CONFIG_SYSCALL
extern void svc_handler();
#endif
extern void isr_null(int nvec);
extern void isr_systick();

static void __reset();

static void *isr_vectors[]
__attribute__((section(".vector"), aligned(4), used)) = {
				/* NUM(IRQ): ADDR - DESC */
				/* -------------------- */
	&_ram_end,		/* 00     : 0x00  - Stack pointer */
	__reset,		/* 01     : 0x04  - Reset */
	isr_fault,		/* 02     : 0x08  - NMI */
	isr_fault,		/* 03     : 0x0c  - HardFault */
	isr_fault,		/* 04     : 0x10  - MemManage */
	isr_fault,		/* 05     : 0x14  - BusFault */
	isr_fault,		/* 06     : 0x18  - UsageFault */
	isr_null,		/* 07     : 0x1c  - Reserved */
	isr_null,		/* 08     : 0x20  - Reserved */
	isr_null,		/* 09     : 0x24  - Reserved */
	isr_null,		/* 10     : 0x28  - Reserved */
#ifdef CONFIG_SYSCALL
	svc_handler,		/* 11     : 0x2c  - SVCall */
#else
	isr_null,		/* 11     : 0x2c  - SVCall */
#endif
	isr_null,		/* 12     : 0x30  - Debug Monitor */
	isr_null,		/* 13     : 0x34  - Reserved */
	__schedule,		/* 14     : 0x38  - PendSV */
	isr_systick,		/* 15     : 0x3c  - SysTick */
	irq_handler,		/* 16(00) : 0x40  - WWDG */
	irq_handler,		/* 17(01) : 0x44  - PVD */
	irq_handler,		/* 18(02) : 0x48  - TAMPER */
	irq_handler,		/* 19(03) : 0x4c  - RTC */
	irq_handler,		/* 20(04) : 0x50  - FLASH */
	irq_handler,		/* 21(05) : 0x54  - RCC */
	irq_handler,		/* 22(06) : 0x58  - EXTI0 */
	irq_handler,		/* 23(07) : 0x5c  - EXTI1 */
	irq_handler,		/* 24(08) : 0x60  - EXTI2 */
	irq_handler,		/* 25(09) : 0x64  - EXTI3 */
	irq_handler,		/* 26(10) : 0x68  - EXTI4 */
	irq_handler,		/* 27(11) : 0x6c  - DMA1_Channel1 */
	irq_handler,		/* 28(12) : 0x70  - DMA1_Channel2 */
	irq_handler,		/* 29(13) : 0x74  - DMA1_Channel3 */
	irq_handler,		/* 30(14) : 0x78  - DMA1_Channel4 */
	irq_handler,		/* 31(15) : 0x7c  - DMA1_Channel5 */
	irq_handler,		/* 32(16) : 0x80  - DMA1_Channel6 */
	irq_handler,		/* 33(17) : 0x84  - DMA1_Channel7 */
	irq_handler,		/* 34(18) : 0x88  - ADC1 | ADC2 */
	irq_handler,		/* 35(19) : 0x8c  - USB High Priority | CAN TX */
	irq_handler,		/* 36(20) : 0x90  - USB Low Priority | CAN RX0 */
	irq_handler,		/* 37(21) : 0x94  - CAN RX1 */
	irq_handler,		/* 38(22) : 0x98  - CAN SCE */
	irq_handler,		/* 39(23) : 0x9c  - EXTI[9:5] */
	irq_handler,		/* 40(24) : 0xa0  - TIM1 Break */
	irq_handler,		/* 41(25) : 0xa4  - TIM1 Update */
	irq_handler,		/* 42(26) : 0xa8  - TIM1 Trigger | Communication */
	irq_handler,		/* 43(27) : 0xac  - TIM1 Capture Compare */
	irq_handler,		/* 44(28) : 0xb0  - TIM2 */
	irq_handler,		/* 45(29) : 0xb4  - TIM3 */
	irq_handler,		/* 46(30) : 0xb8  - TIM4 */
	irq_handler,		/* 47(31) : 0xbc  - I2C1 Event */
	irq_handler,		/* 48(32) : 0xc0  - I2C1 Error */
	irq_handler,		/* 49(33) : 0xc4  - I2C2 Event */
	irq_handler,		/* 50(34) : 0xc8  - I2C2 Error */
	irq_handler,		/* 51(35) : 0xcc  - SPI1 */
	irq_handler,		/* 52(36) : 0xd0  - SPI2 */
	irq_handler,		/* 53(37) : 0xd4  - USART1 */
	irq_handler,		/* 54(38) : 0xd8  - USART2 */
	irq_handler,		/* 55(39) : 0xdc  - USART3 */
	irq_handler,		/* 56(40) : 0xe0  - EXTI[15:10] */
	irq_handler,		/* 57(41) : 0xe4  - RTC Alarm */
	irq_handler,		/* 58(42) : 0xe8  - USB Wakeup */
	irq_handler,		/* 59(43) : 0xec  - TIM8 Break */
	irq_handler,		/* 60(44) : 0xf0  - TIM8 Update */
	irq_handler,		/* 61(45) : 0xf4  - TIM8 Trigger | Communication */
	irq_handler,		/* 62(46) : 0xf8  - TIM8 Capture Compare */
	irq_handler,		/* 63(47) : 0xfc  - ADC3 */
	irq_handler,		/* 64(48) : 0x100 - FSMC */
	irq_handler,		/* 65(49) : 0x104 - SDIO */
	irq_handler,		/* 66(50) : 0x108 - TIM5 */
	irq_handler,		/* 67(51) : 0x10c - SPI3 */
	irq_handler,		/* 68(52) : 0x110 - UART4 */
	irq_handler,		/* 69(53) : 0x114 - UART5 */
	irq_handler,		/* 70(54) : 0x118 - TIM6 */
	irq_handler,		/* 71(55) : 0x11c - TIM7 */
	irq_handler,		/* 72(56) : 0x120 - DMA2_Channel1 */
	irq_handler,		/* 73(57) : 0x124 - DMA2_Channel2 */
	irq_handler,		/* 74(58) : 0x128 - DMA2_Channel3 */
	irq_handler,		/* 75(59) : 0x12c - DMA2_Channel4,5 */
	(void *)EOF
};

static inline void mem_init()
{
	unsigned int i;

	/* copy .data section from flash to sram */
	extern char _etext, _data, _edata;
	for (i = 0; (((unsigned int *)&_data) + i) < (unsigned int *)&_edata;
			i++)
		((unsigned int *)&_data)[i] = ((unsigned int *)&_etext)[i];

	/* clear .bss section */
	extern char _bss, _ebss;
	for (i = 0; (((unsigned int *)&_bss) + i) < (unsigned int *)&_ebss; i++)
		((unsigned int *)&_bss)[i] = 0;

	dsb();
}

#ifdef CONFIG_IRQ_HIERARCHY
void irq_init();
#else
static inline void irq_init()
{
	int i;

	/* copy interrupt vector table to sram */
	extern char _ram_start;
	for (i = 0; (int)isr_vectors[i] != EOF; i++)
		*((unsigned int *)&_ram_start + i) =
			(unsigned int)isr_vectors[i];

	/* activate vector table in sram */
	SCB_VTOR = (unsigned int)&_ram_start;

	dsb();
	isb();
}
#endif

static void __init __attribute__((naked, used)) __reset()
{
	cli();

	/* isb() following dsb() should be put if changing a priority with
	 * interrupt enabled. Refer to:
	 * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHJICIE.html
	 */
	SCB_SHPR3 |= 0x00f00000; /* PendSV : the lowest priority, 15 */
	SCB_SHPR2 |= 0xf0000000; /* SVCall : the lowest priority, 15 */
	SCB_SHCSR |= 0x00070000; /* enable faults */
	SCB_CCR   |= 0x00000008; /* enable unaligned access traps */
	SCB_CCR   |= 0x00000200; /* 8-byte stack alignment */

	if (STACK_ALIGNMENT == 4) /* 4-byte stack alignment */
		SCB_CCR &= ~0x200UL;

#ifdef CONFIG_WRITE_BUFFER_DISABLE
	SCB_ACTLR |= 2; /* disable write buffer */
#endif

	unsigned int i;
	for (i = NVECTOR_IRQ; i < NVECTOR_MAX; i++)
		nvic_set_pri(i, NVIC_DEFAULT_PRIORITY);

	/* the bigger number the lower priority while 0 is the highest
	 * priority. it gives systick the highest priority while giving svc and
	 * pendsv lowest priority */
	nvic_set_pri(NVECTOR_SYSTICK, 1);
	nvic_set_pri(NVECTOR_SVC, NVIC_DEFAULT_PRIORITY + 1);
	nvic_set_pri(NVECTOR_PENDSV, NVIC_DEFAULT_PRIORITY + 1);

	dsb();
	isb();

	mem_init();
	irq_init();

	kernel_init();
}

#define VECTKEY		0x5fa

void __reboot()
{
	SCB_AIRCR = (VECTKEY << 16)
		| (SCB_AIRCR & (7 << 8)) /* keep priority group unchanged */
		| (1 << 2); /* system reset request */
}
