#include "unity.h"
#include "kernel/init.h"
#include "kernel/debug.h"
#include "kernel/timer.h"
#include "heap.h"
#include "firstfit.h"

#include "mock_io.h"
#include "mock_hw_io.h"
#include "mock_interrupt.h"
#include "mock_hw_interrupt.h"
#include "mock_systick.h"
#include "mock_hw_debug.h"
#include "mock_sched.h"
#include "mock_syscall.h"

uintptr_t _bss, _ebss, _data, _edata, _etext, _init_func_list, _driver_list, _heap_start, _ram_end;
struct task *current;

void setUp(void)
{
}

void tearDown(void)
{
}

void test_template(void)
{
	TEST_IGNORE_MESSAGE("Implement me!");
}
