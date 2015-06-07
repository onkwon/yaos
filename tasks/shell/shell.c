#include "shell.h"
#include <string.h>
#include <stdlib.h>
#include <foundation.h>

#define MAXLEN		128
#define MAXARG		10

static unsigned int getline(char *buf, int maxlen)
{
	char c;
	int i = 0;

	while ((c = getc()) != '\r') {
		switch (c) {
		case (char)-1: /* no input */
		case 0x1b: /* extended ascii code */
		case '\t':
			break;
		case '\b':
			if(i){
				puts("\b \b");
				i--;
			}
			break;
		default:
			if(i < maxlen - 1){
				buf[i++] = c;
				putchar(c);
			}
			break;
		}
	}

	buf[i] = '\0';
	putchar('\n');

	return i;
}

static int getcmd(char *s, char **argv)
{
	int i;

	for (i = 0; s && *s; i++) {
		while (*s == ' ') s++;
		argv[i] = s;
		s       = strchr(s, ' ');
		if (s) *s++ = '\0';
	}

	return i;
}

#include <kernel/task.h>

void shell()
{
	int argc;
	char **argv = (char **)malloc(sizeof(char *) * MAXARG);

	char buf[MAXLEN];
	int exit_code = 0;

	puts("Type `help` for help on commands.\r\n");

	do {
		puts("> ");
		if (getline(buf, MAXLEN)) {
			/* add history functionality */
			if (!(argc = getcmd(buf, argv))) continue;

			extern char _shell_cmdlist;
			struct shell_cmd *cmd = (struct shell_cmd *)&_shell_cmdlist;

			for (exit_code = 0; cmd->name; cmd++) {
				if (!strncmp(cmd->name, argv[0], MAXLEN)) {
					if ((exit_code = cmd->run(argc, argv))) {
						printf("usage: %s\n", cmd->usage);
					}
					break;
				}
			}

                        if(!cmd->name) puts("unknown command\r\n");
		}
	} while(exit_code != SHELL_EXIT);

	free(argv);
}
REGISTER_TASK(shell, 0, DEFAULT_PRIORITY);
