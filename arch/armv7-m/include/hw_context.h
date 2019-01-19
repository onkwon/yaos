#ifndef __YAOS_HW_CONTEXT_H__
#define __YAOS_HW_CONTEXT_H__

#include "regs.h"
#include "types.h"

/* raise pendsv for scheduling */
#define hw_raise_sched()		(SCB_ICSR |= 1UL << 28)

#define NR_CONTEXT_SOFT			9 /* r4-r11, EXC_RETURN */
#define NR_CONTEXT_HARD			8 /* r0-r3, r12, lr, pc, psr */
#define NR_CONTEXT			(NR_CONTEXT_HARD + NR_CONTEXT_SOFT)
#define CONTEXT_SIZE			(NR_CONTEXT * WORD_SIZE)
/* depending on SCB_CCR[STKALIGN] */

#define INITIAL_IRQFLAG			0

#define EXC_RETURN_MSPH			0xfffffff1UL /* return to HANDLER mode using MSP */
#define EXC_RETURN_MSPT			0xfffffff9UL /* return to THREAD  mode using MSP */
#define EXC_RETURN_PSPT			0xfffffffdUL /* return to THREAD  mode using PSP */
#define EXC_RETURN_MSPH_FP		0xffffffe1UL /* return to HANDLER mode using MSP with floating-point-state */
#define EXC_RETURN_MSPT_FP		0xffffffe9UL /* return to THREAD  mode using MSP with floating-point-state */
#define EXC_RETURN_PSPT_FP		0xffffffedUL /* return to THREAD  mode using PSP with floating-point-state */

#define DEFAULT_PSR			0x01000000UL

#define INDEX_R0			9
#define INDEX_PSR			16

/*  ___________
 * | FPSCR     | (if fpu used)
 * | s0 - s15  |
 *  -----------
 * | psr       |  |
 * | pc        |  | stack
 * | lr        |  |
 * | r12       |  v
 * | r3        |
 * | r2        |
 * | r1        |
 * | r0        |
 *  -----------
 * | exc_ret   |
 * | s16 - s31 | (if fpu used)
 * | r4 - r11  |
 *  -----------
 */
struct regs {
	uintptr_t r4; /* low address */
	uintptr_t r5;
	uintptr_t r6;
	uintptr_t r7;
	uintptr_t r8;
	uintptr_t r9;
	uintptr_t r10;
	uintptr_t r11;
	/* s16 ~ s31 */
	uintptr_t exc_return;
	uintptr_t r0;
	uintptr_t r1;
	uintptr_t r2;
	uintptr_t r3;
	uintptr_t r12;
	uintptr_t lr;
	uintptr_t pc;
	uintptr_t psr;
	/* s0 ~ s15
	 * FPSCR */
};

#include "kernel/task.h"

void set_task_context_hard(struct task *p, void *addr);
void set_task_context_soft(struct task *p);
void set_task_context(struct task *p, void *addr);

#endif /* __YAOS_HW_CONTEXT_H__ */
