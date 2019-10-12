#include "shell.h"
#include "heap.h"
#include "compiler.h"
#include "kernel/timer.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define MAXLEN		128
#define MAXARG		10

#if defined(TEST)
struct shell_cmd *_shell_cmdlist[];
#else
extern char _shell_cmdlist;
#endif

static void (*putc_callback)(int c);
static int (*getc_callback)(void);

static inline void shell_putc(const char c)
{
	if (!putc_callback)
		return;

	putc_callback((int)c);
}

static inline void shell_puts(const char *str, ...)
{
	for (int i = 0; str[i]; i++)
		shell_putc(str[i]);
}

static unsigned int readline(int (*rxc)(void), char *buf, unsigned int maxlen)
{
	int c;
	unsigned int i = 0;

	if (!rxc) {
		/* go sleep not to waste cpu power doing nothing */
		sleep(1);
		return 0;
	}

	do {
		while ((c = rxc()) <= 0);

		switch ((char)c) {
		case (char)-1: /* no input */
		case 0x1b: /* extended ascii code */
		case '\t':
		case '\r':
		case '\n':
			break;
		case '\b':
			if(i){
				shell_puts("\b \b");
				i--;
			}
			break;
		default:
			if(i < maxlen - 1){
				buf[i++] = c;
				shell_putc((char)c);
			}
			break;
		}
	} while ((char)c != '\r' && (char)c != '\n');

	buf[i] = '\0';
	shell_putc('\n');

	return i;
}

static int getcmd(char *s, char **argv)
{
	int i;

	for (i = 0; s && *s; i++) {
		while (*s == ' ') s++;

		argv[i] = s;
		s = strchr(s, ' ');

		if (s) {
			*s++ = '\0';
		}
	}

	argv[i] = (void *)getc_callback;
	argv[i+1] = (void *)putc_callback;

	return i;
}

static int runcmd(struct shell_cmd *cmd, int argc, char **argv)
{
	int rc;

	rc = cmd->run(argc, argv);

	if (rc) {
		shell_puts(cmd->usage);
	}

	return rc;
}

/* TODO: add history functionality */
STATIC void shell(void)
{
	struct shell_cmd *cmd, *prev;
	char **argv, *buf;
	int argc, rc, len;

	buf = malloc(sizeof(char) * MAXLEN);
	argv = (char **)malloc(sizeof(char *) * MAXARG);
	argc = rc = 0;
	prev = NULL;

	assert(buf);
	assert(argv);

	shell_puts("Type 'help' for help on commands.\n");

	do {
		shell_puts("> ");
		len = readline(getc_callback, buf, MAXLEN);
		if (!len)
			continue;

		if (len == 1 && buf[0] == '.' &&
				argc && prev && prev->name) {
			rc = runcmd(prev, argc, argv);
			continue;
		} else if (!(argc = getcmd(buf, argv))) {
			continue;
		}

#if defined(TEST)
		for (cmd = _shell_cmdlist[0]; cmd->name; cmd++) {
#else
		for (cmd = (void *)&_shell_cmdlist; cmd->name; cmd++) {
#endif
			if (!strncmp(cmd->name, argv[0], MAXLEN)) {
				rc = runcmd(cmd, argc, argv);
				prev = cmd;
				break;
			}
		}

		if(!cmd->name) {
			shell_puts("unknown command\n");
		}
	} while(rc != SHELL_EXIT);

	free(argv);
}
#include "kernel/task.h"
REGISTER_TASK(shell, TASK_USER, TASK_PRIORITY_NORMAL, STACK_SIZE_DEFAULT);

void shell_set(int (*rxc)(void), void (*txc)(int c))
{
	getc_callback = (volatile void *)rxc;
	putc_callback = (volatile void *)txc;
}
