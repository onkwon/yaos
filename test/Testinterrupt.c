#include "unity.h"
#include "mock.h"

#include "arch/interrupt.h"

#include <errno.h>

static unsigned int hasperm = 1;

//int __attribute__((weak)) __get_cntl(void) { return 0; }
int __attribute__((weak)) __get_psr(void) { return 0; }
unsigned int __get_cntl(void) { return !hasperm; }

static void dummy_isr(const int vector) { }
int dummy_isr_register(const int vector, void (*f)(const int)) { return 0; }

static void test_register_primary_isr()
{
	// NVECTOR_IRQ ~ IRQ_MAX(128-1) inclusive
	TEST_ASSERT(register_isr(NVECTOR_IRQ-1, NULL)	== -EACCES);
	TEST_ASSERT(register_isr(PRIMARY_IRQ_MAX, NULL) == -ERANGE);

	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT(register_isr(i, NULL)	== 0);

	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT(register_isr(i, dummy_isr)	== 0);

	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT(register_isr(i, dummy_isr)	== -EEXIST);

	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT(unregister_isr(i)		== 0);

	// do it above once again
	TEST_ASSERT(register_isr(NVECTOR_IRQ-1, NULL)	== -EACCES);
	TEST_ASSERT(register_isr(PRIMARY_IRQ_MAX, NULL) == -ERANGE);

	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT(register_isr(i, NULL)	== 0);

	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT(register_isr(i, dummy_isr)	== 0);

	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT(register_isr(i, dummy_isr)	== -EEXIST);

	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT(unregister_isr(i)		== 0);
}

static void test_register_secondary_isr()
{
	int lvec;

	for (int i = 0; i < SECONDARY_IRQ_MAX; i++) {
		lvec = mkvector(NVECTOR_IRQ, i);
		TEST_ASSERT(register_isr(lvec, NULL)	== -ENOENT);
	}

	TEST_ASSERT(register_isr_register(NVECTOR_IRQ, dummy_isr_register, 0) == 0);

	for (int i = 0; i < SECONDARY_IRQ_MAX; i++) {
		lvec = mkvector(NVECTOR_IRQ, i);
		TEST_ASSERT(register_isr(lvec, NULL)	== 0);
	}

	TEST_ASSERT(register_isr_register(NVECTOR_IRQ, NULL, 0) == -EEXIST);
	TEST_ASSERT(register_isr_register(NVECTOR_IRQ, NULL, 1) == 0);

	for (int i = 0; i < SECONDARY_IRQ_MAX; i++) {
		lvec = mkvector(NVECTOR_IRQ, i);
		TEST_ASSERT(register_isr(lvec, NULL)	== -ENOENT);
	}

	// do it above once again
	TEST_ASSERT(register_isr_register(NVECTOR_IRQ, dummy_isr_register, 0) == 0);

	for (int i = 0; i < SECONDARY_IRQ_MAX; i++) {
		lvec = mkvector(NVECTOR_IRQ, i);
		TEST_ASSERT(register_isr(lvec, NULL)	== 0);
	}

	TEST_ASSERT(register_isr_register(NVECTOR_IRQ, NULL, 0) == -EEXIST);
	TEST_ASSERT(register_isr_register(NVECTOR_IRQ, NULL, 1) == 0);

	for (int i = 0; i < SECONDARY_IRQ_MAX; i++) {
		lvec = mkvector(NVECTOR_IRQ, i);
		TEST_ASSERT(register_isr(lvec, NULL)	== -ENOENT);
	}
}

static void test_boundary()
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
	/* it returns -ERANGE if vector is out of range somthing like overflow */
	TEST_ASSERT(unregister_isr(mkvector(PRIMARY_IRQ_MAX-1, SECONDARY_IRQ_MAX)) == -ERANGE);

	for (int i = 0; i < SECONDARY_IRQ_MAX-1; i++)
		TEST_ASSERT(unregister_isr(mkvector(PRIMARY_IRQ_MAX-1, i)) == -ENOENT);
}

static void test_permission()
{
	hasperm = 0;
	TEST_ASSERT(register_isr(NVECTOR_IRQ-1, NULL)	== -EPERM);
	TEST_ASSERT(unregister_isr(NVECTOR_IRQ-1)	== -EPERM);
	TEST_ASSERT(register_isr_register(NVECTOR_IRQ-1, NULL, 0)	== -EPERM);
	hasperm = 1;
	TEST_ASSERT(register_isr(NVECTOR_IRQ-1, NULL)	!= -EPERM);
	TEST_ASSERT(unregister_isr(NVECTOR_IRQ-1)	!= -EPERM);
	TEST_ASSERT(register_isr_register(NVECTOR_IRQ-1, NULL, 0)	!= -EPERM);
}

static void test_register_isr_register()
{
	TEST_ASSERT(register_isr_register(NVECTOR_IRQ, dummy_isr_register, 0) == 0);
	TEST_ASSERT(register_isr_register(PRIMARY_IRQ_MAX, dummy_isr_register, 0) == -ERANGE);
	TEST_ASSERT(register_isr_register(NVECTOR_IRQ-1, dummy_isr_register, 0) == -EACCES);
	TEST_ASSERT(register_isr_register(NVECTOR_IRQ, NULL, 1) == 0);
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
	RUN_TEST(test_boundary);
	RUN_TEST(test_unregister_isr);
	RUN_TEST(test_register_isr_register);
	RUN_TEST(test_permission);
	return UNITY_END();
}
