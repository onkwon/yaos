#ifndef __FOUNDATION_H__
#define __FOUNDATION_H__

#define HZ			50

#include <types.h>
#include <lock.h>
#include <timer.h>

/* hard coded delay functions, machine dependant */
extern inline void udelay(unsigned us);
#define mdelay(ms)		udelay((ms)  * 1000)
#define sdelay(sec)		mdelay((sec) * 1000)

#include <io.h>

#include <error.h>

#endif /* __FOUNDATION_H__ */
