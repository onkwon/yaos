#include "unity.h"

#include "shell.h"
#include "cmd_dump.h"

#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

extern void shell(void);

static int exit(int argc, char **argv)
{
	return SHELL_EXIT;
	(void)argc;
	(void)argv;
}
static REGISTER_CMD(exit, exit, "exit");

static int test(int argc, char **argv)
{
	return 0;
	(void)argc;
	(void)argv;
}
static REGISTER_CMD(test, test, "test");

struct shell_cmd *_shell_cmdlist[10] = {
	&shell_cmd_exit,
	&shell_cmd_test,
	&shell_cmd_md,
	NULL,
};

static int fd;

void setUp(void)
{
	fd = open("/dev/stdin", O_NONBLOCK);
	assert(fd > 0);
}

void tearDown(void)
{
	close(fd);
}

static char *rx_stream, *tx_stream;
static int rxi, txi;

static void update_rx_stream(char *new)
{
	rx_stream = new;
	rxi = 0;
}

static void update_tx_stream(char *new)
{
	tx_stream = new;
	txi = 0;
}

static int getch(void)
{
	if (rx_stream) {
		if (rxi >= strlen(rx_stream))
			return -1;
		return (int)rx_stream[rxi++];
	}

	char c;
	int rc;

	rc = read(fd, &c, 1);
	if (rc < 0)
		return rc;

	return (int)c;
}

static void putch(int c)
{
	if (tx_stream) {
		tx_stream[txi++] = (char)c;
		return;
	}

	putchar(c);
}

void test_shell_set(void)
{
	char buf[1024];
	char cmd[80];

	sprintf(cmd, "md %p\rmd %p 64\rmd\rexit\r", buf, buf);

	update_rx_stream(cmd);
	//update_tx_stream(buf);
	update_tx_stream(NULL);

	shell_set(getch, putch);
	shell();
}
