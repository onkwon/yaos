#include <io.h>
#include <kernel/init.h>
#include <kernel/task.h>

extern char _ram_end;

extern void sys_init();
extern void ISR_fault();
extern void ISR_schedule();
#ifdef CONFIG_SYSCALL
extern void ISR_svc();
#endif
extern void ISR_null(int nvec);
extern void ISR_systick();

static void ISR_reset();

static void *vectors[]
__attribute__((section(".vector"), aligned(4), used)) = {
			/* nVEC   : ADDR  - DESC */
			/* -------------------- */
	&_ram_end,	/* 00     : 0x00  - Stack pointer */
	ISR_reset,	/* 01     : 0x04  - Reset */
	ISR_fault,	/* 02     : 0x08  - NMI */
	ISR_fault,	/* 03     : 0x0c  - HardFault */
	ISR_fault,	/* 04     : 0x10  - MemManage */
	ISR_fault,	/* 05     : 0x14  - BusFault */
	ISR_fault,	/* 06     : 0x18  - UsageFault */
	ISR_null,	/* 07     : 0x1c  - Reserved */
	ISR_null,	/* 08     : 0x20  - Reserved */
	ISR_null,	/* 09     : 0x24  - Reserved */
	ISR_null,	/* 10     : 0x28  - Reserved */
#ifdef CONFIG_SYSCALL
	ISR_svc,	/* 11     : 0x2c  - SVCall */
#else
	ISR_null,	/* 11     : 0x2c  - SVCall */
#endif
	ISR_null,	/* 12     : 0x30  - Debug Monitor */
	ISR_null,	/* 13     : 0x34  - Reserved */
	ISR_schedule,	/* 14     : 0x38  - PendSV */
	ISR_systick,	/* 15     : 0x3c  - SysTick */
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

#ifdef CONFIG_COMMON_IRQ_FRAMEWORK
void irq_init();
#else
static inline void irq_init()
{
	/* copy interrupt vector table to sram */
	extern void __irq_init();
	extern char _ram_start, _rom_start;
	unsigned int *s, *d;

	s = (unsigned int *)&_rom_start;
	d = (unsigned int *)&_ram_start;

	while (*s)
		*d++ = *s++;

	__irq_init();

	/* activate vector table in sram */
	SCB_VTOR = (unsigned int)&_ram_start;

	dsb();
	isb();
}
#endif

static void __init __attribute__((naked, used)) ISR_reset()
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

	/* FIXME: align stack pointer manually at every exception entry/exit
	 * when STKALIGN is not supported. if not nested interrupt, no problem
	 * actually because all the tasks use psp while msp used in interrupt
	 * context only. so should I get it done or just recommend to always
	 * use STKALIGN option.. */

	if (STACK_ALIGNMENT == 4) /* 4-byte stack alignment */
		SCB_CCR &= ~0x200UL;

#ifdef CONFIG_WRITE_BUFFER_DISABLE
	SCB_ACTLR |= 2; /* disable write buffer */
#endif

	unsigned int i;
	for (i = NVECTOR_IRQ; i < NVECTOR_MAX; i++)
		nvic_set_pri(i, IRQ_PRIORITY_DEFAULT);

	/* the bigger number the lower priority while 0 is the highest
	 * priority. it gives systick the highest priority while giving svc and
	 * pendsv lowest priority */
	nvic_set_pri(NVECTOR_SYSTICK, IRQ_PRIORITY_HIGHEST);
	nvic_set_pri(NVECTOR_SVC, IRQ_PRIORITY_LOWEST);
	nvic_set_pri(NVECTOR_PENDSV, IRQ_PRIORITY_LOWEST);

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
