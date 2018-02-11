/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#include <kernel/lock.h>

unsigned int sysfreq;

unsigned int __attribute__((section(".data"))) systick;
uint64_t __attribute__((section(".data"))) systick64;

DEFINE_SPINLOCK(lock_systick64);

uint64_t get_systick64()
{
	uint64_t stamp = 0;

#ifdef CONFIG_SMP
	do {
		if (!is_locked(lock_systick64))
#endif
			stamp = (typeof(systick64))
				*(volatile typeof(systick64) *)&systick64;
#ifdef CONFIG_SMP
	} while (is_locked(lock_systick64));
#endif

	return stamp;
}

static inline void update_tick(unsigned int delta)
{
	unsigned int irqflag;

	spin_lock_irqsave(&lock_systick64, irqflag);
	systick64 += delta;
	spin_unlock_irqrestore(&lock_systick64, irqflag);
}

static unsigned int period, interval;

unsigned int get_curr_interval()
{
	return interval;
}

void ISR_systick(int nvector)
{
#ifdef CONFIG_SLEEP_LONG
	static unsigned int clks;
	unsigned int ticks;

	clks += interval;

	if (clks >= period) {
		/* TODO: udiv instruction takes 2-12 cycles. replace it with
		 * bit operation. what about making period as a constant
		 * reducing the number of accessing memory? */
		ticks = clks / period;
		clks -= period * ticks;
		update_tick(ticks);
		resched();
	}

	interval = get_sysclk_max();
#else
	update_tick(1);
	resched();
#endif
	(void)nvector;
}

#include <kernel/init.h>
#include <asm/sysclk.h>

void __init systick_init()
{
	register_isr(sysclk_init(), ISR_systick);

	period = get_sysclk_period();
	interval = get_sysclk_max();

	run_sysclk();
}
