#ifndef __ERROR_H__
#define __ERROR_H__

/* return error code */
#define ERR_UNDEF		1
#define ERR_RETRY		2
#define ERR_ALLOC		3
#define ERR_RANGE		4
#define ERR_PARAM		5
#define ERR_PERM		6
#define ERR_DUP			7
#define ERR_PATH		8
#define ERR_ATTR		9
#define ERR_CREATE		10
#define ERR_OPEN		11
#define ERR_MAX			12

#define MSG_ERROR		0
#define MSG_SYSTEM		1
#define MSG_DEBUG		2
#define MSG_USER		3

#define panic()		do {	\
	error("panic");		\
	while (1) ;		\
} while (0)

#define freeze()	do {	\
	error("freezed");	\
	while (1) ;		\
} while (0)

#include <io.h>

#define error(fmt...)	do {			\
	printk("error: %s: ", __func__);	\
	printk(fmt);				\
	printk("\n");				\
} while (0)

#define warn(fmt...)	do {			\
	printk(fmt);				\
	printk("\n");				\
} while (0)

#define notice(fmt...)	do {			\
	printk(fmt);				\
	printk("\n");				\
} while (0)

#ifdef CONFIG_DEBUG
#define debug(fmt...)	do { \
	printk("%p:%s():%d: ", \
			__builtin_return_address(0), __func__, __LINE__); \
	printk(fmt); \
	printk("\n"); \
} while (0)

#define assert(exp)	do {						\
	if (!(exp)) {							\
		printk("%p:%s():%d: Assertion `%s` failed.\n",		\
				__builtin_return_address(0),		\
				__func__, __LINE__, #exp);		\
		freeze();						\
	}								\
} while (0)
#else /* CONFIG_DEBUG */
#define debug(fmt...)
#define assert(exp)
#endif

#endif /* __ERROR_H__ */
