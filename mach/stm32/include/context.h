#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#define NR_CONTEXT_SOFT			8
#define NR_CONTEXT_HARD			8
#define NR_CONTEXT			(NR_CONTEXT_HARD + NR_CONTEXT_SOFT)
#define CONTEXT_SIZE			(NR_CONTEXT * WORD_SIZE)
/* depending on SCB_CCR[STKALIGN] */

#define INIT_IRQFLAG(flag)		((flag) = 0)

#define EXC_RETURN_MSPH	0xfffffff1	/* return to HANDLER mode using MSP */
#define EXC_RETURN_MSPT	0xfffffff9	/* return to THREAD  mode using MSP */
#define EXC_RETURN_PSPT	0xfffffffd	/* return to THREAD  mode using PSP */

/*  __________
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
#define __context_save(task)	do {					\
	__asm__ __volatile__(						\
			"mrs	r12, psp		\n\t"		\
			"stmdb	r12!, {r4-r11}		\n\t"		\
			::: "memory");					\
	__asm__ __volatile__(						\
			"mov	%0, r12			\n\t"		\
			: "=&r"(task->mm.sp)				\
			:: "memory");					\
	dsb();								\
} while (0)

/* When writing to the CONTROL register, an ISB instruction should be used to
 * ensure the new configuration is used in subsequent instructions’ operations.
 * AN321 - ARM Cortex-M Programming Guide to Memory Barrier Instructions */
#define __context_restore(task)	do {					\
	__asm__ __volatile__(						\
			"msr	msp, %0			\n\t"		\
			"mov	r12, #3			\n\t"		\
			"tst	%1, %2			\n\t"		\
			"it	ne			\n\t"		\
			"movne	r12, #2			\n\t"		\
			:: "r"(task->mm.kernel.sp)			\
			, "r"(get_task_flags(task))			\
			, "I"(TASK_PRIVILEGED)				\
			: "memory");					\
	__asm__ __volatile__(						\
			"ldmia	%0!, {r4-r11}		\n\t"		\
			"msr	psp, %0			\n\t"		\
			"ldr	lr, =0xfffffffd		\n\t"		\
			"msr	control, r12		\n\t"		\
			:: "r"(task->mm.sp)				\
			: "r12", "memory");				\
	dsb();								\
	isb();								\
} while (0)

#define __context_prepare()				cli()
#define __context_finish()				sei()

#define INDEX_PSR					15
#define INDEX_PC					14
#define INDEX_R0					8
#define INDEX_R4					0
#define INIT_PSR					0x01000000

#define __save_curr_context(regs) do {					\
	__asm__ __volatile__(						\
			"str	r4, [%0]		\n\t"		\
			"ldr	r4, =1f			\n\t"		\
			"str	r4, [%1]		\n\t"		\
			"mov	r4, #0x01000000		\n\t"		\
			"str	r4, [%2]		\n\t"		\
			"mov	r4, %3			\n\t"		\
			"stmia	r4!, {r5-r11}		\n\t"		\
			"stmia	r4!, {r0-r3}		\n\t"		\
			"stmia	r4!, {r12,lr}		\n\t"		\
			"1:				\n\t"		\
			:: "r"(regs+0), "r"(regs+INDEX_PC)		\
			, "r"(regs+INDEX_PSR), "r"(regs+1)		\
			: "memory", "r4");				\
} while (0)

#define __set_retval(task, value)					\
	__asm__ __volatile__("str %0, [%1]" :				\
			: "r"(value), "r"(task->mm.sp + INDEX_R0))

#endif /* __CONTEXT_H__ */
