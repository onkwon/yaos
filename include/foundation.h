#ifndef __FOUNDATION_H__
#define __FOUNDATION_H__

#define HZ			50

#define CONSOLE			"usart1"

#include <types.h>
#include <error.h>

/* hard coded delay functions, machine dependant */
extern inline void udelay(unsigned us);
#define mdelay(ms)		udelay((ms)  * 1000)
#define sdelay(sec)		mdelay((sec) * 1000)

#include <kernel/lock.h>
#include <io.h>

#endif /* __FOUNDATION_H__ */
