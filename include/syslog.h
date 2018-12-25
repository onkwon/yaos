#ifndef __YAOS_SYSLOG_H__
#define __YAOS_SYSLOG_H__

#include <stdio.h>

enum {
	SYSLOG_ALERT,	// "ALERT:"
	SYSLOG_ERROR,	// "ERROR:"
	SYSLOG_WARN,	// "WARN:"
	SYSLOG_NOTICE,	// "NOTICE:"
	SYSLOG_INFO,	// "INFO:"
	SYSLOG_DEFAULT,
};

#if !defined(NDEBUG)
/** Print out debug messages using hardware debug port in early booting stage */
	#define debug(...)	do { \
		printf("%p:%s():%d: ", \
			__builtin_return_address(0), __func__, __LINE__); \
		printf(__VA_ARGS__); \
		printf("\r\n"); \
	} while (0)

// TODO: Implement syslog()
	#define syslog(...)	(printf(__VA_ARGS__))
#else
	#define debug(...)	((void)0)
	#define syslog(...)	(printf(__VA_ARGS__))
#endif

#endif /* __YAOS_SYSLOG_H__ */
