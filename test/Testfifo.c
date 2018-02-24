#include <lib/fifo.h>
#include "unity.h"
#include <stdint.h>
#include <errno.h>

int fls(int x)
{
	return UNITY_INT_WIDTH - __builtin_clz(x);
}

int __ldrex(int *addr)
{
	return *addr;
}

int __strex(int v, int *addr)
{
	*addr = v;
	return 0;
}

#define NQ		10

void test_fifo_byte()
{
	struct fifo q;
	uint8_t buf[NQ];
	int i;

	fifo_init(&q, buf, NQ);

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(0, fifo_putb(&q, i));

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(-ENOSPC, fifo_putb(&q, i));

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(i, fifo_getb(&q));

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(-ENOENT, fifo_getb(&q));

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(0, fifo_putb(&q, i));

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(-ENOSPC, fifo_putb(&q, i));

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(i, fifo_getb(&q));

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(-ENOENT, fifo_getb(&q));
}

void test_fifo_word()
{
	struct fifo q;
	unsigned int buf[NQ], t;
	int i;

	fifo_init(&q, buf, NQ);

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(0, fifo_putw(&q, i));

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(-ENOSPC, fifo_putw(&q, i));

	for (i = 1; i < NQ; i++) {
		TEST_ASSERT_EQUAL_INT(0, fifo_getw(&q, &t));
		TEST_ASSERT_EQUAL_INT(i, t);
	}

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(-ENOENT, fifo_getw(&q, &t));

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(0, fifo_putw(&q, i));

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(-ENOSPC, fifo_putw(&q, i));

	for (i = 1; i < NQ; i++) {
		TEST_ASSERT_EQUAL_INT(0, fifo_getw(&q, &t));
		TEST_ASSERT_EQUAL_INT(i, t);
	}

	for (i = 1; i < NQ; i++)
		TEST_ASSERT_EQUAL_INT(-ENOENT, fifo_getw(&q, &t));
}

void test_fifo_ptr()
{
	struct fifo q;
	unsigned int buf[NQ], *t;
	int i;

	fifo_init(&q, buf, NQ);

	for (i = 0; i < (NQ-1); i++) {
		t = (unsigned int *)(0x80000000 + i * 4);
		TEST_ASSERT_EQUAL_INT(0, fifo_put(&q, t));
	}

	for (i = 0; i < (NQ-1); i++)
		TEST_ASSERT_EQUAL_INT(-ENOSPC, fifo_put(&q, t));

	for (i = 0; i < (NQ-1); i++) {
		TEST_ASSERT_EQUAL_INT(0, fifo_get(&q, &t));
		TEST_ASSERT_EQUAL_INT(0x80000000 + i * 4, (unsigned int)t);
	}

	for (i = 0; i < (NQ-1); i++)
		TEST_ASSERT_EQUAL_INT(-ENOENT, fifo_get(&q, &t));
}

void setUp()
{
}

void tearDown()
{
}

int main()
{
	UNITY_BEGIN();
	RUN_TEST(test_fifo_byte);
	RUN_TEST(test_fifo_word);
	RUN_TEST(test_fifo_ptr);
	return UNITY_END();
}
