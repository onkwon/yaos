#include "shell.h"
#include "types.h"

static int exit(int argc, char **argv)
{
	return SHELL_EXIT;
	(void)argc;
	(void)argv;
}
REGISTER_CMD(exit, exit, "exit");
