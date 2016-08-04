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
#include <kernel/timer.h>

static int runcmd(struct shell_cmd *cmd, int argc, char **argv)
{
	int ret;

	ret = cmd->run(argc, argv);

	if (ret) {
		printf("usage: %s\n", cmd->usage);
	}

	return ret;
}

/* TODO: add history functionality */
void shell()
{
	extern char _shell_cmdlist;
	int argc, ret, len;
	char **argv, buf[MAXLEN];
	struct shell_cmd *cmd, *prev;

	argv = (char **)malloc(sizeof(char *) * MAXARG);
	argc = 0;
	ret = 0;
	prev = NULL;

	sleep(1); /* not to be mixed with other init messages */

	/* TODO: Separate resource like string below,
	 * in the way of categorized by locale */
	printf("Type `help` for help on commands.\n");

	do {
		puts("> ");
		if ((len = getline(stdin, buf, MAXLEN))) {
			if (len == 1 && buf[0] == '/' &&
					argc && prev && prev->name) {
				ret = runcmd(prev, argc, argv);
				continue;
			} else if (!(argc = getcmd(buf, argv)))
				continue;

			for (cmd = (struct shell_cmd *)&_shell_cmdlist;
					cmd->name; cmd++) {
				if (!strncmp(cmd->name, argv[0], MAXLEN)) {
					ret = runcmd(cmd, argc, argv);
					prev = cmd;
					break;
				}
			}

			if(!cmd->name) {
				puts("unknown command\n");
			}
		}
	} while(ret != SHELL_EXIT);

	free(argv);
}
REGISTER_TASK(shell, 0, DEFAULT_PRIORITY);
