#include <foundation.h>
#include <shell.h>

#include <string.h>
#include <kernel/sched.h>
#include <kernel/task.h>

static int run(int argc, char **argv)
{
	if (argc != 2)
		return -1;

	struct task_t *p;
	unsigned long addr = atoi(argv[1]);

	p = find_task(addr, &init);

	if (!p) {
		printf("0x%x: no such a task\n", addr);
		return -1;
	}

	set_task_state(p, TASK_RUNNING);
	runqueue_add(p);
	schedule();

	return 0;
}
REGISTER_CMD(run, run, "run ADDR");

static int stop(int argc, char **argv)
{
	if (argc != 2)
		return -1;

	struct task_t *p;
	unsigned long addr = atoi(argv[1]);

	p = find_task(addr, &init);

	if (!p) {
		printf("0x%x: no such a task\n", addr);
		return -1;
	}

	reset_task_state(p, TASK_RUNNING);

	return 0;
}
REGISTER_CMD(stop, stop, "stop ADDR");
