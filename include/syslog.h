#ifndef __YAOS_SYSLOG_H__
#define __YAOS_SYSLOG_H__

#include <stdio.h>

enum {
	SYSLOG_FD_DEBUG,
	SYSLOG_FD_STDOUT,
	SYSLOG_FD_BUFFER,
};

enum {
	SYSLOG_ALERT,
	SYSLOG_ERROR,
	SYSLOG_WARN,
	SYSLOG_NOTICE,
	SYSLOG_INFO,
	SYSLOG_DEFAULT,
};

#if !defined(NDEBUG)
/** Print out debug messages using hardware debug port in early booting stage */
	#define debug(...)	do { \
		dprintf(SYSLOG_FD_DEBUG, "%p:%s():%d: ", \
			__builtin_return_address(0), __func__, __LINE__); \
		dprintf(SYSLOG_FD_DEBUG, __VA_ARGS__); \
		dprintf(SYSLOG_FD_DEBUG, "\r\n"); \
	} while (0)

#else
	#define debug(...)	((void)0)
#endif

#define syslog(...)	do { \
	dprintf(SYSLOG_FD_STDOUT, __VA_ARGS__); \
	dprintf(SYSLOG_FD_STDOUT, "\r\n"); \
} while (0)
#define syserr(...)	do { \
	dprintf(SYSLOG_FD_STDOUT, "[ERROR] "); \
	syslog(__VA_ARGS__); \
} while (0)
#define syswarn(...)	do { \
	dprintf(SYSLOG_FD_STDOUT, "[WARN]  "); \
	syslog(__VA_ARGS__); \
} while (0)
#define sysinfo(...)	do { \
	dprintf(SYSLOG_FD_STDOUT, "[INFO]  "); \
	syslog(__VA_ARGS__); \
} while (0)
#define alert(...)	do { \
	dprintf(SYSLOG_FD_STDOUT, "[ALERT] "); \
	syslog(__VA_ARGS__); \
} while (0)

/** Custom logging function
 *
 * ram buffer, flash memory, uart stream anything can be done implementing
 * your own function and put it in.
 */
void (*printc)(const int c);

#endif /* __YAOS_SYSLOG_H__ */
