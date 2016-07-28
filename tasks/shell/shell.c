#include "shell.h"
#include <string.h>
#include <stdlib.h>
#include <foundation.h>

#define MAXLEN		128
#define MAXARG		10

static unsigned int getline(int fd, char *buf, int maxlen)
{
	char c;
	unsigned int i = 0;

	do {
		while (read(fd, &c, 1) <= 0);

		switch (c) {
		case (char)-1: /* no input */
		case 0x1b: /* extended ascii code */
		case '\t':
		case '\r':
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
	} while (c != '\r');

	buf[i] = '\0';
	putchar('\n');

	return i;
}

static unsigned int getcmd(char *s, char **argv)
{
	unsigned int i;

	for (i = 0; s && *s; i++) {
		while (*s == ' ') s++;
		argv[i] = s;
		s       = strchr(s, ' ');
		if (s) *s++ = '\0';
	}

	return i;
}

#include <kernel/task.h>
#include <kernel/device.h>

void shell()
{
	int argc, ret;
	char **argv, buf[MAXLEN];

	argv = (char **)malloc(sizeof(char *) * MAXARG);
	ret = 0;

	write(stdout, "Type `help` for help on commands.\r\n", 35);

	do {
		puts("> ");
		if (getline(stdin, buf, MAXLEN)) {
			/* add history functionality */
			if (!(argc = getcmd(buf, argv))) continue;

			extern char _shell_cmdlist;
			struct shell_cmd *cmd = (struct shell_cmd *)&_shell_cmdlist;

			for (ret = 0; cmd->name; cmd++) {
				if (!strncmp(cmd->name, argv[0], MAXLEN)) {
					if ((ret = cmd->run(argc, argv))) {
						printf("usage: %s\n", cmd->usage);
					}
					break;
				}
			}

			if(!cmd->name) puts("unknown command\n");
		}
	} while(ret != SHELL_EXIT);

	free(argv);
}
REGISTER_TASK(shell, 0, DEFAULT_PRIORITY);
