#ifndef __ARMv7M_INTERRUPT_H__
#define __ARMv7M_INTERRUPT_H__

#define IRQ_MAX			128

/* nvector : actual hardware vector number
 * lvector : logical vector number (primary + secondary)
 * nirq    : actual hardware interrupt number */

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

#define SECONDARY_IRQ_BITS	16
/* primary irq   : actual hardware interrupt number
 * secondary irq : logical interrupt number used by kernel itself
 *                 it would be a pin number+1 typically */
enum {
	PRIMARY_IRQ_MAX		= IRQ_MAX,
	SECONDARY_IRQ_MIN	= PRIMARY_IRQ_MAX,
	/* make sure MSB(sign bit) is reserved as using `int` data type */
	SECONDARY_IRQ_MAX	= (1 << (SECONDARY_IRQ_BITS - 1)),
};

#define mkvector(p, s)		((p) | (((s) + 1) << SECONDARY_IRQ_BITS))
#define get_primary_vector(x)	((x) & ((1 << SECONDARY_IRQ_BITS) - 1UL))
#define get_secondary_vector(x)	(((x) >> SECONDARY_IRQ_BITS) - 1)

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

#define __dmb()			__asm__ __volatile__("dmb" ::: "memory")
#define __dsb()			__asm__ __volatile__("dsb" ::: "memory")
#define __isb()			__asm__ __volatile__("isb" ::: "memory")

#define __irq_save(flag)						\
	__asm__ __volatile__("mrs %0, primask" : "=r"(flag) :: "memory")
#define __irq_restore(flag)						\
	__asm__ __volatile__(						\
			"msr primask, %0	\n\t"			\
			"isb			\n\t"			\
			:: "r"(flag) : "cc", "memory")

#define __get_active_irq()	(__get_psr() & 0x1ff)
#define __in_interrupt()	__get_active_irq()

int register_isr(int lvector, void (*handler)(int));
int register_isr_register(int lvector, int (*f)(int, void (*)(int)), bool force);
int unregister_isr(int lvector);
void nvic_set(int nirq, int on);
void nvic_set_pri(int nirq, int pri);

#endif /* __ARMv7M_INTERRUPT_H__ */
