#include <foundation.h>
#include <kernel/task.h>

static void test_task()
{
	unsigned long long t;
	unsigned int th, bh;
	unsigned int v;

	printf("start write test\n");
	int fd;
	char *buf = "abcdefABCDEF";
	fd = open("/test_write", O_CREATE | O_RDWR);
	write(fd, buf, 12);
	close(fd);
	printf("end write test\n");

	printf("test()\n");
	printf("sp : %x, lr : %x\n", GET_SP(), GET_LR());

	while (1) {
		t = get_jiffies_64();
		th = t >> 32;
		bh = t;

		dmb();
		v = jiffies;

		printf("%08x ", v);
		printf("%08x", th);
		printf("%08x %d (%d sec)\n", bh, v, v/HZ);

		sleep(1);
	}
}
REGISTER_TASK(test_task, 0, DEFAULT_PRIORITY);

static void test_led()
{
	unsigned int v = 0;
	int fd;

	fd = open("/dev/led", O_RDWR);

	while (1) {
		write(fd, &v, 1);
		read(fd, &v, 1);
		printf("led %08x\n", v);
		v ^= 1;
		sleep(1);
	}
}
REGISTER_TASK(test_led, 0, DEFAULT_PRIORITY);

static void mayfly()
{
	printf("onetime function\n");
}
REGISTER_TASK(mayfly, 0, DEFAULT_PRIORITY);

static void fork_test()
{
	int tid;

	while (1) {
		tid = fork();

		if (tid == 0) { /* parent */
			printf("fork_test: parent\n");
		} else if (tid > 0) { /* child */
			printf("fork_test: child\n");
			return;
		} else { /* error */
			printf("fork_test: error\n");
		}

		sleep(1);
	}
}
REGISTER_TASK(fork_test, 0, DEFAULT_PRIORITY);

#include <kernel/page.h>
#include <kernel/device.h>
#include <lib/firstfit.h>
static void rt_task1()
{
	while (1) {
		printf("REALTIME START\n");
#ifdef CONFIG_PAGING
	show_buddy(NULL);
#endif
	show_freelist(current->mm.heap);
#ifdef CONFIG_SYSCALL
	display_devtab();
#endif
		sleep(3);
		printf("REALTIME END\n");
		yield();
	}
}
REGISTER_TASK(rt_task1, TASK_USER, RT_LEAST_PRIORITY);
