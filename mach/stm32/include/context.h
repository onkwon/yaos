#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#define CONTEXT_NR	(16)
#define CONTEXT_SIZE	(CONTEXT_NR * sizeof(long)) /* depending on SCB_CCR[STKALIGN] */

#define EXC_RETURN_MSPH	0xfffffff1	/* return to HANDLER mode using MSP */
#define EXC_RETURN_MSPT	0xfffffff9	/* return to THREAD  mode using MSP */
#define EXC_RETURN_PSPT	0xfffffffd	/* return to THREAD  mode using PSP */

/* We ignore CONTROL register that controls which stack to use
 * and the privilege level. Suppose we work only in privileged mode.
 *  __________
 * | psr      |  |
 * | pc       |  | stack
 * | lr       |  |
 * | r12      |  v
 * | r3       |
 * | r2       |
 * | r1       |
 * | r0       |
 *  ----------
 * | r4 - r11 |
 * | lr       |
 *  ----------
 */
#define context_save(sp)				\
	__asm__ __volatile__(				\
			"mrs	r12, psp	\n\t"	\
			"tst	lr, #4		\n\t"	\
			"it	eq		\n\t"	\
			"mrseq	r12, msp	\n\t"	\
			"stmdb	r12!, {r4-r11}	\n\t"	\
			"mov	%0, r12		\n\t"	\
			: "=&r"(sp) :: "memory")

#define context_restore(sp)				\
	__asm__ __volatile__(				\
			"ldmia	%0!, {r4-r11}	\n\t"	\
			"tst	%1, %2		\n\t"	\
			"bne	1f		\n\t"	\
			"ldr	lr, =0xfffffffd	\n\t"	\
			"msr	psp, %0		\n\t"	\
			"mov	r3, #1		\n\t"	\
			"msr	control, r3	\n\t"	\
			"b	2f		\n\t"	\
		"1:"	"ldr	lr, =0xfffffff9	\n\t"	\
			"msr	msp, %0		\n\t"	\
			"mov	r3, #0		\n\t"	\
			"msr	control, r3	\n\t"	\
		"2:"					\
			:: "r"(sp),			\
			   "r"(get_task_state(current)),\
			   "I"(TASK_KERNEL)		\
			: "memory"			\
	)

#include <kernel/task.h>

extern inline void init_task_context(struct task_t *p);

#endif /* __CONTEXT_H__ */
