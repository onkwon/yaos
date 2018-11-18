#include "unity.h"

void test_assert()
{
	TEST_ASSERT(1 == 1);
	TEST_ASSERT_EQUAL_INT(1, 1);
	TEST_ASSERT_EQUAL_HEX8(1, 1);
	TEST_ASSERT_EQUAL_UINT16(1, 1);
	TEST_ASSERT_EQUAL_FLOAT(1.1, 1.1);
	TEST_ASSERT_EQUAL_STRING("str", "str");
	//TEST_ASSERT_EQUAL_ARRAY(, );
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
	RUN_TEST(test_assert);
	return UNITY_END();
}
