#ifndef __FOUNDATION_H__
#define __FOUNDATION_H__

#define HZ			50

#define barrier()		__asm__ __volatile__("" ::: "memory")

#include <asm/interrupt.h>

#define irq_save(flag)		__irq_save(flag)
#define irq_restore(flag)	__irq_restore(flag)
#define cli()			__cli()
#define sei()			__sei()
#define dmb()			__dmb()
#define dsb()			__dsb()
#define isb()			__isb()

extern int printf(const char *format, ...);
extern int printk(const char *format, ...);

#ifdef CONFIG_DEBUG
#define DEBUG(fmt) do { \
	printk("%s:%s():%d: ", __FILE__, __func__, __LINE__); \
	printk fmt; \
	printk("\n"); \
} while (0)
#else
#define DEBUG(fmt)
#endif

#include <types.h>
#include <lock.h>

#include <timer.h>

/* hard coded delay functions, machine dependant */
extern inline void udelay(unsigned us);
#define mdelay(ms)		udelay((ms)  * 1000)
#define sdelay(sec)		mdelay((sec) * 1000)

#endif /* __FOUNDATION_H__ */
