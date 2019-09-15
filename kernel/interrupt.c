#include "kernel/interrupt.h"
#include "arch/atomic.h"
#include "io.h"
#include "syslog.h"

#include <errno.h>
#include <stdlib.h>

#if defined(CONFIG_COMMON_IRQ_HANDLER)
void (*primary_isr_table[PRIMARY_IRQ_MAX - NVECTOR_IRQ])(const int);
#endif

#if defined(CONFIG_COMMON_IRQ_HANDLER)
static int register_isr_core(const int lvec, void (*handler)(const int))
{
	int nvec = abs(lvec);

	if (nvec < NVECTOR_IRQ)
		return -EACCES;

	if (nvec >= PRIMARY_IRQ_MAX)
		return -ERANGE;

	uintptr_t *p = (uintptr_t *)&primary_isr_table[nvec - NVECTOR_IRQ];

	do {
		void (*f)(void) = (void (*)(void))ldrex(p);

		if (((uintptr_t)f != (uintptr_t)ISR_null)
				&& ((uintptr_t)handler != (uintptr_t)ISR_null))
			return -EEXIST;
	} while (strex(handler, p));

	return 0;
}
#else /* !CONFIG_COMMON_IRQ_HANDLER */
extern uintptr_t _ram_start;

static int register_isr_core(const int lvec, void (*handler)(const int))
{
	int nvec = abs(lvec);

	if (nvec < NVECTOR_IRQ)
		return -EACCES;

	if (nvec >= PRIMARY_IRQ_MAX)
		return -ERANGE;

	uintptr_t *p = (uintptr_t *)&_ram_start;

	p += nvec;

	do {
		void (*f)(int) = (void (*)(int))ldrex(p);

		if (((uintptr_t)f != (uintptr_t)ISR_irq)
				&& ((uintptr_t)handler != (uintptr_t)ISR_irq))
			return -EEXIST;
	} while (strex(handler, p));

	return 0;
}
#endif /* CONFIG_COMMON_IRQ_HANDLER */

int register_isr(const int lvec, void (*handler)(const int))
{
	int ret;

	if (!is_honored())
		return -EPERM;

	if (!handler)
		handler = (void (*)(const int))ISR_irq;

	if (abs(lvec) >= PRIMARY_IRQ_MAX)
		return -ERANGE;

	ret = register_isr_core(lvec, handler);

	if (ret >= 0) {
		dsb();
		isb();
	}

	return ret;
}

int unregister_isr(const int lvec)
{
	int ret;

	if (!is_honored())
		return -EPERM;

	if (abs(lvec) >= PRIMARY_IRQ_MAX)
		return -ERANGE;

	ret = register_isr_core(lvec, (void (*)(const int))ISR_irq);

	if (ret >= 0) {
		dsb();
		isb();
	}

	return ret;
}

#include "kernel/init.h"

void __init irq_init(void)
{
#if defined(CONFIG_COMMON_IRQ_HANDLER)
	for (int i = 0; i < (PRIMARY_IRQ_MAX - NVECTOR_IRQ); i++)
		primary_isr_table[i] = ISR_null;
#endif

	hw_irq_init();

	dsb();
	isb();
}
