#include <foundation.h>
#include <string.h>
#include "shell.h"

const char *rname1[] = { "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
		"r0", "r1", "r2", "r3", "r12", "lr", "pc", "psr" };

static inline void print_context(unsigned int *regs)
{
	unsigned int i;

	for (i = 0; i < NR_CONTEXT; i++)
		printf("  %s\t 0x%08x <%08x>\n", rname1[i], &regs[i], regs[i]);
}

static inline void print_task_status(struct task *task)
{
	printf("  task->sp	%08x\n", task->mm.sp);
	printf("  task->base	%08x\n", task->mm.base);
	printf("  task->heap	%08x\n", task->mm.heap);
	printf("  task->kernel	%08x\n", task->mm.kernel.base);
	printf("  task->ksp	%08x\n", task->mm.kernel.sp);
	printf("  task->state	%08x\n", task->state);
	printf("  task->irqflag %08x\n", task->irqflag);
	printf("  task->addr	%08x\n", task->addr);
	printf("  task		%08x\n", task);
	printf("  parent	%08x\n", task->parent);
	printf("  parent->addr	%08x\n", task->parent->addr);
}

static inline void print_kernel_status(unsigned int *sp, unsigned int lr,
		unsigned int psr)
{
	printf("  kernel  SP	%08x\n", sp);
	printf("  stacked PSR	%08x\n", sp[7]);
	printf("  stacked PC	%08x\n", sp[6]);
	printf("  stacked LR	%08x\n", sp[5]);
	printf("  current LR	%08x\n", lr);
	printf("  current PSR	%08x(vector number:%d)\n", psr, psr & 0x1ff);
}

static inline void print_user_status(unsigned int *sp)
{
	printf( "  user SP	%08x\n"
		"  stacked PSR	%08x\n"
		"  stacked PC	%08x\n"
		"  stacked LR	%08x\n",
		sp, sp[7], sp[6], sp[5]);
}

static int inspect(int argc, char **argv)
{
	if (argc != 2)
		return -1;

	struct task *task;
	unsigned int addr = atoi(argv[1]);

	task = (struct task *)addr;

	print_task_status(task);
	print_user_status(task->mm.sp);
	print_context(task->mm.sp);

	return 0;
}
REGISTER_CMD(dbg, inspect, "dbg {addr}");
