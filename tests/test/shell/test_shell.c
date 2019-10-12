#include "unity.h"

#include "shell.h"

#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#define WORD_SIZE		sizeof(long)

extern void shell(void);

static int exit(int argc, char **argv)
{
	return SHELL_EXIT;
	(void)argc;
	(void)argv;
}
REGISTER_CMD(exit, exit, "exit");

static int test(int argc, char **argv)
{
	return 0;
	(void)argc;
	(void)argv;
}
REGISTER_CMD(test, test, "test");

struct shell_cmd *_shell_cmdlist[10] = {
	&shell_cmd_exit,
	&shell_cmd_test,
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

static int mygetc(void)
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

static void myputc(int c)
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

	update_rx_stream("exit\r");
	update_tx_stream(buf);

	shell_set(mygetc, myputc);
	shell();
	TEST_ASSERT(strncmp(buf, RC_STR_SHELL_WELCOME,
				strlen(RC_STR_SHELL_WELCOME)) == 0);
}

void test_shell_unknown_command(void)
{
	char buf[1024];

	update_rx_stream("qwergfs\rexit\r");
	update_tx_stream(buf);

	shell_set(mygetc, myputc);
	shell();
	TEST_ASSERT(strncmp(strstr(buf, RC_STR_SHELL_UNKNOWN),
				RC_STR_SHELL_UNKNOWN,
				strlen(RC_STR_SHELL_UNKNOWN)) == 0);
}

void test_shell_command_maxlen(void)
{
	char buf[1024];
	char *cmd = "12345678901234567890123456789012345678901234567890"
		"12345678901234567890123456789012345678901234567890"
		"12345678901234567890123456789012345678901234567890"
		"\rexit\r";

	update_rx_stream(cmd);
	update_tx_stream(buf);

	shell_set(mygetc, myputc);
	shell();
	TEST_ASSERT( (strlen(buf)
				- strlen(RC_STR_SHELL_WELCOME)
				- strlen(RC_STR_SHELL_UNKNOWN)) <
			(strlen(cmd)) );
}
