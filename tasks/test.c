#include <foundation.h>
#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/systick.h>

#include <string.h>
#include <stdlib.h>
static void test_clcd()
{
	int fd;
	if ((fd = open("/dev/clcd", O_RDWR)) <= 0) {
		printf("clcd: open error %x\n", fd);
		return;
	}

	unsigned int v;
	char *buf;

	if ((buf = malloc(128)) == NULL) {
		printf("malloc(): error\n");
		return;
	}

	while (1) {
		v = systick;
		sprintf(buf, "0x%08x %d (%d sec) ", v, v, v/HZ);
		write(fd, buf, strlen(buf));

		sleep(1);
	}

	write(fd, "Hello, World!", 13);
	sleep(5);
	write(fd, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
	stdout = fd;
}
REGISTER_TASK(test_clcd, 0, DEFAULT_PRIORITY);

static void test_usart2()
{
	int fd;

	if ((fd = open("/dev/usart2", O_RDWR | O_NONBLOCK)) <= 0) {
		printf("usart2: open error %x\n", fd);
		return;
	}

	int buf, ret;

	while (1) {
		if ((ret = read(fd, &buf, 1)))
			putchar(buf);
	}
}
REGISTER_TASK(test_usart2, 0, DEFAULT_PRIORITY);

static volatile int flag;
static void t1()
{
	printf("TIMER1\n");
	flag = 1;
}

static void test_timer()
{
	struct timer ttt;
	while (1) {
		flag = 0;
		ttt.expires = systick + msec_to_ticks(500);
		ttt.event = t1;
		add_timer(&ttt);

		msleep(900);
		while (flag == 0);
	}
}
REGISTER_TASK(test_timer, 0, DEFAULT_PRIORITY);

static void test_write()
{
	int fd;
	char *buf = "01234567890123456789012345678901234567890123456789"
		"01234567890123456789012345678901234567890123456789"
		"0123456789012345678901234567";

	sleep(3);
	printk("\nstart write test #2 %x %x\n", current, current->addr);
	if ((fd = open("/test_write", O_RDWR)) == -ERR_PATH) {
		if ((fd = open("/test_write", O_CREATE | O_RDWR)) < 0) {
			printk("FAILED to create #2\n");
			goto out;
		}

		int i;
		for (i = 0; i < 50; i++) {
			if (!write(fd, buf, 128))
			//if (write(fd, buf, 32) <= 0)
				break;
			printf("=> %d written\n", (i+1) * 128);
		}
	}

	close(fd);

out:
	printf("\nend write test #2\n");
}
REGISTER_TASK(test_write, 0, DEFAULT_PRIORITY);

static void test_task()
{
	unsigned long long t;
	unsigned int th, bh;
	unsigned int v;

	int fd;
	char *buf = "abcdefABCDEFabcdefABCDEFabcdefABCDEFabcdefABCDEFabcdef"
		"ABCDEFabcdabcdefABCDEFabcdefABCDEFabcdefABCDEFabcdefABCDEF"
		"abcdefABCDEFabcd";

	sleep(3);
	printk("\nstart write test %x %x\n", current, current->addr);
	//if ((fd = open("/test_write", O_CREATE | O_RDWR)) <= 0) {
	if ((fd = open("/test_write", O_RDWR)) == -ERR_PATH) {
		if ((fd = open("/test_write", O_CREATE | O_RDWR)) < 0) {
			printk("FAILED to create\n");
			goto out;
		}

		int i;
		//for (i = 0; i < 5000; i++) {
		for (i = 0; i < 50; i++) {
			if (write(fd, buf, 128) <= 0)
				break;
			printf("%d written\n", (i+1) * 128);
		}
	}

	close(fd);
out:

	if ((fd = open("/dev_overlap", O_CREATE)) > 0)
		close(fd);
	printf("\nend write test\n");

	printf("test()\n");
	printf("sp : %x, lr : %x\n", __get_sp(), __get_lr());

	while (1) {
		t = get_systick64();
		th = t >> 32;
		bh = t;

		dmb();
		v = systick;

		printf("%08x ", v);
		printf("%08x", th);
		printf("%08x %d (%d sec)\n", bh, v, v/HZ);

		sleep(1);
	}
}
REGISTER_TASK(test_task, 0, DEFAULT_PRIORITY);

static void test_led()
{
#if 1
	float a = -0.33947753;
	float b = 0.93957519;

	sleep(2);
	printf("%x %#x %#08x %#8x %-#8x %-#08x\n", &a, &a, &a, &a, &a, &a);
	printf("%x %#x %#06x %#6x %-#6x %-#06x\n", &a, &a, &a, &a, &a, &a);
	printf("f   {x: %f, y: %f}\n", (float)123456789, (float)123456789);
	printf("10f {x: %10f, y: %10f}\n", (float)123456789, (float)123456789);
	printf("010f{x: %010f, y: %010f}\n", (float)123456789, (float)123456789);
	printf("f   {x: %f, y: %f}\n", (float)1234, (float)1234);
	printf("8f  {x: %8f, y: %-8f}\n", (float)1234, (float)1234);
	printf("08f {x: %08f, y: %-08f}\n", (float)1234, (float)1234);
	printf("f   {x: %f, y: %f}\n", (float)-1234, (float)-1234);
	printf("8f  {x: %8f, y: %-8f}\n", (float)-1234, (float)-1234);
	printf("08f {x: %08f, y: %-08f}\n", (float)-1234, (float)-1234);
	printf(".2  {x: %.2f, y: %.2f}\n", (float)1234, (float)1234);
	printf(".2  {x: %.2f, y: %.2f}\n", (float)0, (float)0);
	printf("f   {x: %f, y: %f}\n", a, b);
	printf("8.2 {x: %8.2f, y: %8.2f}\n", a, b);
	printf("8.  {x: %8.f, y: %8.f}\n", a, b);
	printf("08f {x: %08f, y: %08f}\n", a, b);
	printf("08.2{x: %08.2f, y: %08.2f}\n", a, b);
	printf(".2  {x: %.2f, y: %.2f}\n", a, b);
	printf(".8  {x: %.8f, y: %.8f}\n", a, b);
	printf(".08 {x: %.08f, y: %.08f}\n", a, b);
	printf("-8.2{x: %-8.2f, y: %-8.2f}\n", a, b);
	printf("-8. {x: %-8.f, y: %-8.f}\n", a, b);
	printf("-08f{x: %-08f, y: %-08f}\n", a, b);
	printf("08.2{x: %-08.2f, y: %-08.2f}\n", a, b);
	printf("-.2 {x: %-.2f, y: %-.2f}\n", a, b);
	printf("-08f{x: %-08f, y: %-08f}\n", (float)1234, (float)1234);
	printf("-8f {x: %-8f, y: %-8f}\n", (float)1234, (float)1234);
	printf("-8.2{x: %-8.2f, y: %-8.2f}\n", (float)1234, (float)1234);
	sleep(1);
#endif

	unsigned int v = 0;
	int fd;

	if ((fd = open("/dev/led", O_RDWR)) <= 0) {
		printf("led open error %x\n", fd);
		return;
	}

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

	sleep(5);
	printf("REBOOT %x\n", shutdown(2));
}
REGISTER_TASK(mayfly, 0, DEFAULT_PRIORITY);

static void fork_test()
{
	int tid;

	while (1) {
		tid = fork();

		if (tid == 0) { /* parent */
			write(stdout, "fork_test: parent\r\n", 19);
		} else if (tid > 0) { /* child */
			write(stdout, "fork_test: child\r\n", 18);
			return;
		} else { /* error */
			printf("fork_test: error\n");
		}

		sleep(2);
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

		getfree();
		show_freelist(&current->mm.heap);
#ifdef CONFIG_SYSCALL
		display_devtab();
#endif
		sleep(3);
		printf("REALTIME END\n");
		yield();
	}
}
REGISTER_TASK(rt_task1, TASK_USER, RT_PRIORITY);
