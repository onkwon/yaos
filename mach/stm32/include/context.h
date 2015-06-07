#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#define NR_CONTEXT_SOFT		8
#define NR_CONTEXT_HARD		8
#define NR_CONTEXT		(NR_CONTEXT_HARD + NR_CONTEXT_SOFT)
#define CONTEXT_SIZE		(NR_CONTEXT * WORD_SIZE)
/* depending on SCB_CCR[STKALIGN] */

#define INIT_IRQFLAG(flag)	((flag) = 0)

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
 *  ----------
 */
#define context_save(task)				\
	__asm__ __volatile__(				\
			"mrs	r12, psp	\n\t"	\
			"stmdb	r12!, {r4-r11}	\n\t"	\
			"mov	%0, r12		\n\t"	\
			: "=&r"(task->mm.sp)		\
			:: "memory")

#define context_restore(task)				\
	__asm__ __volatile__(				\
			"ldmia	%0!, {r4-r11}	\n\t"	\
			"msr	psp, %0		\n\t"	\
			"ldr	lr, =0xfffffffd	\n\t"	\
			"mov	r3, #3		\n\t"	\
			"tst	%1, %2		\n\t"	\
			"it	ne		\n\t"	\
			"movne	r3, #2		\n\t"	\
			"msr	control, r3	\n\t"	\
			"msr	msp, %3		\n\t"	\
			:: "r"(task->mm.sp),		\
			   "r"(get_task_type(task)),	\
			   "I"(TASK_KERNEL),		\
			   "r"(task->mm.kernel)		\
			: "memory")

void sys_schedule();

#include <kernel/task.h>

extern inline void set_task_context(struct task *p);
extern inline void set_task_context_soft(struct task *p);
extern inline void set_task_context_hard(struct task *p);

#endif /* __CONTEXT_H__ */
