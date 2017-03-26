#include <foundation.h>
#include <string.h>
#include <kernel/debug.h>
#include "shell.h"

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
