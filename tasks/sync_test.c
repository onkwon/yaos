#include <foundation.h>
#include <kernel/task.h>

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
		get_shared();
		sleep(5);
	}
}
REGISTER_TASK(shared_test, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);

static void shared_test2()
{
	while (1) {
		printf("control %08x, sp %08x, msp %08x, psp %08x\n", GET_CON(), GET_SP(), GET_KSP(), GET_USP());
		set_shared(4);
		get_shared();
		sleep(5);
	}
}
REGISTER_TASK(shared_test2, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);

static void shared_test3()
{
	while (1) {
		set_shared(5);
		get_shared();
		sleep(5);
	}
}
REGISTER_TASK(shared_test3, DEFAULT_STACK_SIZE, DEFAULT_PRIORITY);
