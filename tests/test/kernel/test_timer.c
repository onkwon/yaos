#include "unity.h"

#include "kernel/timer.h"

#include "mock_systick.h"
#include "mock_sched.h"
#include "mock_task.h"
#include "mock_heap.h"
#include "mock_syscall.h"

void setUp(void)
{
	current = &init_task;
	timer_init();
}

void tearDown(void)
{
}

struct task *current, init_task;
static unsigned int done;
void do_nothing(void) { done++; }

void test_timer_create_invalid(void)
{
	void (*dummy_callback)(void) = do_nothing;

	TEST_ASSERT(timer_create_core(0, NULL, 0) == -EINVAL);
	TEST_ASSERT(timer_create_core(0, dummy_callback, 0) == -EINVAL);
	TEST_ASSERT(timer_create_core(0, dummy_callback, TIMER_REPEAT) == -EINVAL);
	TEST_ASSERT(timer_create_core(1, NULL, 0) == -EINVAL);
	TEST_ASSERT(timer_create_core(1, NULL, TIMER_REPEAT) == -EINVAL);
	TEST_ASSERT(timer_create_core(1, dummy_callback, 0) == -EINVAL);
}

#define TEST_TIMER_CREATE_100		100

void test_timer_create_once(void)
{
	uintptr_t timer[TEST_TIMER_CREATE_100];

	free_to_Ignore();

	for (int i = 0; i < TEST_TIMER_CREATE_100; i++) {
		get_systick_ExpectAndReturn(0);
		timer[i] = timer_create_core(i+1, do_nothing, 1);
		TEST_ASSERT(timer[i] != -EINVAL);
		TEST_ASSERT(timer[i] != -ENOMEM);
		TEST_ASSERT(timer[i] > 0);
	}
	for (int i = 0; i < TEST_TIMER_CREATE_100*TIMER_MAX_RERUN; i++) {
		timer_handler(i+1);
	}
	TEST_ASSERT(done == TEST_TIMER_CREATE_100);
}

void test_timer_create_several(void)
{
	uintptr_t timer[TEST_TIMER_CREATE_100];

	free_to_Ignore();

	done = 0;
	for (int i = 0; i < TEST_TIMER_CREATE_100; i++) {
		get_systick_ExpectAndReturn(0);
		timer[i] = timer_create_core(i+1, do_nothing, 2);
		TEST_ASSERT(timer[i] != -EINVAL);
		TEST_ASSERT(timer[i] != -ENOMEM);
		TEST_ASSERT(timer[i] > 0);
	}
	for (int i = 0; i < TEST_TIMER_CREATE_100*TIMER_MAX_RERUN; i++) {
		timer_handler(i+1);
	}
	TEST_ASSERT(done == TEST_TIMER_CREATE_100 * 2);

	done = 0;
	for (int i = 0; i < TEST_TIMER_CREATE_100; i++) {
		get_systick_ExpectAndReturn(0);
		timer[i] = timer_create_core(i+1, do_nothing, 3);
		TEST_ASSERT(timer[i] != -EINVAL);
		TEST_ASSERT(timer[i] != -ENOMEM);
		TEST_ASSERT(timer[i] > 0);
	}
	for (int i = 0; i < TEST_TIMER_CREATE_100*TIMER_MAX_RERUN; i++) {
		timer_handler(i+1);
	}
	TEST_ASSERT(done == TEST_TIMER_CREATE_100 * 3);

	done = 0;
	for (int i = 0; i < TEST_TIMER_CREATE_100; i++) {
		get_systick_ExpectAndReturn(0);
		timer[i] = timer_create_core(i+1, do_nothing, TIMER_MAX_RERUN);
		TEST_ASSERT(timer[i] != -EINVAL);
		TEST_ASSERT(timer[i] != -ENOMEM);
		TEST_ASSERT(timer[i] > 0);
	}
	for (int i = 0; i < TEST_TIMER_CREATE_100*TIMER_MAX_RERUN; i++) {
		timer_handler(i+1);
	}
	TEST_ASSERT(done == TEST_TIMER_CREATE_100 * TIMER_MAX_RERUN);
}

void test_timer_create_repeat(void)
{
	uintptr_t timer[TEST_TIMER_CREATE_100];

	done = 0;
	for (int i = 0; i < TEST_TIMER_CREATE_100; i++) {
		get_systick_ExpectAndReturn(0);
		timer[i] = timer_create_core(i+1, do_nothing, TIMER_REPEAT);
		TEST_ASSERT(timer[i] != -EINVAL);
		TEST_ASSERT(timer[i] != -ENOMEM);
		TEST_ASSERT(timer[i] > 0);
	}

	for (int i = 0; i < TEST_TIMER_CREATE_100; i++) {
		timer_handler(i+1);
	}
}

void test_timer_nearest(void)
{
}

void test_timer_delete(void)
{
}
