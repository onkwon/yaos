#ifndef __RPI_IO_H__
#define __RPI_IO_H__

#ifndef bcm2835
#define bcm2835			1
#define bcm2836			2
#endif

#if (SOC == bcm2835)
#define PERI_BASE		0x20000000
#elif (SOC == bcm2836)
#define PERI_BASE		0x3f000000
#endif
#define GPIO_BASE		(PERI_BASE + 0x00200000)
#define INTCNTL_BASE		(PERI_BASE + 0x0000b200)
#define SYSTIMER_BASE		(PERI_BASE + 0x00003000)
#define GENTIMER_BASE		(PERI_BASE + 0x0000b400)
#define AUX_BASE		(PERI_BASE + 0x00215000)
#define UART_BASE		(AUX_BASE + 0x40)

#include <types.h>

struct gpio {
	reg_t fs0;
	reg_t fs1;
	reg_t fs2;
	reg_t fs3;
	reg_t fs4;
	reg_t fs5;
	reg_t reserved0;
	reg_t set0;
	reg_t set1;
	reg_t reserved1;
	reg_t clr0;
	reg_t clr1;
	reg_t reserved2;
	reg_t lev0;
	reg_t lev1;
	reg_t reserved3;
	reg_t event0;
	reg_t event1;
	reg_t reserved4;
	reg_t r_edge0;
	reg_t r_edge1;
	reg_t resetved5;
	reg_t f_edge0;
	reg_t f_edge1;
	reg_t reserved6;
	reg_t lev_high0;
	reg_t lev_high1;
	reg_t reserved7;
	reg_t lev_low0;
	reg_t lev_low1;
	reg_t reserved8;
	reg_t r_edge_async0;
	reg_t r_edge_async1;
	reg_t reserved9;
	reg_t f_edge_async0;
	reg_t f_edge_async1;
	reg_t reserved10;
	reg_t pud;
	reg_t pudclk0;
	reg_t pudclk1;
} __attribute__((packed));

struct interrupt_controller {
	reg_t basic_pending;
	reg_t pending1;
	reg_t pending2;
	reg_t fiq_control;
	reg_t irq1_enable;
	reg_t irq2_enable;
	reg_t basic_enable;
	reg_t irq1_disable;
	reg_t irq2_disable;
	reg_t basic_disable;
} __attribute__((packed));

struct system_timer {
	reg_t status;
	union {
		struct {
			reg_t counter_h;
			reg_t counter_l;
		};
		volatile unsigned long long counter;
	};
	reg_t compare0;
	reg_t compare1;
	reg_t compare2;
	reg_t compare3;
} __attribute__((packed));

struct general_timer {
	reg_t load;
	reg_t value;
	reg_t control;
	reg_t irq_clear;
	reg_t irq_raw;
	reg_t irq_masked;
	reg_t reload;
	reg_t divider;
	reg_t counter;
} __attribute__((packed));

struct aux {
	reg_t irq;
	reg_t enable;
} __attribute__((packed));

struct mini_uart {
	reg_t dr;
	reg_t ier;
	reg_t iir;
	reg_t lcr;
	reg_t mcr;
	reg_t lsr;
	reg_t msr;
	reg_t scratch;
	reg_t cntl;
	reg_t status;
	reg_t baudrate;
} __attribute__((packed));

#endif /* __RPI_IO_H__ */
