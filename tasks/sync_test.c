#include <foundation.h>
#include <driver/usart.h>

static int shared;
DEFINE_MUTEX(lock_shared);

int get_shared()
{
	int t;

	mutex_lock(lock_shared);

	printf("LOCKED (%d)\n", lock_shared.count);
	t = shared;

	mutex_unlock(lock_shared);
	printf("UNLOCKED (%d)\n", t);

	return t;
}

void set_shared(int v)
{
	mutex_lock(lock_shared);

	printf("LOCKED (%d)\n", lock_shared.count);
	shared = v;
	msleep(100);

	mutex_unlock(lock_shared);
	printf("UNLOCKED\n");
}

static void shared_test()
{
	while (1) {
		set_shared(1);
		sleep(5);
	}
}

#include <kernel/task.h>
REGISTER_TASK(shared_test, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);

static void shared_test2()
{
	while (1) {
		set_shared(4);
		sleep(5);
	}
}

REGISTER_TASK(shared_test2, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);

static void shared_test3()
{
	while (1) {
		set_shared(5);
		sleep(5);
	}
}

REGISTER_TASK(shared_test3, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);
