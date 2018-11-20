/**
 * @file reset.c
 * @brief The file contains booting process
 * @author Kyunghwan Kwon
 */

#include "io.h"
#include "arch/interrupt.h"
#include "arch/regs.h"

extern const unsigned int _etext, _edata;
extern const unsigned int _ebss;
extern const unsigned int _ram_end;
extern unsigned int _data, _bss;

static inline void mem_init(void)
{
	const unsigned int *end, *src;
	unsigned int *dst;
	unsigned int i;

	/* copy .data section from flash to sram */
	dst = (unsigned int *)&_data;
	end = (const unsigned int *)&_edata;
	src = (const unsigned int *)&_etext;

	for (i = 0; &dst[i] < end; i++)
		dst[i] = src[i];

	/* clear .bss section */
	dst = (unsigned int *)&_bss;
	end = (const unsigned int *)&_ebss;

	for (i = 0; &dst[i] < end; i++)
		dst[i] = 0;

	dsb();
}

#define VECTKEY		0x5faUL

/**
 * @brief software reset
 * @note bear in mind that all the work in progress should be done before
 * rebooting. be careful when it comes to syncronization and cache
 */
void __reboot(void)
{
	dsb();
	isb();

	SCB_AIRCR = (VECTKEY << 16)
		| (SCB_AIRCR & (7UL << 8)) /* keep priority group unchanged */
		| (1UL << 2); /* system reset request */
}

#include "kernel/init.h"

/** @brief the booting starts from here */
void __init __attribute__((naked, used)) ISR_reset(void)
{
	__cli();

	__set_sp(&_ram_end);

	/* isb() following dsb() should be put if changing a priority with
	 * interrupt enabled. Refer to:
	 * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHJICIE.html
	 */
	SCB_SHPR3 |= 0x00f00000UL; /* PendSV : the lowest priority, 15 */
	SCB_SHPR2 |= 0xf0000000UL; /* SVCall : the lowest priority, 15 */
	SCB_SHCSR |= 0x00070000UL; /* enable faults */
	SCB_CCR   |= 0x00000008UL; /* enable unaligned access traps */
	SCB_CCR   |= 0x00000200UL; /* 8-byte stack alignment */

	/* FIXME: Align stack pointer manually at every exception entry/exit
	 * when STKALIGN is not supported. if not nested interrupt, no problem
	 * actually because all the tasks use psp while msp used in interrupt
	 * context only. so should I get it done or just recommend to always
	 * use STKALIGN option.. */

#ifdef CONFIG_STACK_ALIGNMENT_4BYTE
	SCB_CCR &= ~0x200UL;
#endif

#ifdef CONFIG_NO_WRITE_BUFFER
	SCB_ACTLR |= 2U; /* disable write buffer */
#endif

	for (int i = NVECTOR_IRQ; i < (NVECTOR_IRQ + IRQ_MAX); i++)
		nvic_set_pri(i, IRQ_PRIORITY_DEFAULT);

	/* the bigger number the lower priority while 0 is the highest
	 * priority. */
	nvic_set_pri(NVECTOR_SYSTICK, IRQ_PRIORITY_LOWEST);
	nvic_set_pri(NVECTOR_SVC, IRQ_PRIORITY_LOWEST);
	nvic_set_pri(NVECTOR_PENDSV, IRQ_PRIORITY_LOWEST);

	dsb();
	isb();

	mem_init();
	irq_init();

	kernel_init();
}
