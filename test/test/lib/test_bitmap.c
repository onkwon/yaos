#include "unity.h"
#include "bitmap.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_bitmap_init(void)
{
	TEST_IGNORE();
	// return -EINVAL when array is null
	// return 0 with array initialized as 0
	// return 0 with array initialized as 1
	// check boundary bits
}

void test_bitmap_set(void)
{
	TEST_IGNORE();
	// return -ERANGE when pos is bigger than max
	// return -EINVAL when not initialized
	// return 0 when ok, assert the result
}

void test_bitmap_get(void)
{
	TEST_IGNORE();
	// return -ERANGE when pos is bigger than max
	// return -ENOENT when not initialized
}

void test_template(void)
{
	TEST_IGNORE();
	TEST_IGNORE_MESSAGE("Implement me!");
}
