#include "log.h"

void __assert_func(const char * const file, const int line,
		const char * const func, const char * const exp);

void __assert_func(const char * const file, const int line,
		const char * const func, const char * const exp)
{
	printf("%s:%s:%d: %s\r\n", file, func, line, exp);
	while (1); // TODO: Stop all the other tasks too
}
