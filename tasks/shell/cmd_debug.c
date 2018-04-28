#include "shell.h"
#include <foundation.h>
#include <kernel/debug.h>
#include <string.h>
#include <stdlib.h>

static int inspect(int argc, char **argv)
{
	if (argc != 2)
		return -1;

	struct task *task;
	unsigned int addr = (unsigned int)strtol(argv[1], NULL, 16);

	task = (struct task *)addr;

	print_task_status(task);
	print_user_status(task->mm.sp);
	print_context(task->mm.sp);

	return 0;
}
REGISTER_CMD(dbg, inspect, "dbg {0xaddr}");
