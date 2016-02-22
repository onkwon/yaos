#include <foundation.h>
#include "shell.h"
#include <kernel/task.h>

static unsigned int visit(struct task *p, unsigned int nr)
{
	struct task *next;
	static int tab = 0;

	unsigned int i;
#define print_tab() for (i = 0; i < tab; i++) puts("|\t");

	print_tab();
	printf("+-- 0x%08x(0x%08x) 0x%02x 0x%02x %d\n", p, p->addr,
			get_task_type(p), get_task_state(p), get_task_pri(p));
	print_tab();
	printf("|   /vruntime %d /exec_runtime %d (%d sec)\n",
			(unsigned)p->se.vruntime,
			(unsigned)p->se.sum_exec_runtime,
			(unsigned)p->se.sum_exec_runtime / HZ);
	print_tab();
	printf("|   /sp 0x%08x /base 0x%08x /heap 0x%08x /size %d\n",
			p->mm.sp, p->mm.base, p->mm.heap, STACK_SIZE);
	print_tab();
	printf("|   /kernel stack 0x%08x base 0x%08x\n", p->mm.kernel.sp, p->mm.kernel.base);
	print_tab();
	printf("|\n");

	if (list_empty(&p->children))
		return nr + 1;

	tab++;

	next = get_container_of(p->children.next, struct task, sibling);
	nr   = visit(next, nr);
	p    = next;

	while (p->sibling.next != &p->parent->children) {
		next = get_container_of(
				p->sibling.next, struct task, sibling);
		nr   = visit(next, nr);
		p    = next;
	}

	tab--;

	return nr;
}

#include <kernel/page.h>
#include <kernel/systick.h>
#include <kernel/timer.h>

static int ps(int argc, char **argv)
{
	printf("    ADDR                   TYPE STAT PRI\n");

	printf("%d tasks running out of %d\n",
			get_nr_running() + 1, /* including the current task */
			visit(&init, 1)); /* count from the init task */

	printf("%d bytes free\n", getfree() * PAGE_SIZE);

#ifdef CONFIG_DEBUG
	extern unsigned int alloc_fail_count;
	printf("Memory allocation errors : %d\n", alloc_fail_count);
#endif

	printf("%d timer(s) activated\n", get_timer_nr());

#ifdef CONFIG_DEBUG_SCHED
#define MHZ	1000000
#define FREQ	9 /* (get_systick_max() * HZ / MHZ) --> privileged */
	extern int sched_overhead;
	printf("scheduling overhead %dus / %dus (%d)\n",
			sched_overhead / FREQ, MHZ / HZ, sched_overhead);
#endif

#ifdef CONFIG_DEBUG_CLONE
#define MHZ	1000000
#define FREQ	9 /* (get_systick_max() * HZ / MHZ) --> privileged */
	extern int clone_overhead;
	printf("cloning overhead %dus / %dus (%d)\n",
			clone_overhead / FREQ, MHZ / HZ, clone_overhead);
#endif

	unsigned long long uptime = get_systick64();
	printf("uptime: %d minutes (0x%08x%08x)\n"
			, systick / HZ / 60
			, (unsigned int)(uptime >> 32)
			, (unsigned int)uptime);

#ifdef CONFIG_DEBUG
	printf("control %08x, sp %08x, msp %08x, psp %08x\n",
			GET_CNTL(), GET_SP(), GET_KSP(), GET_USP());
#endif

	extern void print_rq();
	printf("\nRun queue list:\n");
	print_rq();

#ifdef CONFIG_DEBUG
	extern unsigned int get_usart_bufover();
	printf("\nUSART buffer overflow: %d, %d, %d, %d, %d\n",
			get_usart_bufover(0),
			get_usart_bufover(1),
			get_usart_bufover(2),
			get_usart_bufover(3),
			get_usart_bufover(4));
#endif

	return 0;
}
REGISTER_CMD(ps, ps, "report process status");
