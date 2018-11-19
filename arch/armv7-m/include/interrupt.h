#ifndef __YAOS_ARMv7M_INTERRUPT_H__
#define __YAOS_ARMv7M_INTERRUPT_H__

/* nvec : exception number defined in ARMv7-M exception model
 * lvec : logical vector number (primary + secondary)
 * nirq : hardware interrupt number, starts from NVECTOR_IRQ */

enum {
	NVECTOR_RESET		= 1,
	NVECTOR_NMI		= 2,
	NVECTOR_HARDFAULT	= 3,
	NVECTOR_MEMMANAGE	= 4,
	NVECTOR_BUSFAULT	= 5,
	NVECTOR_USAGEFAULT	= 6,
	NVECTOR_SVC		= 11,
	NVECTOR_DEBUGMONITOR	= 12,
	NVECTOR_PENDSV		= 14,
	NVECTOR_SYSTICK		= 15,
	NVECTOR_MAX		= 16,
	NVECTOR_IRQ		= NVECTOR_MAX,
};

#define irq2vec(nirq)		((nirq) + NVECTOR_IRQ)
#define vec2irq(nvec)		((nvec) - NVECTOR_IRQ)

#define IRQ_MAX			128

#define SECONDARY_IRQ_BITS	10U
/* primary irq   : actual hardware interrupt number
 * secondary irq : logical interrupt number used by kernel itself
 *                 typically it would be a pin number+1 */
enum {
	PRIMARY_IRQ_MAX		= IRQ_MAX,
	SECONDARY_IRQ_MIN	= PRIMARY_IRQ_MAX,
	/* make sure MSB(sign bit) is reserved as using `int` data type */
	SECONDARY_IRQ_MAX	= (1UL << (SECONDARY_IRQ_BITS - 1)),
};

#define mkvector(p, s)		(((unsigned int)(s) > SECONDARY_IRQ_MAX) \
		? 0 : (p) | (((s) + 1) << SECONDARY_IRQ_BITS))
#define get_primary_vector(x)	((x) & ((1U << SECONDARY_IRQ_BITS) - 1U))
#define get_secondary_vector(x)	(((x) >> SECONDARY_IRQ_BITS) - 1)

#if !defined(TEST)
/* isb should be used after cpsie/cpsid to catch the pended interrupt. Or it
 * may execute up to two instructions before catching the interrupt. Refer to:
 * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHBFEIB.html
 *
 * the situation that entering a critical section while an interrupt is pending
 * is prevented as isb is put after cpsie. so we doesn't need another barrier
 * after cpsid.
 */
#define __sei()								\
	__asm__ __volatile__(						\
			"cpsie i	\n\t"				\
			"isb		\n\t"				\
			::: "cc", "memory")
#define __cli()								\
	__asm__ __volatile__("cpsid i" ::: "cc", "memory")

#define __irq_save(flag)						\
	__asm__ __volatile__("mrs %0, primask" : "=r"(flag) :: "memory")
#define __irq_restore(flag)						\
	__asm__ __volatile__(						\
			"msr primask, %0	\n\t"			\
			"isb			\n\t"			\
			:: "r"(flag) : "cc", "memory")
#endif /* !defined(TEST) */

#include <stdbool.h>

int register_isr(const int lvec, void (*handler)(const int));
int register_isr_register(const int nvec,
		int (*f)(const int, void (*)(const int)), const bool force);
int unregister_isr(const int lvec);

void nvic_set(const int nvec, const bool on);
void nvic_set_pri(const int nvec, const int pri);

void irq_init(void);
void ISR_reset(void);

void __reboot(void);

#endif /* __YAOS_ARMv7M_INTERRUPT_H__ */
