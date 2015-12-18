#include <foundation.h>
#include <kernel/task.h>

void __attribute__((naked)) isr_fault()
{
	unsigned int sp, lr, psr, usp;

	sp  = GET_SP ();
	psr = GET_PSR();
	lr  = GET_LR ();
	usp = GET_USP();

	printk("\nKernel SP      0x%08x\n"
		"Stacked PSR    0x%08x\n"
		"Stacked PC     0x%08x\n"
		"Stacked LR     0x%08x\n"
		"Current LR     0x%08x\n"
		"Current PSR    0x%08x(vector number:%d)\n", sp,
		*(unsigned int *)(sp + 28),
		*(unsigned int *)(sp + 24),
		*(unsigned int *)(sp + 20),
		lr, psr, psr & 0x1ff);
	printk("\nUser SP        0x%08x\n"
		"Stacked PSR    0x%08x\n"
		"Stacked PC     0x%08x\n"
		"Stacked LR     0x%08x\n",
		usp,
		*(unsigned int *)(usp + 28),
		*(unsigned int *)(usp + 24),
		*(unsigned int *)(usp + 20));

	printk("\ncurrent->sp         0x%08x\n"
		"current->base       0x%08x\n"
		"current->heap       0x%08x\n"
		"current->kernel     0x%08x\n"
		"current->kernel->sp 0x%08x\n"
		"current->state      0x%08x\n"
		"current->irqflag    0x%08x\n"
		"current->addr       0x%08x\n"
		"current             0x%08x\n"
		, current->mm.sp, current->mm.base, current->mm.heap,
		current->mm.kernel.base, current->mm.kernel.sp, current->state,
		current->irqflag, current->addr, current);

	printk("\ncurrent context\n");
	unsigned int i;
	for (i = 0; i < NR_CONTEXT*2; i++)
		printk("[0x%08x] 0x%08x\n", usp + i*4, ((unsigned int *)usp)[i]);

	printk("\nSCB_ICSR  0x%08x\n"
		"SCB_CFSR  0x%08x\n"
		"SCB_HFSR  0x%08x\n"
		"SCB_MMFAR 0x%08x\n"
		"SCB_BFAR  0x%08x\n",
		SCB_ICSR, SCB_CFSR, SCB_HFSR, SCB_MMFAR, SCB_BFAR);

	/* led for debugging */
#ifdef LED_DEBUG
	SET_PORT_CLOCK(ENABLE, PORTD);
	SET_PORT_PIN(PORTD, 2, PIN_OUTPUT_50MHZ);
	unsigned int j;
	while (1) {
		PUT_PORT(PORTD, GET_PORT(PORTD) ^ 4);

		for (i = 100; i; i--) {
			for (j = 10; j; j--) {
				__asm__ __volatile__(
						"nop		\n\t"
						"nop		\n\t"
						"nop		\n\t"
						"nop		\n\t"
						"nop		\n\t"
						"nop		\n\t"
						"nop		\n\t"
						"nop		\n\t"
						"nop		\n\t"
						"nop		\n\t"
						::: "memory");
			}
		}
	}
#endif

	while (1);
}
