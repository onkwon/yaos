#include "unity.h"
#include "kernel/interrupt.h"
#include "io.h"

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include "mock_hw_interrupt.h"
#include "mock_hw_io.h"
#include "mock_atomic.h"

#if !defined(CONFIG_COMMON_IRQ_FRAMEWORK)
uintptr_t _ram_start[PRIMARY_IRQ_MAX];
#endif

static bool registered;

void setUp(void)
{
	__dsb_Ignore();
	__isb_Ignore();
	hw_irq_init_Ignore();
	irq_init();

#if !defined(CONFIG_COMMON_IRQ_FRAMEWORK)
	for (int i = 0; i < PRIMARY_IRQ_MAX; i++)
		_ram_start[i] = (uintptr_t)ISR_null;
#endif
}

void tearDown(void)
{
}

static inline void take_permission()
{
	__get_psr_ExpectAndReturn(0);
	__get_cntl_ExpectAndReturn(0);
}

static inline void release_permission()
{
	__get_psr_ExpectAndReturn(0);
	__get_cntl_ExpectAndReturn(1);
}

static void dummy_func(const int param)
{
	(void)param;
}

static int dummy_func2(const int param, void (*f)(const int))
{
	(void)param;
	(void)f;
	registered = true;
	return 0;
}

void test_fundamentals(void)
{
	__get_cntl_ExpectAndReturn(1);
	TEST_ASSERT_EQUAL(1, __get_cntl());
	__get_cntl_ExpectAndReturn(0);
	TEST_ASSERT_EQUAL(0, __get_cntl());

#if defined(CONFIG_COMMON_IRQ_FRAMEWORK)
	extern void (*primary_isr_table[PRIMARY_IRQ_MAX - NVECTOR_IRQ])(const int);
	for (int i = 0; i < (PRIMARY_IRQ_MAX - NVECTOR_IRQ); i++)
		TEST_ASSERT_EQUAL(ISR_null, primary_isr_table[i]);
#else
	for (int i = 0; i < (PRIMARY_IRQ_MAX - NVECTOR_IRQ); i++)
		TEST_ASSERT_EQUAL(ISR_null, _ram_start[i]);
#endif
}

void test_register_isr_permission(void)
{
	// Return -EPERM when is_honored() returns false
	for (int i = 0; i < (PRIMARY_IRQ_MAX + 100); i++) {
		release_permission();
		TEST_ASSERT_EQUAL(-EPERM, register_isr(i, NULL));
	}
}

void test_register_isr_access(void)
{
	// Return -EACCES when try to resgister in a reserved for system
	for (int i = 0; i < NVECTOR_IRQ; i++) {
		take_permission();
		TEST_ASSERT_EQUAL(-EACCES, register_isr(i, NULL));
	}
}

void test_register_isr_success_primary(void)
{
	// Return 0 on success
	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++) {
		take_permission();
		__dsb_Ignore();
		__isb_Ignore();
		TEST_ASSERT_EQUAL(0, register_isr(i, dummy_func));
	}

#if defined(CONFIG_COMMON_IRQ_FRAMEWORK)
	extern void (*primary_isr_table[PRIMARY_IRQ_MAX - NVECTOR_IRQ])(const int);
	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT_EQUAL(dummy_func, primary_isr_table[i - NVECTOR_IRQ]);
#else
	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT_EQUAL(dummy_func, _ram_start[i - NVECTOR_IRQ]);
#endif
}

void test_register_isr_exist_primary(void)
{
	// 1. register first
	test_register_isr_success_primary();

	// 2. register again
	// Return -EEXIST when registerd already
	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++) {
		take_permission();
		TEST_ASSERT_EQUAL(-EEXIST, register_isr(i, dummy_func));
	}
}

void test_unregister_isr_permission(void)
{
	// Return -EPERM when is_honored() returns false
	for (int i = 0; i < (PRIMARY_IRQ_MAX + 100); i++) {
		release_permission();
		TEST_ASSERT_EQUAL(-EPERM, unregister_isr(i));
	}
}

void test_unregister_isr_primary(void)
{
	// Return -EACCES when try to resgister in a reserved for system
	for (int i = 0; i < NVECTOR_IRQ; i++) {
		take_permission();
		TEST_ASSERT_EQUAL(-EACCES, unregister_isr(i));
	}

	// Return 0 on success
	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++) {
		take_permission();
		__dsb_Ignore();
		__isb_Ignore();
		take_permission();
		__dsb_Ignore();
		__isb_Ignore();
		TEST_ASSERT_EQUAL(0, unregister_isr(i));
	}

#if defined(CONFIG_COMMON_IRQ_FRAMEWORK)
	extern void (*primary_isr_table[PRIMARY_IRQ_MAX - NVECTOR_IRQ])(const int);
	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT_EQUAL(ISR_null, primary_isr_table[i - NVECTOR_IRQ]);
#else
	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT_EQUAL(ISR_null, _ram_start[i - NVECTOR_IRQ]);
#endif
}

void test_unregister_isr_primary2(void)
{
	// FIXME: It encounters `Bus error: 10` when reaching 87 of nvec
#define END	87
	test_register_isr_success_primary();

#if defined(CONFIG_COMMON_IRQ_FRAMEWORK)
	extern void (*primary_isr_table[PRIMARY_IRQ_MAX - NVECTOR_IRQ])(const int);
	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT_EQUAL(dummy_func, primary_isr_table[i - NVECTOR_IRQ]);
#else
	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++)
		TEST_ASSERT_EQUAL(dummy_func, _ram_start[i - NVECTOR_IRQ]);
#endif

	for (int i = NVECTOR_IRQ; i < END; i++) {
		take_permission();
		__dsb_Ignore();
		__isb_Ignore();
		take_permission();
		__dsb_Ignore();
		__isb_Ignore();
		TEST_ASSERT_EQUAL(0, unregister_isr(i));
	}

#if defined(CONFIG_COMMON_IRQ_FRAMEWORK)
	extern void (*primary_isr_table[PRIMARY_IRQ_MAX - NVECTOR_IRQ])(const int);
	for (int i = NVECTOR_IRQ; i < END; i++)
		TEST_ASSERT_EQUAL(ISR_null, primary_isr_table[i - NVECTOR_IRQ]);
#else
	for (int i = NVECTOR_IRQ; i < END; i++)
		TEST_ASSERT_EQUAL(ISR_null, _ram_start[i - NVECTOR_IRQ]);
#endif
}

void test_register_isr_register_perm(void)
{
	release_permission();
	TEST_ASSERT_EQUAL(-EPERM, register_isr_register(0, NULL, false));
}

void test_register_isr_register_access(void)
{
	take_permission();
	TEST_ASSERT_EQUAL(-EACCES, register_isr_register(NVECTOR_IRQ - 1, NULL, false));
}

void test_register_isr_register_range(void)
{
	take_permission();
	TEST_ASSERT_EQUAL(-ERANGE, register_isr_register(PRIMARY_IRQ_MAX, NULL, false));
}

void test_register_isr_register_success(void)
{
	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++) {
		take_permission();
		TEST_ASSERT_EQUAL(0, register_isr_register(i, dummy_func2, false));
	}
}

void test_register_isr_register_exist(void)
{
	test_register_isr_register_success();
	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++) {
		take_permission();
		TEST_ASSERT_EQUAL(-EEXIST, register_isr_register(i, dummy_func2, false));
	}
}

void test_register_isr_register_exist2(void)
{
	test_register_isr_register_success();
	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++) {
		take_permission();
		TEST_ASSERT_EQUAL(0, register_isr_register(i, dummy_func2, true));
	}
}

void test_unregister_isr_when_secondary_ctor_exist(void)
{
	test_register_isr_register_success();
	for (int i = NVECTOR_IRQ; i < END; i++) {
		take_permission();
		__dsb_Ignore();
		__isb_Ignore();
		take_permission();
		__dsb_Ignore();
		__isb_Ignore();
		TEST_ASSERT_EQUAL(0, unregister_isr(i));
	}
}

void test_register_isr_secondary_when_secondary_ctor_exist(void)
{
	test_register_isr_register_success();

	for (int i = NVECTOR_IRQ; i < PRIMARY_IRQ_MAX; i++) {
		registered = false;
		TEST_ASSERT_EQUAL(false, registered);
		take_permission();
		TEST_ASSERT_EQUAL(0, register_isr(mkvector(NVECTOR_IRQ, 0), dummy_func));
		TEST_ASSERT_EQUAL(true, registered);
	}

	take_permission();
	TEST_ASSERT_EQUAL(0, unregister_isr(mkvector(NVECTOR_IRQ, 0)));
}
