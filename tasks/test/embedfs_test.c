#include <foundation.h>
#include <kernel/task.h>
#include <kernel/timer.h>

#define REPEAT		50
#define BUFSIZE		128
#define FILENAME1	"/test_write"
#define FILENAME2	"/dev_overlap"

static void test_write()
{
	int fd;
	char *buf = "01234567890123456789012345678901234567890123456789"
		"01234567890123456789012345678901234567890123456789"
		"0123456789012345678901234567";

	sleep(3);
	notice("start write test #2 %x %x", current, current->addr);
	if ((fd = open(FILENAME1, O_RDWR)) == -ERR_PATH) {
		debug("create %s", FILENAME1);
		if ((fd = open(FILENAME1, O_CREATE | O_RDWR)) < 0) {
			error("FAILED to create #2");
			goto out;
		}

		debug("writing in %s", FILENAME1);
		int i;
		for (i = 0; i < REPEAT; i++) {
			if (!write(fd, buf, BUFSIZE))
				break;

			printf("=> %d written\n", (i+1) * BUFSIZE);
		}
	}

	close(fd);

out:
	notice("end write test #2");
}
REGISTER_TASK(test_write, 0, DEFAULT_PRIORITY);

static void test_task()
{
	int fd;
	char *buf = "abcdefABCDEFabcdefABCDEFabcdefABCDEFabcdefABCDEFabcdef"
		"ABCDEFabcdabcdefABCDEFabcdefABCDEFabcdefABCDEFabcdefABCDEF"
		"abcdefABCDEFabcd";

	sleep(3);
	notice("start write test %x %x", current, current->addr);
	if ((fd = open(FILENAME1, O_RDWR)) == -ERR_PATH) {
		debug("create2 %s", FILENAME1);
		if ((fd = open(FILENAME1, O_CREATE | O_RDWR)) < 0) {
			error("FAILED to create");
			goto out;
		}

		debug("writing2 in %s", FILENAME1);
		int i;
		for (i = 0; i < REPEAT; i++) {
			if (write(fd, buf, BUFSIZE) <= 0)
				break;
			printf("%d written\n", (i+1) * BUFSIZE);
		}
	}

	close(fd);
out:

	if ((fd = open(FILENAME2, O_CREATE)) > 0)
		close(fd);
	notice("end write test");
}
REGISTER_TASK(test_task, 0, DEFAULT_PRIORITY);
