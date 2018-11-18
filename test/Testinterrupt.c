#include "unity.h"
#include "mock.h"

#include "arch/interrupt.h"

#include <errno.h>

static unsigned int hasperm = 1;

//int __attribute__((weak)) __get_cntl(void) { return 0; }
int __attribute__((weak)) __get_psr(void) { return 0; }
unsigned int __get_cntl(void) { return !hasperm; }

static void dummy_isr(const int vector) { }

static void test_register_primary_isr()
{
	TEST_ASSERT(register_isr(NVECTOR_IRQ-1, NULL)	== -EACCES);
	//TEST_ASSERT(register_isr(NVECTOR_IRQ, NULL) == -EEXIST);
	//TEST_ASSERT(register_isr(NVECTOR_IRQ, dummy_isr) == 0);
	//TEST_ASSERT(register_isr(NVECTOR_IRQ, dummy_isr) == -EEXIST);
	//TEST_ASSERT(register_isr(NVECTOR_IRQ, NULL) == 0);

	//TEST_ASSERT_EQUAL_INT(nvic_set(NVECTOR_IRQ, false), 0);
	//TEST_ASSERT_EQUAL_HEX8(a, b);
	//TEST_ASSERT_EQUAL_UINT16(a, b);
	//TEST_ASSERT_EQUAL_FLOAT(a, b);
	//TEST_ASSERT_EQUAL_STRING(a, b);
	//TEST_ASSERT_EQUAL_ARRAY(a, b);
}

static void test_register_secondary_isr()
{
	TEST_ASSERT(register_isr(NVECTOR_IRQ-1, NULL)	== -EACCES);
}

static void test_register_isr_outbound()
{
	TEST_ASSERT(register_isr(NVECTOR_IRQ-1, NULL)	== -EACCES);
}

static void test_unregister_isr()
{
	TEST_ASSERT(unregister_isr(NVECTOR_IRQ-1)	== -EACCES);
	TEST_ASSERT(unregister_isr(NVECTOR_IRQ)		== 0);
	TEST_ASSERT(unregister_isr(PRIMARY_IRQ_MAX-1)	== 0);
	TEST_ASSERT(unregister_isr(PRIMARY_IRQ_MAX)	== -ERANGE);

	TEST_ASSERT(unregister_isr(mkvector(PRIMARY_IRQ_MAX, 0)) == -ERANGE);
	/* it returns 0 from register_primary_isr() because of overflow */
	TEST_ASSERT(unregister_isr(mkvector(PRIMARY_IRQ_MAX-1, SECONDARY_IRQ_MAX)) == 0);

	for (int i = 0; i < SECONDARY_IRQ_MAX-1; i++)
		TEST_ASSERT(unregister_isr(mkvector(PRIMARY_IRQ_MAX-1, i)) == -ENOENT);
}

static void test_permission()
{
	hasperm = 0;
	TEST_ASSERT(register_isr(NVECTOR_IRQ-1, NULL)	== -EPERM);
	TEST_ASSERT(unregister_isr(NVECTOR_IRQ-1)	== -EPERM);
	TEST_ASSERT(register_isr_constructor(NVECTOR_IRQ-1, NULL, 0)	== -EPERM);
	hasperm = 1;
}

static void test_register_isr_constructor()
{
	TEST_ASSERT(register_isr(NVECTOR_IRQ-1, NULL)	== -EACCES);
}

void setUp()
{
	irq_init();
}

void tearDown()
{
}

int main()
{
	UNITY_BEGIN();
	RUN_TEST(test_register_primary_isr);
	RUN_TEST(test_register_secondary_isr);
	RUN_TEST(test_register_isr_outbound);
	RUN_TEST(test_unregister_isr);
	RUN_TEST(test_register_isr_constructor);
	RUN_TEST(test_permission);
	return UNITY_END();
}
