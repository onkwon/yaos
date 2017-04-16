#include <foundation.h>
#include "clock.h"

unsigned int get_pllclk()
{
	return get_hclk();
}

unsigned int get_hclk()
{
	return HWCLOCK;
}

unsigned int get_pclk1()
{
	return get_hclk() / 2;
}

unsigned int get_pclk2()
{
	return get_hclk() / 2;
}

unsigned int get_adclk()
{
	return 0;
}

unsigned int get_stkclk()
{
	unsigned int clk, hclk;

	hclk = get_hclk();

	if (STK_CTRL & 4)
		clk = hclk;
	else
		clk = hclk >> 3;

	return clk;
}

unsigned int get_sysclk_freq()
{
	return get_stkclk();
}

void __turn_apb1_clock(unsigned int nbit, bool on)
{
}

void __turn_apb2_clock(unsigned int nbit, bool on)
{
}

void __turn_ahb1_clock(unsigned int nbit, bool on)
{
}

void __turn_port_clock(reg_t *port, bool on)
{
}

unsigned int __read_apb1_clock()
{
	return 0;
}

unsigned int __read_apb2_clock()
{
	return 0;
}

unsigned int __read_ahb1_clock()
{
	return 0;
}

void __reset_apb1_device(unsigned int nbit)
{
}

void __reset_apb2_device(unsigned int nbit)
{
}
