#ifndef __ARMv7M_INTERRUPT_H__
#define __ARMv7M_INTERRUPT_H__

#define irq2vec(nirq)		((nirq) + 16)
#define vec2irq(nvec)		((nvec) - 16)

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

int register_isr(unsigned int nvector, void (*func)());
void nvic_set(unsigned int nirq, int on);
void nvic_set_pri(unsigned int nirq, unsigned int pri);

#endif /* __ARMv7M_INTERRUPT_H__ */
