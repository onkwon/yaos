#ifndef __FOUNDATION_H__
#define __FOUNDATION_H__

#define HSI		8000000	/* 8MHz */
#define HEAP_SIZE	0x8000	/* 32KiB */

#include "types.h"

/*
static inline int set_atomw(int var, int val)
{
	int res;

	__asm__ __volatile__("ldrex %1, %0	\n\t"
			     "strex %1, %2, %0	\n\t"
			     : "+m" (var), "=l" (res) : "l" (val));

	return res;
}
#define get_atomw(var)
*/

#define preempt_disable()
#define preempt_enable()
#define sei()		__asm__ __volatile__("cpsie i")
#define cli()		__asm__ __volatile__("cpsid i")

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

void udelay(unsigned us);
#define mdelay(ms)	udelay((ms)  * 1000)
#define sdelay(sec)	mdelay((sec) * 1000)

int printf(const char *format, ...);
int kprintf(const char *format, ...);

int get_resetf();

#endif /* __FOUNDATION_H__ */
