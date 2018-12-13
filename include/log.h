#ifndef __YAOS_LOG_H__
#define __YAOS_LOG_H__

#include <stdio.h>

#if !defined(NDEBUG)
/** Print out debug messages using hardware debug port in early booting stage */
	#define debug(...)	do { \
		printf("%p:%s():%d: ", \
			__builtin_return_address(0), __func__, __LINE__); \
		printf(__VA_ARGS__); \
		printf("\r\n"); \
	} while (0)
#else
	#define debug(...)	((void)0)
#endif

#endif /* __YAOS_LOG_H__ */
