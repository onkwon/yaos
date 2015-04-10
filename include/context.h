#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#define CONTEXT_NR	(17)
#define CONTEXT_SIZE	(CONTEXT_NR * sizeof(long)) /* depending on SCB_CCR[STKALIGN] */

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
	);					\
	__asm__ __volatile__("mov %0, sp" : "=&r"(sp));

#define context_restore(sp)			\
	__asm__ __volatile__(			\
		"mov	sp, %0		\n\t"	\
		"pop 	{lr}		\n\t"	\
		"pop 	{r4-r11}	\n\t"	\
		:: "r"(sp)			\
	);

#endif /* __CONTEXT_H__ */
