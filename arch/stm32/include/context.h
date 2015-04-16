#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#define CONTEXT_NR	(17)
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
#define context_save(sp)			\
	__asm__ __volatile__(			\
		"push	{r4-r11}	\n\t"	\
		"push	{lr}		\n\t"	\
	::: "memory");					\
	__asm__ __volatile__("mov %0, sp" : "=&r"(sp) :: "memory");

#define context_restore(sp)			\
	__asm__ __volatile__(			\
		"mov	sp, %0		\n\t"	\
		"pop 	{lr}		\n\t"	\
		"pop 	{r4-r11}	\n\t"	\
		:: "r"(sp) : "memory"		\
	);

#define schedule()	__asm__ __volatile__("svc 0");

#endif /* __CONTEXT_H__ */
