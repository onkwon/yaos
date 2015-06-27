#include <foundation.h>
#include "shell.h"
#include <kernel/task.h>

static void visit(struct task *p)
{
	struct task *next;
	static int tab = 0;

	int i;
#define print_tab() for (i = 0; i < tab; i++) puts("|\t");

	print_tab();
	printf("+-- 0x%x 0x%02x 0x%02x %d\n", p->addr,
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
		return;

	tab++;

	next = get_container_of(p->children.next, struct task, sibling);
	visit(next);
	p = next;

	while (p->sibling.next != &p->parent->children) {
		next = get_container_of(
				p->sibling.next, struct task, sibling);
		visit(next);
		p = next;
	}

	tab--;

	printf("control %08x, sp %08x, msp %08x, psp %08x\n",
			GET_CON(), GET_SP(), GET_KSP(), GET_USP());
}

#ifdef CONFIG_PAGING
#include <kernel/buddy.h>
#endif

static int ps(int argc, char **argv)
{
	printf("    ADDR  TYPE STAT PRI\n");

	visit(&init);

#ifdef CONFIG_PAGING
	extern struct buddy buddypool;
	printf("%d pages free out of %d pages\n",
			buddypool.nr_free, buddypool.nr_pages);
#endif

#ifdef CONFIG_DEBUG
#define MHZ	1000000
#define FREQ	9 /* (get_systick_max() * HZ / MHZ) --> privileged */
	extern int sched_overhead;
	printf("scheduling overhead %dus / %dus (%d)\n",
			sched_overhead / FREQ, MHZ / HZ, sched_overhead);
#endif

	return 0;
}
REGISTER_CMD(ps, ps, "report process status");
