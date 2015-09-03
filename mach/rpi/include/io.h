#ifndef __RPI_IO_H__
#define __RPI_IO_H__

#define PERI_BASE		0x20000000
#define GPIO_BASE		(PERI_BASE + 0x00200000)
#define INTCNTL_BASE		(PERI_BASE + 0x0000b200)
#define SYSTIMER_BASE		(PERI_BASE + 0x00003000)
#define GENTIMER_BASE		(PERI_BASE + 0x0000b400)
#define AUX_BASE		(PERI_BASE + 0x00215000)
#define UART_BASE		(AUX_BASE + 0x40)

struct gpio {
	volatile unsigned int fs0;
	volatile unsigned int fs1;
	volatile unsigned int fs2;
	volatile unsigned int fs3;
	volatile unsigned int fs4;
	volatile unsigned int fs5;
	volatile unsigned int reserved0;
	volatile unsigned int set0;
	volatile unsigned int set1;
	volatile unsigned int reserved1;
	volatile unsigned int clr0;
	volatile unsigned int clr1;
	volatile unsigned int reserved2;
	volatile unsigned int lev0;
	volatile unsigned int lev1;
	volatile unsigned int reserved3;
	volatile unsigned int event0;
	volatile unsigned int event1;
	volatile unsigned int reserved4;
	volatile unsigned int r_edge0;
	volatile unsigned int r_edge1;
	volatile unsigned int resetved5;
	volatile unsigned int f_edge0;
	volatile unsigned int f_edge1;
	volatile unsigned int reserved6;
	volatile unsigned int lev_high0;
	volatile unsigned int lev_high1;
	volatile unsigned int reserved7;
	volatile unsigned int lev_low0;
	volatile unsigned int lev_low1;
	volatile unsigned int reserved8;
	volatile unsigned int r_edge_async0;
	volatile unsigned int r_edge_async1;
	volatile unsigned int reserved9;
	volatile unsigned int f_edge_async0;
	volatile unsigned int f_edge_async1;
	volatile unsigned int reserved10;
	volatile unsigned int pud;
	volatile unsigned int pudclk0;
	volatile unsigned int pudclk1;
} __attribute__((packed));

struct interrupt_controller {
	volatile unsigned int basic_pending;
	volatile unsigned int pending1;
	volatile unsigned int pending2;
	volatile unsigned int fiq_control;
	volatile unsigned int irq1_enable;
	volatile unsigned int irq2_enable;
	volatile unsigned int basic_enable;
	volatile unsigned int irq1_disable;
	volatile unsigned int irq2_disable;
	volatile unsigned int basic_disable;
} __attribute__((packed));

struct system_timer {
	volatile unsigned int status;
	union {
		struct {
			volatile unsigned int counter_h;
			volatile unsigned int counter_l;
		};
		volatile unsigned long long counter;
	};
	volatile unsigned int compare0;
	volatile unsigned int compare1;
	volatile unsigned int compare2;
	volatile unsigned int compare3;
} __attribute__((packed));

struct general_timer {
	volatile unsigned int load;
	volatile unsigned int value;
	volatile unsigned int control;
	volatile unsigned int irq_clear;
	volatile unsigned int irq_raw;
	volatile unsigned int irq_masked;
	volatile unsigned int reload;
	volatile unsigned int divider;
	volatile unsigned int counter;
} __attribute__((packed));

struct aux {
	volatile unsigned int irq;
	volatile unsigned int enable;
} __attribute__((packed));

struct mini_uart {
	volatile unsigned int dr;
	volatile unsigned int ier;
	volatile unsigned int iir;
	volatile unsigned int lcr;
	volatile unsigned int mcr;
	volatile unsigned int lsr;
	volatile unsigned int msr;
	volatile unsigned int scratch;
	volatile unsigned int cntl;
	volatile unsigned int status;
	volatile unsigned int baudrate;
} __attribute__((packed));

#endif /* __RPI_IO_H__ */
