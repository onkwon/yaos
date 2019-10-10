#include "syslog.h"
#include "kernel/syscall.h"

void __assert_func(const char * const file, const int line,
		const char * const func, const char * const exp);

void __attribute__((noreturn)) __assert_func(const char * const file,
		const int line, const char * const func, const char * const exp)
{
	error("%s:%s:%d: %s\r\n", file, func, line, exp);
#if !defined(NDEBUG)
	while (1);
#else
	reboot(0);
#endif
}
