#include "unity.h"
#include "queue.h"
#include <errno.h>

#include "mock_atomic.h"

void setUp(void)
{
}

void tearDown(void)
{
}

#define ARR_SIZE	QUEUE_MAX_ITEM

void test_queue(void)
{
	static uint32_t arr[ARR_SIZE];
	uint32_t val;
	queue_t q;

	queue_init_static(&q, ARR_SIZE, sizeof(*arr), arr);
	TEST_ASSERT_EQUAL(arr, q.data);
	TEST_ASSERT_EQUAL(sizeof(*arr), q.itemsize);
	TEST_ASSERT_EQUAL(ARR_SIZE, q.n);

	for (int i = 0; i < (ARR_SIZE - 1); i++) {
		TEST_ASSERT_EQUAL(0, enqueue(&q, QUEUE_ITEM(i)));
		TEST_ASSERT_EQUAL(0, queue_peek(&q, &val));
		TEST_ASSERT_EQUAL(val, 0);
		TEST_ASSERT_EQUAL(i + 1, queue_count(&q));
	}

	TEST_ASSERT_EQUAL(-ENOSPC, enqueue(&q, QUEUE_ITEM(val)));

	for (int i = 0; i < (ARR_SIZE - 1); i++) {
		TEST_ASSERT_EQUAL(0, dequeue(&q, &val));
		TEST_ASSERT_EQUAL(val, i);
	}

	TEST_ASSERT_EQUAL(-ENOENT, dequeue(&q, &val));

	for (int i = 0; i < (ARR_SIZE / 5); i++) {
		TEST_ASSERT_EQUAL(0, enqueue(&q, QUEUE_ITEM(i)));
		TEST_ASSERT_EQUAL(i+1, queue_count(&q));
	}

	TEST_ASSERT_EQUAL(ARR_SIZE / 5, queue_count(&q));
	TEST_ASSERT_EQUAL(0, dequeue(&q, &val));
	TEST_ASSERT_EQUAL(0, dequeue(&q, &val));
	TEST_ASSERT_EQUAL(ARR_SIZE / 5 - 2, queue_count(&q));
}

static void serialize(int val)
{
	static int prev = 0;

	//spin_lock();

	if ((prev + 1) != val)
		printf("ERROR: %d | %d\n", prev, val);

	prev = val;

	//spin_unlock();
}

static void task1(void)
{
	int val = 0;
	//enqueue(val);

	while (val < ARR_SIZE) {
		// random delay
		// val = dequeue()
		// val = val + 1;
		// serialize(val);
		// random delay
		// enqueue(val)
	}
}

static void task2(void)
{
	int val = 0;

	while (val < ARR_SIZE) {
		// random delay
		// val = dequeue()
		// val = val + 1;
		// serialize(val);
		// random delay
		// enqueue(val)
	}
}
