#include "unity.h"
#include "bitmap.h"
#include <errno.h>

#define BITMAP_TESTSIZE_MAX		4096 // bits

void setUp(void)
{
}

void tearDown(void)
{
}

void test_bitmap(void)
{
	for (int bits = 0; bits < BITMAP_TESTSIZE_MAX; bits++) {
		DEFINE_BITMAP(bitmap, bits);

		bitmap_init(bitmap, bits, 0);
		for (int i = 0; i < bits; i++)
			TEST_ASSERT_EQUAL(0, bitmap_get(bitmap, i));
		bitmap_init(bitmap, bits, 1);
		for (int i = 0; i < bits; i++)
			TEST_ASSERT_EQUAL(1, bitmap_get(bitmap, i));
	}

	DEFINE_BITMAP(bitmap, BITMAP_TESTSIZE_MAX);
	bitmap_init(bitmap, BITMAP_TESTSIZE_MAX, 0);

	for (int i = 0; i < BITMAP_TESTSIZE_MAX; i++) {
		if (i % 2)
			continue;
		bitmap_set(bitmap, i);
		TEST_ASSERT_EQUAL(i/2+1, bitmap_count(bitmap, BITMAP_TESTSIZE_MAX));
	}

	TEST_ASSERT_EQUAL(BITMAP_TESTSIZE_MAX / 2, bitmap_count(bitmap, BITMAP_TESTSIZE_MAX));

	for (int i = 0; i < BITMAP_TESTSIZE_MAX; i++) {
		if (i % 2)
			TEST_ASSERT_EQUAL(0, bitmap_get(bitmap, i));
		else
			TEST_ASSERT_EQUAL(1, bitmap_get(bitmap, i));
	}

	bitmap_clear(bitmap, 2);
	TEST_ASSERT_EQUAL(BITMAP_TESTSIZE_MAX / 2 - 1, bitmap_count(bitmap, BITMAP_TESTSIZE_MAX));
}
