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
	mdelay(100);

	mutex_unlock(lock_shared);
	printf("UNLOCKED\n");
}

static void shared_test()
{
	while (1) {
		set_shared(1);
		mdelay(7);
	}
}

#include <task.h>
REGISTER_TASK(shared_test, STACK_SIZE_DEFAULT, NORMAL_PRIORITY);

static void shared_test2()
{
	while (1) {
		set_shared(4);
		mdelay(8);
	}
}

REGISTER_TASK(shared_test2, STACK_SIZE_DEFAULT, NORMAL_PRIORITY);

static void shared_test3()
{
	while (1) {
		set_shared(5);
		mdelay(9);
	}
}

REGISTER_TASK(shared_test3, STACK_SIZE_DEFAULT, NORMAL_PRIORITY);
