#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#define ARMv7A

#define NR_CONTEXT_SOFT			0
#define NR_CONTEXT_HARD			17
#define NR_CONTEXT			(NR_CONTEXT_HARD + NR_CONTEXT_SOFT)
#define CONTEXT_SIZE			(NR_CONTEXT * WORD_SIZE)

#define INIT_IRQFLAG(flag)		((flag) = 0)

/*  __________
 * | r15(pc)  |  |
 * | r14(lr)  |  |
 * | r12      |  |
 * | r11      |  | stack
 * | r10      |  |
 * | r9       |  |
 * | r8       |  v
 * | r7       |
 * | r6       |
 * | r5       |
 * | r4       |
 * | r3       |
 * | r2       |
 * | r1       |
 * | r13(sp)  |
 * | r0       |
 * | cpsr     |
 *  ----------
 */
struct regs {
	unsigned int cpsr;
	unsigned int r0;
	unsigned int sp;
	unsigned int r1;
	unsigned int r2;
	unsigned int r3;
	unsigned int r4;
	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;
	unsigned int r10;
	unsigned int r11;
	unsigned int r12;
	unsigned int lr;
	unsigned int pc;
};

#define __context_save(task)	do {					\
	__asm__ __volatile__(						\
			"push	{r0}				\n\t"	\
			"stmdb	sp, {sp}^			\n\t"	\
			"sub	sp, sp, #4			\n\t"	\
			"ldr	r0, [sp]			\n\t"	\
			"str	lr, [r0, #-4]!	@ pc		\n\t"	\
			"stmdb	r0, {lr}^	@ lr		\n\t"	\
			"sub	r0, r0, #4			\n\t"	\
			"stmdb	r0!, {r1-r12}			\n\t"	\
			"pop	{r1}		@ sp		\n\t"	\
			"str	r1, [r0, #-4]!			\n\t"	\
			"pop	{r1}		@ r0		\n\t"	\
			"str	r1, [r0, #-4]!			\n\t"	\
			"mrs	r1, spsr	@ cpsr		\n\t"	\
			"str	r1, [r0, #-4]!			\n\t"	\
			::: "memory");					\
	__asm__ __volatile__(						\
			"mov	%0, r0				\n\t"	\
			: "=&r"(task->mm.sp)				\
			:: "memory");					\
} while (0)

#define __context_restore(task)	do {					\
	__asm__ __volatile__(						\
			"mov	r12, %0				\n\t"	\
			"ldr	r0, [r12], #4			\n\t"	\
			"tst	%1, %2				\n\t"	\
			"orrne	r0, #0x1f			\n\t"	\
			:: "r"(task->mm.sp)				\
			, "r"(get_task_flags(task))			\
			, "I"(TASK_PRIVILEGED)				\
			: "memory");					\
	__asm__ __volatile__(						\
			"msr	spsr, r0			\n\t"	\
			"ldr	r0, [r12], #4			\n\t"	\
			"ldmia	r12, {sp}^			\n\t"	\
			"add	r12, r12, #4			\n\t"	\
			"ldmia	r12!, {r1-r11}			\n\t"	\
			/* return to user mode */			\
			"ldr	lr, [r12, #8]			\n\t"	\
			"ldmia	r12, {r12, lr}^			\n\t"	\
			::: "memory");					\
} while (0)

#define __context_prepare()
#define __context_finish()

#define INDEX_PSR			0
#define INDEX_PC			16
#define INDEX_R0			1
#define INDEX_R1			3
#define INDEX_SP			2

#define DEFAULT_PSR			0x10
#define DEFAULT_PSR_SYS			0x1f

#define __save_curr_context(regs) do {					\
	__asm__ __volatile__(						\
			"str	r0, [%0]			\n\t"	\
			"mov	r0, %4				\n\t"	\
			"stmia	r0!, {r1-r12, lr}		\n\t"	\
			"mrs	r0, cpsr			\n\t"	\
			"str	r0, [%1]			\n\t"	\
			"ldr	r0, =1f				\n\t"	\
			"str	r0, [%2]			\n\t"	\
			"mov	r0, %3				\n\t"	\
			"stmia	r0, {sp}			\n\t"	\
		"1:						\n\t"	\
			:: "r"(regs+INDEX_R0)				\
			, "r"(regs+INDEX_PSR)				\
			, "r"(regs+INDEX_PC)				\
			, "r"(regs+INDEX_SP)				\
			, "r"(regs+INDEX_R1)				\
			: "r0", "memory");				\
} while (0)

#define __set_retval(task, value)					\
	__asm__ __volatile__("str %0, [%1]" :				\
			: "r"(value), "r"(task->mm.sp + INDEX_R0))

#endif /* __CONTEXT_H__ */
