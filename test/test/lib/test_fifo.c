#include "unity.h"
#include "fifo.h"
#include <stdint.h>
#include <errno.h>

#include "mock_atomic.h"

void setUp(void)
{
}

void tearDown(void)
{
}

#define FIFO_SIZE	100

void test_fifob(void)
{
	static uint8_t arr[FIFO_SIZE];
	struct fifo q1;
	fifo_init(&q1, arr, FIFO_SIZE);

	TEST_ASSERT_EQUAL(-ENOENT, fifo_getb(&q1));
	TEST_ASSERT_EQUAL(0, fifo_putb(&q1, 1));
	TEST_ASSERT_EQUAL(0, fifo_putb(&q1, -1));
	TEST_ASSERT_EQUAL(0, fifo_putb(&q1, 'A'));
	TEST_ASSERT_EQUAL(1, (char)fifo_getb(&q1));
	TEST_ASSERT_EQUAL(-1, (char)fifo_getb(&q1));
	TEST_ASSERT_EQUAL('A', (char)fifo_getb(&q1));
	TEST_ASSERT_EQUAL(-ENOENT, fifo_getb(&q1));

	for (int i = 0; i < FIFO_SIZE-1; i++)
		TEST_ASSERT_EQUAL(0, fifo_putb(&q1, i));
	TEST_ASSERT_EQUAL(-ENOSPC, fifo_putb(&q1, 1));

	for (int i = 0; i < FIFO_SIZE-1; i++)
		TEST_ASSERT_EQUAL((char)i, (char)fifo_getb(&q1));
	TEST_ASSERT_EQUAL(-ENOENT, fifo_getb(&q1));
}

void test_fifo(void)
{
	static uintptr_t arr[FIFO_SIZE];
	struct fifo q1;
	uintptr_t val;
	fifo_init(&q1, arr, FIFO_SIZE);

	TEST_ASSERT_EQUAL(-ENOENT, fifo_get(&q1, &val));
	TEST_ASSERT_EQUAL(0, fifo_put(&q1, 1));
	TEST_ASSERT_EQUAL(0, fifo_put(&q1, -1));
	TEST_ASSERT_EQUAL(0, fifo_put(&q1, 'A'));
	TEST_ASSERT_EQUAL(0, fifo_get(&q1, &val));
	TEST_ASSERT_EQUAL(1, val);
	TEST_ASSERT_EQUAL(0, fifo_get(&q1, &val));
	TEST_ASSERT_EQUAL(-1, val);
	TEST_ASSERT_EQUAL(0, fifo_get(&q1, &val));
	TEST_ASSERT_EQUAL('A', val);
	TEST_ASSERT_EQUAL(-ENOENT, fifo_get(&q1, &val));
	TEST_ASSERT_EQUAL(-EINVAL, fifo_get(&q1, NULL));

	for (int i = 0; i < FIFO_SIZE-1; i++)
		TEST_ASSERT_EQUAL(0, fifo_put(&q1, i));
	TEST_ASSERT_EQUAL(-ENOSPC, fifo_put(&q1, 1));

	for (int i = 0; i < FIFO_SIZE-1; i++) {
		TEST_ASSERT_EQUAL(0, fifo_get(&q1, &val));
		TEST_ASSERT_EQUAL(i, val);
	}
	TEST_ASSERT_EQUAL(-ENOENT, fifo_get(&q1, &val));
}
