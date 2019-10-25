#ifndef __YAOS_SYSLOG_H__
#define __YAOS_SYSLOG_H__

#include <stdio.h>

enum {
	SYSLOG_FD_DEBUG,
	SYSLOG_FD_STDOUT,
	SYSLOG_FD_BUFFER,
};

#if !defined(NDEBUG)
/** Print out debug messages using hardware debug port in early booting stage */
	#define debug(...)		do { \
		dprintf(SYSLOG_FD_DEBUG, "%p:%s():%d: ", \
			__builtin_return_address(0), __func__, __LINE__); \
		dprintf(SYSLOG_FD_DEBUG, __VA_ARGS__); \
		dprintf(SYSLOG_FD_DEBUG, "\r\n"); \
	} while (0)
	#define info(...)		debug(__VA_ARGS__)
	#define warn(...)		debug(__VA_ARGS__)
	#define error(...)		debug(__VA_ARGS__)
	#define alert(...)		debug(__VA_ARGS__)
#else
	#define debug(...)		((void)0)
	#define info(...)		((void)0)
	#define warn(...)		((void)0)
	#define error(...)		((void)0)
	#define alert(...)		((void)0)
#endif

//#define syslog(...)		((void)0)
#define syslog(...)		dprintf(SYSLOG_FD_DEBUG, __VA_ARGS__)
#define printk(...)		syslog(__VA_ARGS__)

/** Custom logging function
 *
 * ram buffer, flash memory, uart stream, or anything can be binded.
 */
extern void (*printc)(const int c);

#endif /* __YAOS_SYSLOG_H__ */
