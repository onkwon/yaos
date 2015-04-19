#ifndef __FOUNDATION_H__
#define __FOUNDATION_H__

#define HZ			50
#define HEAP_SIZE		0x8000	/* 32KiB */

#define barrier()		__asm__ __volatile__("" ::: "memory")

#include <asm/interrupt.h>

#define irq_save(flag)		__irq_save(flag)
#define irq_restore(flag)	__irq_restore(flag)
#define cli()			__cli()
#define sei()			__sei()
#define dmb()			__dmb()
#define dsb()			__dsb()
#define isb()			__isb()

extern void udelay(unsigned us);
#define mdelay(ms)		udelay((ms)  * 1000)
#define sdelay(sec)		mdelay((sec) * 1000)

extern int printf(const char *format, ...);
extern int kprintf(const char *format, ...);

#ifdef DEBUG
#define DBUG(fmt) do { \
	kprintf("%s:%s():%d: ", __FILE__, __func__, __LINE__); \
	kprintf fmt; \
} while (0)
#else
#define DBUG(fmt)
#endif

#include <types.h>
#include <lock.h>

#endif /* __FOUNDATION_H__ */
