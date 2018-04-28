#include "shell.h"
#include <foundation.h>
#include <kernel/sched.h>
#include <kernel/task.h>
#include <string.h>
#include <stdlib.h>

static int run(int argc, char **argv)
{
	if (argc != 2)
		return -1;

	struct task *p;
	unsigned int addr = (unsigned int)strtol(argv[1], NULL, 16);

	p = find_task(addr, &init);

	if (!p) {
		printf("0x%x: no such a task\n", addr);
		return -1;
	}

	set_task_state(p, TASK_RUNNING);
	runqueue_add(p); /* make a system call for this */
	schedule();

	return 0;
}
REGISTER_CMD(run, run, "run 0xADDR");

static int stop(int argc, char **argv)
{
	if (argc != 2)
		return -1;

	struct task *p;
	unsigned int addr = (unsigned int)strtol(argv[1], NULL, 16);

	p = find_task(addr, &init);

	if (!p) {
		printf("0x%x: no such a task\n", addr);
		return -1;
	}

	set_task_state(p, TASK_STOPPED);

	return 0;
}
REGISTER_CMD(stop, stop, "stop 0xADDR");
