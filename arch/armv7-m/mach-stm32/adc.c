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

#include "include/adc.h"
#include <error.h>
#include <io.h>
#include <kernel/timer.h>
#include <kernel/systick.h>

enum reg_index {
	SR	= 0,  /* 0x00 */
	CR1	= 1,  /* 0x04 */
	CR2	= 2,  /* 0x08 */
	SMPR1	= 3,  /* 0x0c */
	SMPR2	= 4,  /* 0x10 */
	SQR1	= 11, /* 0x2c */
	SQR2	= 12, /* 0x30 */
	SQR3	= 13, /* 0x34 */
	DR	= 19, /* 0x4c */
};

enum reg_bit {
	ADON	= 0,
	CAL	= 2,
	EOC	= 1,
	TSVREFE	= 23,
	EXTSEL	= 17,
	SWSTART	= 22,
	EXTTRIG	= 20,
};

#define idx2reg(n)	(reg_t *)(ADC1_BASE + (0x400 * (n - 1)))
#define idx2clk(n)	(n + 8)

void __adc_init(int nadc, int trigger)
{
	reg_t *reg = idx2reg(nadc);

	if (!nadc || nadc >= ADC_MAX)
		return;

	__turn_apb2_clock(idx2clk(nadc), ENABLE);
	__reset_apb2_device(idx2clk(nadc));
	reg[CR2] |= (1 << ADON);
	udelay(1000);
	reg[CR2] |= (1 << CAL); /* calibration */
	while (reg[CR2] & 4) ;
	debug("ADC calibration offset = %d", reg[DR]);

	if (trigger >= ADC_TRIGGER_MAX)
		trigger = ADC_TRIGGER_SOFT;

	reg[CR2] |= trigger << EXTSEL; /* SWSTART */
}

void __adc_temperature_enable()
{
	reg_t *reg = idx2reg(ADC1);
	reg[CR2] |= (1 << TSVREFE);
}

void __adc_temperature_disable()
{
	reg_t *reg = idx2reg(ADC1);
	reg[CR2] &= ~(1 << TSVREFE);
}

void __adc_terminate(int nadc)
{
	reg_t *reg = idx2reg(nadc);

	if (!nadc || nadc >= ADC_MAX)
		return;

	while (reg[SR] & EOC);

	__turn_apb2_clock(idx2clk(nadc), DISABLE);
}

void __adc_sampling_set(int nadc, int ch, int sample_time)
{
#define SAMPLE_TIME_BIT		3
#define SAMPLE_TIME_MASK	7
	reg_t *reg = idx2reg(nadc);
	int bit, i;

	if (!nadc || nadc >= ADC_MAX)
		return;

	if (ch >= 10) {
		ch -= 10;
		i = SMPR1;
	} else
		i = SMPR2;

	if (sample_time >= ADC_SAMPLE_CYCLE_MAX)
		sample_time = ADC_SAMPLE_CYCLE_MAX - 1;

	bit = ch * SAMPLE_TIME_BIT;
	reg[i] &= ~(SAMPLE_TIME_MASK << bit);
	reg[i] |= sample_time << bit;
}

void __adc_channel_set(int nadc, int ch)
{
	reg_t *reg = idx2reg(nadc);
	reg[SQR3] = ch;
	return;

#define CH_BIT	5
#define CH_MASK	0x1f
	int bit, i;

	if (!nadc || nadc >= ADC_MAX)
		return;

	if (ch >= 13) {
		ch -= 13;
		i = SQR1;
	} else if (ch >= 7) {
		ch -= 7;
		i = SQR2;
	} else
		i = SQR3;

	bit = ch * CH_BIT;
	reg[i] &= ~(CH_MASK << bit);
	reg[i] |= ch << bit;
}

int __adc_start(int nadc, unsigned int timeout_ms)
{
	reg_t *reg = idx2reg(nadc);
	int ret;
	unsigned int tout;

	if (!nadc || nadc >= ADC_MAX)
		return -ERANGE;

	if (!timeout_ms)
		timeout_ms = -1; /* infinite */

	set_timeout(&tout, msec_to_ticks(timeout_ms));

	reg[CR2] |= (1 << SWSTART) | (1 << EXTTRIG);
	while (!(reg[SR] & (1 << EOC))) {
		if (is_timeout(tout))
			break;
	}

	ret = reg[DR];

	return ret;
}
