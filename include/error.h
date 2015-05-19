#ifndef __ERROR_H__
#define __ERROR_H__

/* return error code */
#define ERR_UNDEF		1
#define ERR_ALLOC		2
#define ERR_RANGE		3

#ifdef CONFIG_DEBUG
#define DEBUG(fmt) do { \
	printk("%s:%s():%d: ", __FILE__, __func__, __LINE__); \
	printk fmt; \
	printk("\n"); \
} while (0)
#else
#define DEBUG(fmt)
#endif

#endif /* __ERROR_H__ */
