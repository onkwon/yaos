#ifndef __NRF5_CLOCK_H__
#define __NRF5_CLOCK_H__

#define HWCLOCK			64000000 /* 64MHz */
#define HSE			HWCLOCK
#define HSI			8000000	/* 8MHz */

unsigned int get_pllclk();
unsigned int get_sysclk_freq();
unsigned int get_stkclk();
unsigned int get_pclk2();
unsigned int get_pclk1();
unsigned int get_hclk();
unsigned int get_adclk();

void __turn_apb1_clock(unsigned int nbit, bool on);
void __turn_apb2_clock(unsigned int nbit, bool on);
void __turn_ahb1_clock(unsigned int nbit, bool on);
void __turn_port_clock(reg_t *port, bool on);
unsigned int __read_apb1_clock();
unsigned int __read_apb2_clock();
unsigned int __read_ahb1_clock();
void __reset_apb1_device(unsigned int nbit);
void __reset_apb2_device(unsigned int nbit);

void clock_init();

#endif /* __NRF5_CLOCK_H__ */
