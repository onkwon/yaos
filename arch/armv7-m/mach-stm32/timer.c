#include <timer.h>
#include <error.h>
#include <asm/timer.h>

#define TIM_COUNT_MAX		(1 << 16) /* 65536 */

#define RUN		0
#define CMS		5
#define DIR		4
#define ARPE		7
#define OCPE		3
#define TS		4
#define SMS		0

#define UPDATE		0

static unsigned int calc_arr(unsigned int fclk, unsigned int hz)
{
	/* TODO: Do not use float data type but int */
	float quotient, remainder, min;
	unsigned int arr, reserve = 0;

	if (fclk < hz)
		return 0;

	min = TIM_COUNT_MAX;

	/* TODO: change the way of finding the largest value of arr */
	for (arr = TIM_COUNT_MAX; arr; arr--) {
		quotient = (float)fclk / hz / arr;
		remainder = quotient - (int)quotient;

		if (remainder < min) {
			min = remainder;
			reserve = arr;
		}

		if (remainder <= 0)
			break;
	}

	if (!arr)
		arr = reserve;

	return arr;
}

static unsigned int calc_psc(unsigned int fclk, unsigned int hz,
		unsigned int arr)
{
	if (fclk < hz)
		return 0;

	return fclk / (hz * arr);
}

void __timer_run(int id, bool on)
{
	struct __timer *tim = __timer_id2reg(id);

	if (on)
		tim->cr1 |= (1 << RUN);
	else
		tim->cr1 &= ~(1 << RUN);
}

void __timer_mode_set(int id, enum tim_mode mode)
{
	struct __timer *tim = __timer_id2reg(id);

	if (mode > TIM_MODE_MAX)
		mode = TIM_OP_EDGE;

	tim->cr1 &= ~(3 << CMS);
	tim->cr1 |= (mode & TIM_OP_MASK) << CMS;

	tim->cr1 &= ~(1 << DIR);
	tim->cr1 |= ((mode & TIM_OP_DNCOUNT) >> TIM_DIR_SHIFT) << DIR;
}

void __timer_channel_set(int id, enum tim_iochannel ch, enum tim_iomode iomode,
		int pin, bool dir)
{
	struct __timer *tim = __timer_id2reg(id);
	int shift;
	unsigned int *reg;

	if (ch < TIM_IO_CH1 || ch >= TIM_CHANNEL_MAX)
		return;

	shift = ch & 1? 0 : 8;
	reg = ch < 3? (unsigned int *)&tim->ccmr1 : (unsigned int *)&tim->ccmr2;

	/* clear the default set by open() */
	tim->ccmr1 = 0;
	tim->ccer = 0;

	if (iomode >= TIM_IOMODE_MAX)
		iomode = TIM_IO_FROZEN;

	if (dir) { /* output */
		*reg &= ~(7 << (4 + shift));
		*reg |= iomode << (4 + shift);

		if (iomode & (TIM_IO_PWM | TIM_IO_PWM2)) {
			tim->cr1 |= 1 << ARPE;
			*reg |= 1 << (OCPE + shift);
		}
	} else { /* input */
		*reg &= ~(3 << shift);
		*reg |= 1 << shift;

		if (iomode & (TIM_IO_PWM | TIM_IO_PWM2) &&
				(ch == 1 || ch == 2)) {
			int pair;

			tim->smcr &= ~(7 << TS);
			tim->smcr |= (ch + 4) << TS;
			tim->smcr &= ~(7 << SMS);
			tim->smcr |= 4 << SMS;

			/* configure buddy channel */
			shift = shift? 0 : 8;
			pair = ch & 1? ch + 1 : ch - 1;
			*reg &= ~(3 << shift);
			*reg |= 2 << shift;
			tim->ccer |= 3 << (pair - 1) * 4;
		}

		/* just in case. it will be updated soon by
		 * __timer_period_set() if the user passed the value for it */
		tim->arr = TIM_COUNTER_MAX - 1;
	}

	tim->ccer |= 1 << (ch - 1) * 4;

	if (dir)
		gpio_init(pin, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	else
		gpio_init(pin, GPIO_MODE_ALT);
}

void __timer_prescale_set(int id, unsigned int v)
{
	struct __timer *tim = __timer_id2reg(id);
	tim->psc = v;
}

void __timer_period_set(int id, unsigned int v)
{
	struct __timer *tim = __timer_id2reg(id);
	tim->arr = v;
}

void __timer_match_set(int id, enum tim_iochannel ch, unsigned int v)
{
	struct __timer *tim;
	unsigned int *reg;

	if (ch < TIM_IO_CH1 || ch > TIM_IO_CH4)
		return;

	tim = __timer_id2reg(id);
	reg = (unsigned int *)&tim->ccr1 + ch - 1;
	*reg = v;
}

void __timer_intr_set(int id, enum tim_iochannel ch)
{
	struct __timer *tim = __timer_id2reg(id);
	tim->dier &= ~(1 << ch);
	tim->dier |= 1 << ch;
}

void __timer_reload(int id)
{
	struct __timer *tim = __timer_id2reg(id);
	tim->egr |= 1;
}

int __timer_open(int id, bool dir, unsigned int hz)
{
	struct __timer *tim;
	int apb_nbit, nvector;

	debug("tim%d %s %x", id, dir? "output" : "input capture", hz);

	switch (id) {
	case 2:
		apb_nbit = 0;
		nvector = 44;
		break;
	case 3:
		apb_nbit = 1;
		nvector = 45;
		break;
	case 4:
		apb_nbit = 2;
		nvector = 46;
		break;
	case 5:
		apb_nbit = 3;
		nvector = 66;
		break;
	default:
		return -ERR_RANGE;
		break;
	}

	tim = __timer_id2reg(id);

	/* enable and reset the peripheral */
	__turn_apb1_clock(apb_nbit, ENABLE);
	__reset_apb1_device(apb_nbit);

	/* default: */
	tim->cr1 = (0 << CMS) | (0 << DIR); /* edge aligned, upcounter */

	if (!dir) { /* Input */
		tim->ccmr1 = 1; /* mapping on channel 1 */
		tim->ccer = 1; /* enable capture on channel 1 */
	}

	if (hz) {
		tim->arr = calc_arr(get_hclk(), hz);
		tim->psc = calc_psc(get_hclk(), hz, tim->arr);
	}

	nvic_set(vec2irq(nvector), true);

	return nvector;
}
