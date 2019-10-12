#include "shell.h"
#include <stdio.h>
#include <string.h>

#define MAX_BUFSIZE			128

extern char _shell_cmdlist;

static void (*putch)(int c);
static int (*getch)(void);

static inline void putstr(const char * const str)
{
	for (size_t i = 0; i < strnlen(str, 256); i++)
		putch(str[i]);
}

static int help(int argc, char **argv)
{
	char buf[MAX_BUFSIZE];
	struct shell_cmd *cmd = (void *)&_shell_cmdlist;

	getch = (void *)argv[argc];
	putch = (void *)argv[argc+1];

	while (cmd && cmd->name) {
		snprintf(buf, MAX_BUFSIZE, "%s\t\t%s\r\n",
				cmd->name, cmd->usage);
		putstr(buf);

		cmd++;
	}

	return 0;
}
REGISTER_CMD(help, help, "display this information");
