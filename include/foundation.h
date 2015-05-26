#ifndef __FOUNDATION_H__
#define __FOUNDATION_H__

#define HZ			50

#define WORD_SIZE		sizeof(int)
#define WORD_BITS		(WORD_SIZE << 3)

#define HASH_CONSTANT		0x9e370001UL

#include <types.h>
#include <error.h>

/* hard coded delay functions, machine dependant */
extern inline void udelay(unsigned us);
#define mdelay(ms)		udelay((ms)  * 1000)
#define sdelay(sec)		mdelay((sec) * 1000)

#include <io.h>
#include <kernel/lock.h>
#include <time.h>

#endif /* __FOUNDATION_H__ */
