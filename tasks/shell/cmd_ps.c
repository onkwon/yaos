#include <foundation.h>
#include "shell.h"
#include <kernel/task.h>
#include <kernel/systick.h>

static unsigned int visit(struct task *p, unsigned int nr)
{
	struct task *next;
	static unsigned int tab = 0;

	unsigned int i;
#define print_tab() for (i = 0; i < tab; i++) printk("|\t");

	print_tab();
	printk("+-- 0x%08x(0x%08x) 0x%02x 0x%02x %d %s[%c]\n",
			p, p->addr, get_task_type(p), get_task_state(p),
			get_task_pri(p), p->name,
			get_task_flags(p) & TF_PRIVILEGED? 'P':'U');
	print_tab();
	printk("|   /vruntime %d /exec_runtime %d (%d sec)\n",
			(unsigned)p->se.vruntime,
			(unsigned)p->se.sum_exec_runtime,
			(unsigned)p->se.sum_exec_runtime / sysfreq);
	print_tab();
	printk("|   /sp 0x%08x /base 0x%08x /heap 0x%08x\n",
			p->mm.sp, p->mm.base, p->mm.heap);
	print_tab();
	printk("|   /kernel stack 0x%08x base 0x%08x\n", p->mm.kernel.sp, p->mm.kernel.base);
	print_tab();
	printk("|\n");

	if (links_empty(&p->children))
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

		if (p->sibling.next == &next->sibling)
			break;
	}

	tab--;

	return nr;
}

#include <kernel/page.h>
#include <kernel/power.h>
#include <kernel/timer.h>
#include <kernel/softirq.h>

static int ps(int argc, char **argv)
{
	unsigned int i;

	printk("    ADDR                   TYPE STAT PRI NAME\n");

	printk("%d tasks running out of %d\n",
			nr_running() + 1, /* including the current task */
			visit(&init, 1)); /* count from the init task */

	printk("%d bytes free\n", getfree());

	printk("%d timer(s) activated\n", get_timer_nr());

#ifdef CONFIG_DEBUG_SCHED
#define MHZ	1000000
#define FREQ	9 /* (get_systick_max() * HZ / MHZ) --> privileged */
	extern int sched_overhead;
	printk("scheduling overhead %dus / %dus (%d)\n",
			sched_overhead / FREQ, MHZ / sysfreq, sched_overhead);
#endif

#ifdef CONFIG_DEBUG_CLONE
#define MHZ	1000000
#define FREQ	9 /* (get_systick_max() * HZ / MHZ) --> privileged */
	extern int clone_overhead;
	printk("cloning overhead %dus / %dus (%d)\n",
			clone_overhead / FREQ, MHZ / sysfreq, clone_overhead);
#endif

	unsigned long long uptime = get_systick64();
	printk("uptime: %d minutes (0x%08x%08x)\n"
			, systick / sysfreq / 60
			, (unsigned int)(uptime >> 32)
			, (unsigned int)uptime);

#ifdef CONFIG_DEBUG
	printk("control %08x, sp %08x, msp %08x, psp %08x\n",
			__get_cntl(), __get_sp(), __get_ksp(), __get_usp());
	printk("cpu load: %d%%\n", cpuload);
#endif
	for (i = 0; i < SOFTIRQ_MAX; i++) {
		if (softirq.bitmap & (1 << i))
			printk("softirq overrun[%02d]: %d\n",
					i, softirq.pool[i].overrun);
	}

#ifndef CONFIG_PAGING
	extern unsigned int mem_bottom;
	printk("memory bottom %d\n", mem_bottom);
#endif

#ifdef CONFIG_DEBUG_SYSCALL
	extern unsigned int syscall_count;
	printk("syscall: %d times called\n", syscall_count);
#endif

#if 0
	extern void print_rq();
	printk("\nRun queue list:\n");
	print_rq();
#endif

	(void)argc;
	(void)argv;
	return 0;
}
REGISTER_CMD(ps, ps, "report process status");
