#include <foundation.h>
#include <shell.h>
#include <kernel/task.h>

static void visit(struct task_t *p)
{
	struct task_t *next;
	static int tab = 0;

	int i;
#define print_tab() for (i = 0; i < tab; i++) puts("|\t");

	print_tab();
	printf("+-- 0x%x 0x%02x %d\n",
			p->addr, get_task_state(p), get_task_priority(p));
	print_tab();
	printf("|   /vruntime %d /exec_runtime %d (%d sec)\n",
			(unsigned)p->se.vruntime,
			(unsigned)p->se.sum_exec_runtime,
			(unsigned)p->se.sum_exec_runtime / HZ);
	print_tab();
	printf("|   /sp 0x%08x /base 0x%08x /heap 0x%08x /size %d\n",
			p->mm.sp, p->mm.base, p->mm.heap, STACK_SIZE);
	print_tab();
	printf("|   /kernel stack 0x%08x\n", p->mm.kernel);
	print_tab();
	printf("|\n");

	if (list_empty(&p->children))
		return;

	tab++;

	next = get_container_of(p->children.next, struct task_t, sibling);
	visit(next);
	p = next;

	while (p->sibling.next != &p->parent->children) {
		next = get_container_of(
				p->sibling.next, struct task_t, sibling);
		visit(next);
		p = next;
	}

	tab--;
}

#ifdef CONFIG_PAGING
#include <kernel/buddy.h>
#endif

static int ps(int argc, char **argv)
{
	printf("    ADDR  STAT PRI\n");

	visit(&init);

#ifdef CONFIG_PAGING
	extern struct buddypool_t buddypool;
	printf("%d pages free out of %d pages\n",
			buddypool.nr_free, buddypool.nr_pages);
#endif

	return 0;
}
REGISTER_CMD(ps, ps, "report process status");
