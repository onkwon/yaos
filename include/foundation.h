#ifndef __FOUNDATION_H__
#define __FOUNDATION_H__

#define HZ		50
#define HEAP_SIZE	0x8000	/* 32KiB */

#define preempt_disable()
#define preempt_enable()
#define preempt_count()

extern void udelay(unsigned us);
#define mdelay(ms)	udelay((ms)  * 1000)
#define sdelay(sec)	mdelay((sec) * 1000)

extern int printf(const char *format, ...);
extern int kprintf(const char *format, ...);

#ifdef DEBUG
#define DBUG(fmt) do { \
	kprintf("%s:%d:%s(): ", __FILE__, __LINE__, __func__); \
	kprintf fmt; \
} while (0)
#else
#define DBUG(fmt)
#endif

#include <types.h>
#include <lock.h>

#endif /* __FOUNDATION_H__ */
