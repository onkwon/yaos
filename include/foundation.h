#ifndef __FOUNDATION_H__
#define __FOUNDATION_H__

#define HZ		50
#define HEAP_SIZE	0x8000	/* 32KiB */

#define HSI		8000000	/* 8MHz */

#include "types.h"

#define GET_PC() ({ \
		unsigned __pc; \
		__asm__ __volatile__("mov %0, pc" : "=r" (__pc)); \
		__pc; \
	})
#define GET_SP() ({ \
		unsigned __sp; \
		__asm__ __volatile__("mov %0, sp" : "=r" (__sp)); \
		__sp; \
	})
#define GET_PSR() ({ \
		unsigned __psr; \
		__asm__ __volatile__("mrs %0, psr" : "=r" (__psr)); \
		__psr; \
	})
#define GET_LR() ({ \
		unsigned __lr; \
		__asm__ __volatile__("mov %0, lr" : "=r" (__lr)); \
		__lr; \
	})
#define GET_INT() ({ \
		unsigned __primask; \
		__asm__ __volatile__("mrs %0, primask" : "=r" (__primask)); \
		__primask; \
	})
#define GET_CON() ({ \
		unsigned __control; \
		__asm__ __volatile__("mrs %0, control" : "=r" (__control)); \
		__control; \
	})

extern void udelay(unsigned us);
#define mdelay(ms)	udelay((ms)  * 1000)
#define sdelay(sec)	mdelay((sec) * 1000)

extern int printf(const char *format, ...);
extern int kprintf(const char *format, ...);

extern int get_resetf();

#ifdef DEBUG
#define DBUG(fmt) do { \
	kprintf("%s:%d:%s(): ", __FILE__, __LINE__, __func__); \
	kprintf fmt; \
} while (0)
#else
#define DBUG(fmt)
#endif

#include "lock.h"

#endif /* __FOUNDATION_H__ */
