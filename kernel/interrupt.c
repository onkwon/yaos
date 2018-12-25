#include "kernel/interrupt.h"
#include "arch/atomic.h"
#include "io.h"
#include "syslog.h"

#include <errno.h>
#include <stdlib.h>

extern uintptr_t _ram_start;

#if defined(CONFIG_COMMON_IRQ_FRAMEWORK)
void (*primary_isr_table[PRIMARY_IRQ_MAX - NVECTOR_IRQ])(const int);
#endif
static int (*secondary_isr_table[PRIMARY_IRQ_MAX - NVECTOR_IRQ])(const int, void (*)(const int));

#if defined(CONFIG_COMMON_IRQ_FRAMEWORK)
static int register_isr_primary(const int lvec, void (*handler)(const int))
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
#else /* !CONFIG_COMMON_IRQ_FRAMEWORK */
static int register_isr_primary(const int lvec, void (*handler)(const int))
{
	int nvec = abs(lvec);

	if (nvec < NVECTOR_IRQ)
		return -EACCES;

	if (nvec >= PRIMARY_IRQ_MAX)
		return -ERANGE;

	uintptr_t *p = (uintptr_t *)&_ram_start;

	p += nvec - NVECTOR_IRQ;

	do {
		void (*f)(int) = (void (*)(int))ldrex(p);

		if (((uintptr_t)f != (uintptr_t)ISR_null)
				&& ((uintptr_t)handler != (uintptr_t)ISR_null))
			return -EEXIST;
	} while (strex(handler, p));

	return 0;
}
#endif /* CONFIG_COMMON_IRQ_FRAMEWORK */

/* NOTE: calling multiple handlers is possible chaining handlers to a list,
 * which sounds flexible. but I don't think it would be useful since such use
 * cases don't look nice but causing latency and complexity. */
int register_isr_register(const int nvec,
		int (*ctor)(const int, void (*)(const int)), const bool force)
{
	if (!is_honored())
		return -EPERM;

	if (abs(nvec) < NVECTOR_IRQ)
		return -EACCES;

	if (abs(nvec) >= PRIMARY_IRQ_MAX)
		return -ERANGE;

	uintptr_t *p = (uintptr_t *)&secondary_isr_table[nvec - NVECTOR_IRQ];

	do {
		void (*f)(void) = (void (*)(void))ldrex(p);

		if (!force && f) {
			debug("already exist or no room");
			return -EEXIST;
		}
	} while (strex(ctor, p));

	return 0;
}

static int register_isr_secondary(const int lvec, void (*handler)(const int))
{
	int nvec = get_primary_vector(lvec);

	if (nvec < NVECTOR_IRQ)
		return -EACCES;

	if ((nvec >= PRIMARY_IRQ_MAX)
			|| (get_secondary_vector(lvec) >= SECONDARY_IRQ_MAX))
		return -ERANGE;

	int (*f)(const int, void (*)(const int));

	if ((f = secondary_isr_table[nvec - NVECTOR_IRQ]))
		return f(lvec, handler);

	debug("no irq register for %d:%d", nvec, get_secondary_vector(lvec));

	return -ENOENT;
}

int register_isr(const int lvec, void (*handler)(const int))
{
	int ret;

	if (!is_honored())
		return -EPERM;

	if (!handler)
		handler = ISR_null;

	if (abs(lvec) < PRIMARY_IRQ_MAX)
		ret = register_isr_primary(lvec, handler);
	else
		ret = register_isr_secondary(lvec, handler);

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

	if (abs(lvec) < PRIMARY_IRQ_MAX) {
		if ((ret = register_isr_primary(lvec, ISR_null)) == 0) {
			/* unregister all the secondaries of it */
			for (int i = 0; i < SECONDARY_IRQ_MAX; i++)
				register_isr_secondary(mkvector(lvec, i), ISR_null);

			register_isr_register(lvec, NULL, 1);
		}
	} else {
		ret = register_isr_secondary(lvec, ISR_null);
	}

	if (ret >= 0) {
		dsb();
		isb();
	}

	return ret;
}

#include "kernel/init.h"

void __init irq_init(void)
{
#if defined(CONFIG_COMMON_IRQ_FRAMEWORK)
	for (int i = 0; i < (PRIMARY_IRQ_MAX - NVECTOR_IRQ); i++)
		primary_isr_table[i] = ISR_null;
#endif

	hw_irq_init();

	for (int i = 0; i < (PRIMARY_IRQ_MAX - NVECTOR_IRQ); i++)
		secondary_isr_table[i] = NULL;

	dsb();
	isb();
}
