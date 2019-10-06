#include "io.h"
#include "arch/regs.h"
#include "kernel/interrupt.h"
#include "kernel/init.h"

#define VECTKEY		0x5faUL

extern const uintptr_t _ram_end;

void hw_reboot(void)
{
	dsb();
	isb();

	SCB_AIRCR = (VECTKEY << 16)
		| (SCB_AIRCR & (7UL << 8)) /* keep priority group unchanged */
		| (1UL << 2); /* system reset request */
}

void __attribute__((naked, used)) ISR_reset(void)
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

	/* NOTE: Align stack pointer manually at every exception entry/exit
	 * when STKALIGN is not supported. if not nested interrupt, no problem
	 * actually because all the tasks use psp while msp used in interrupt
	 * context only. Recommend to use STKALIGN option always. */

#ifdef CONFIG_STACK_ALIGNMENT_4BYTE
	SCB_CCR &= ~0x200UL;
#endif

#ifdef CONFIG_NO_WRITE_BUFFER
	SCB_ACTLR |= 2U; /* disable write buffer */
#endif

	for (int i = NVECTOR_IRQ; i < (NVECTOR_IRQ + IRQ_MAX); i++)
		hw_irq_set_pri(i, IRQ_PRIORITY_DEFAULT);

	/* the bigger number the lower priority while 0 is the highest
	 * priority. */
	hw_irq_set_pri(NVECTOR_SYSTICK, IRQ_PRIORITY_LOWEST);
	hw_irq_set_pri(NVECTOR_SVC, IRQ_PRIORITY_LOWEST);
	hw_irq_set_pri(NVECTOR_PENDSV, IRQ_PRIORITY_LOWEST);

	dsb();
	isb();

	kernel_init();
}
