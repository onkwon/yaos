#ifndef __YAOS_LOG_H__
#define __YAOS_LOG_H__

#include <stdio.h>

#if !defined(NDEBUG)
	#define debug(...)	do { \
		printf("%p:%s():%d: ", \
			__builtin_return_address(0), __func__, __LINE__); \
		printf(__VA_ARGS__); \
		printf("\r\n"); \
	} while (0)
#else
	#define debug(...)
#endif

#endif /* __YAOS_LOG_H__ */
