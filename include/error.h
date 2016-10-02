#ifndef __ERROR_H__
#define __ERROR_H__

/* return error code */
#define ERR_RETRY		1
#define ERR_UNDEF		2
#define ERR_ALLOC		3
#define ERR_RANGE		4
#define ERR_PARAM		5
#define ERR_PERM		6
#define ERR_DUP			7
#define ERR_PATH		8
#define ERR_ATTR		9
#define ERR_CREATE		10

#define MSG_ERROR		0
#define MSG_SYSTEM		1
#define MSG_DEBUG		2
#define MSG_USER		3

#define panic()			({					\
		debug(MSG_ERROR, "panic");				\
		while (1) ;						\
})

#define freeze()		({					\
		debug(MSG_ERROR, "freezed");				\
		while (1) ;						\
})

#include <io.h>
#ifdef CONFIG_DEBUG
#define debug(lv, fmt...)	do {					\
	printk("%04x %s:%s():%d: ", lv, __FILE__, __func__, __LINE__);	\
	printk(fmt);							\
	printk("\n");							\
} while (0)

#define assert(exp)		do {					\
	if (!(exp)) {							\
		printk("%s:%s():%d: Assertion `%s` failed.\n",		\
				__FILE__, __func__, __LINE__, #exp);	\
	}								\
	freeze();							\
} while (0)
#else
#define debug(lv, fmt...)	do {					\
	if (lv <= MSG_SYSTEM) {						\
		if (lv == MSG_ERROR)					\
			printk("error: %s: ", __func__);		\
		printk(fmt);						\
		printk("\n");						\
	} else if (lv == MSG_USER) {					\
		printf("%04x %s:%s():%d: ",				\
				lv, __FILE__, __func__, __LINE__);	\
		putchar('\n');						\
	}								\
} while (0)

#define assert(exp)
#endif

#endif /* __ERROR_H__ */
