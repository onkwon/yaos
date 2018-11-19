/**
 * @file interrupt.c
 * @brief The file contains interrupt related code
 * @author Kyunghwan Kwon
 */

#include "arch/interrupt.h"
#include "arch/atomic.h"
#include "arch/regs.h"
#include "io.h"
#include "types.h"
#include "log.h"

#include <errno.h>
#include <assert.h>
#include <stdlib.h>

extern long _ram_start;
extern const long _rom_start;
extern void *_ram_end;

void ISR_svc(void);
void ISR_svc_pend(void);
void ISR_dbgmon(void);
void ISR_systick(void);
void ISR_fault(void);

#ifdef CONFIG_COMMON_IRQ_FRAMEWORK
void (*primary_isr_table[PRIMARY_IRQ_MAX])(const int);
#endif
static int (*secondary_isr_table[PRIMARY_IRQ_MAX])(const int, void (*)(const int));

static void ISR_null(const int nvec)
{
	debug("ISR is not yet registered: %x", nvec);
}

void __attribute__((weak)) ISR_svc(void)
{
	ISR_null(11);
}

void __attribute__((weak)) ISR_svc_pend(void)
{
	ISR_null(14);
}

void __attribute__((weak)) ISR_dbgmon(void)
{
	ISR_null(12);
}

void __attribute__((weak)) ISR_systick(void)
{
	ISR_null(15);
}

void __attribute__((weak)) ISR_fault(void)
{
	ISR_null(3);
}

static void (* const vectors[])(void)
#if !defined(TEST)
__attribute__((section(".vector")))
#endif
__attribute__((aligned(4), used)) = {
					/* nVEC   : addr  - desc. */
					/* -------------------- */
	(void (*)(void))&_ram_end,	/* 00     : 0x00  - Stack pointer */
	ISR_reset,			/* 01     : 0x04  - Reset */
	ISR_fault,			/* 02     : 0x08  - NMI */
	ISR_fault,			/* 03     : 0x0c  - HardFault */
	ISR_fault,			/* 04     : 0x10  - MemManage */
	ISR_fault,			/* 05     : 0x14  - BusFault */
	ISR_fault,			/* 06     : 0x18  - UsageFault */
	(void (*)(void))ISR_null,	/* 07     : 0x1c  - Reserved */
	(void (*)(void))ISR_null,	/* 08     : 0x20  - Reserved */
	(void (*)(void))ISR_null,	/* 09     : 0x24  - Reserved */
	(void (*)(void))ISR_null,	/* 10     : 0x28  - Reserved */
	ISR_svc,			/* 11     : 0x2c  - SVCall */
	ISR_dbgmon,			/* 12     : 0x30  - Debug Monitor */
	(void (*)(void))ISR_null,	/* 13     : 0x34  - Reserved */
	ISR_svc_pend,			/* 14     : 0x38  - PendSV */
	ISR_systick,			/* 15     : 0x3c  - SysTick */
};

#if !defined(TEST) && defined(CONFIG_COMMON_IRQ_FRAMEWORK)
static void __attribute__((naked)) ISR_irq(void)
{
	__asm__ __volatile__(
			"sub	sp, sp, #8		\n\t"
			"str	lr, [sp]		\n\t"
			"mrs	r0, ipsr		\n\t"
			"ldr	r1, =primary_isr_table	\n\t"
			"sub	r2, r0, #16		\n\t"
			"ldr	r3, [r1, r2, lsl #2]	\n\t"
			"blx	r3			\n\t"
			"ldr	lr, [sp]		\n\t"
			"add	sp, sp, #8		\n\t"
			"bx	lr			\n\t"
			::: "memory");
}
#else /* !CONFIG_COMMON_IRQ_FRAMEWORK */
static void ISR_irq(void)
{
	ISR_null(__get_psr());
}
#endif

static void (* const irq_vectors[])(void)
#if !defined(TEST)
__attribute__((section(".vector_irq")))
#endif
__attribute__((aligned(4), used)) = {
			/* nVEC(nIRQ): ADDR - DESC */
			/* ----------------------- */
	ISR_irq,	/*  16(0)   : 0x40  - WWDG */
	ISR_irq,	/*  17(1)   : 0x44  - PVD */
	ISR_irq,	/*  18(2)   : 0x48  - TAMPER */
	ISR_irq,	/*  19(3)   : 0x4c  - RTC */
	ISR_irq,	/*  20(4)   : 0x50  - FLASH */
	ISR_irq,	/*  21(5)   : 0x54  - RCC */
	ISR_irq,	/*  22(6)   : 0x58  - EXTI0 */
	ISR_irq,	/*  23(7)   : 0x5c  - EXTI1 */
	ISR_irq,	/*  24(8)   : 0x60  - EXTI2 */
	ISR_irq,	/*  25(9)   : 0x64  - EXTI3 */
	ISR_irq,	/*  26(10)  : 0x68  - EXTI4 */
	ISR_irq,	/*  27(11)  : 0x6c  - DMA1_Channel1 */
	ISR_irq,	/*  28(12)  : 0x70  - DMA1_Channel2 */
	ISR_irq,	/*  29(13)  : 0x74  - DMA1_Channel3 */
	ISR_irq,	/*  30(14)  : 0x78  - DMA1_Channel4 */
	ISR_irq,	/*  31(15)  : 0x7c  - DMA1_Channel5 */
	ISR_irq,	/*  32(16)  : 0x80  - DMA1_Channel6 */
	ISR_irq,	/*  33(17)  : 0x84  - DMA1_Channel7 */
	ISR_irq,	/*  34(18)  : 0x88  - ADC1 | ADC2 */
	ISR_irq,	/*  35(19)  : 0x8c  - USB High Priority | CAN TX */
	ISR_irq,	/*  36(20)  : 0x90  - USB Low Priority | CAN RX0 */
	ISR_irq,	/*  37(21)  : 0x94  - CAN RX1 */
	ISR_irq,	/*  38(22)  : 0x98  - CAN SCE */
	ISR_irq,	/*  39(23)  : 0x9c  - EXTI[9:5] */
	ISR_irq,	/*  40(24)  : 0xa0  - TIM1 Break */
	ISR_irq,	/*  41(25)  : 0xa4  - TIM1 Update */
	ISR_irq,	/*  42(26)  : 0xa8  - TIM1 Trigger | Communication */
	ISR_irq,	/*  43(27)  : 0xac  - TIM1 Capture Compare */
	ISR_irq,	/*  44(28)  : 0xb0  - TIM2 */
	ISR_irq,	/*  45(29)  : 0xb4  - TIM3 */
	ISR_irq,	/*  46(30)  : 0xb8  - TIM4 */
	ISR_irq,	/*  47(31)  : 0xbc  - I2C1 Event */
	ISR_irq,	/*  48(32)  : 0xc0  - I2C1 Error */
	ISR_irq,	/*  49(33)  : 0xc4  - I2C2 Event */
	ISR_irq,	/*  50(34)  : 0xc8  - I2C2 Error */
	ISR_irq,	/*  51(35)  : 0xcc  - SPI1 */
	ISR_irq,	/*  52(36)  : 0xd0  - SPI2 */
	ISR_irq,	/*  53(37)  : 0xd4  - USART1 */
	ISR_irq,	/*  54(38)  : 0xd8  - USART2 */
	ISR_irq,	/*  55(39)  : 0xdc  - USART3 */
	ISR_irq,	/*  56(40)  : 0xe0  - EXTI[15:10] */
	ISR_irq,	/*  57(41)  : 0xe4  - RTC Alarm */
	ISR_irq,	/*  58(42)  : 0xe8  - USB Wakeup */
	ISR_irq,	/*  59(43)  : 0xec  - TIM8 Break */
	ISR_irq,	/*  60(44)  : 0xf0  - TIM8 Update */
	ISR_irq,	/*  61(45)  : 0xf4  - TIM8 Trigger | Communication */
	ISR_irq,	/*  62(46)  : 0xf8  - TIM8 Capture Compare */
	ISR_irq,	/*  63(47)  : 0xfc  - ADC3 */
	ISR_irq,	/*  64(48)  : 0x100 - FSMC */
	ISR_irq,	/*  65(49)  : 0x104 - SDIO */
	ISR_irq,	/*  66(50)  : 0x108 - TIM5 */
	ISR_irq,	/*  67(51)  : 0x10c - SPI3 */
	ISR_irq,	/*  68(52)  : 0x110 - UART4 */
	ISR_irq,	/*  69(53)  : 0x114 - UART5 */
	ISR_irq,	/*  70(54)  : 0x118 - TIM6 */
	ISR_irq,	/*  71(55)  : 0x11c - TIM7 */
	ISR_irq,	/*  72(56)  : 0x120 - DMA2_Channel0 */
	ISR_irq,	/*  73(57)  : 0x124 - DMA2_Channel1 */
	ISR_irq,	/*  74(58)  : 0x128 - DMA2_Channel2 */
	ISR_irq,	/*  75(59)  : 0x12c - DMA2_Channel3 */
	ISR_irq,	/*  76(60)  : 0x130 - DMA2_Channel4 */
	ISR_irq,	/*  77(61)  : 0x134 - Ethernet */
	ISR_irq,	/*  78(62)  : 0x138 - Ethernet wakeup */
	ISR_irq,	/*  79(63)  : 0x13c - CAN2 TX */
	ISR_irq,	/*  80(64)  : 0x140 - CAN2 RX0 */
	ISR_irq,	/*  81(65)  : 0x144 - CAN2 RX1 */
	ISR_irq,	/*  82(66)  : 0x148 - CAN2 SCE */
	ISR_irq,	/*  83(67)  : 0x14c - USB OTG FS */
	ISR_irq,	/*  84(68)  : 0x150 - DMA2 Stream 5 */
	ISR_irq,	/*  85(69)  : 0x154 - DMA2 Stream 6 */
	ISR_irq,	/*  86(70)  : 0x158 - DMA2 Stream 7 */
	ISR_irq,	/*  87(71)  : 0x15c - USART6 */
	ISR_irq,	/*  88(72)  : 0x160 - I2C3 event */
	ISR_irq,	/*  89(73)  : 0x164 - I2C3 error */
	ISR_irq,	/*  90(74)  : 0x168 - USB OTG HS end point 1 out */
	ISR_irq,	/*  91(75)  : 0x16c - USB OTG HS end point 1 in */
	ISR_irq,	/*  92(76)  : 0x170 - USB OTG HS wakeup */
	ISR_irq,	/*  93(77)  : 0x174 - USB OTG HS */
	ISR_irq,	/*  94(78)  : 0x178 - DCMI */
	ISR_irq,	/*  95(79)  : 0x17c - Reserved */
	ISR_irq,	/*  96(80)  : 0x180 - Hash and RNG */
	ISR_irq,	/*  97(81)  : 0x184 - FPU */
	ISR_irq,	/*  98(82)  : 0x188 - UART7 */
	ISR_irq,	/*  99(83)  : 0x18c - UART8 */
	ISR_irq,	/* 100(84)  : 0x190 - SPI4 */
	ISR_irq,	/* 101(85)  : 0x194 - SPI5 */
	ISR_irq,	/* 102(86)  : 0x198 - SPI6 */
	ISR_irq,	/* 103(87)  : 0x19c - SAI1 */
	ISR_irq,	/* 104(88)  : 0x1a0 - LTDC */
	ISR_irq,	/* 105(89)  : 0x1a4 - LTDC error */
	ISR_irq,	/* 106(90)  : 0x1a8 - DMA2D */
	ISR_irq,	/* 107(91)  : 0x1ac - QUADSPI */
	ISR_irq,	/* 108(92)  : 0x1b0 - DSI */
};

#ifdef CONFIG_COMMON_IRQ_FRAMEWORK
static int register_isr_primary(const int lvec, void (*handler)(const int))
{
	int nvec = abs(lvec);

	if (nvec < NVECTOR_IRQ)
		return -EACCES;

	if (nvec >= PRIMARY_IRQ_MAX)
		return -ERANGE;

	long *p = (long *)&primary_isr_table[nvec - NVECTOR_IRQ];

	do {
		void (*f)(void) = (void (*)(void))__ldrex(p);

		if (((long)f != (long)ISR_null)
				&& ((long)handler != (long)ISR_null))
			return -EEXIST;
	} while (__strex(handler, p));

	return 0;
}
#else /* !CONFIG_COMMON_IRQ_FRAMEWORK */
static int register_isr_primary(const int lvec, void (*handler)(const int))
{
	int nvec = abs(lvec);

	if (nvec < NVECTOR_IRQ)
		return -EACCES;

	if (nvec >= PRIMARY_IRQ_MAX)
		return -ERANGE;

	long *p = (long *)&_ram_start;

	p += nvec;

	do {
		void (*f)(int) = (void (*)(int))__ldrex(&p);

		if (((long)f != (long)ISR_null)
				&& ((long)handler != (long)ISR_null))
			return -EEXIST;
	} while (__strex(handler, &p));

	return 0;
}
#endif /* CONFIG_COMMON_IRQ_FRAMEWORK */

/* NOTE: calling multiple handlers is possible chaining handlers to a list,
 * which sounds flexible. but I don't think it would be useful since such use
 * cases don't look nice but causing latency and complexity. */
/* Unregistering can be done to set ctor as intended with force=1 */
int register_isr_register(const int nvec,
		int (*ctor)(const int, void (*)(const int)), const bool force)
{
	if (!is_honored())
		return -EPERM;

	if (abs(nvec) < NVECTOR_IRQ)
		return -EACCES;

	if (abs(nvec) >= PRIMARY_IRQ_MAX)
		return -ERANGE;

	long *p = (long *)&secondary_isr_table[nvec - NVECTOR_IRQ];

	do {
		void (*f)(void) = (void (*)(void))__ldrex(p);

		if (!force && f) {
			debug("already exist or no room");
			return -EEXIST;
		}
	} while (__strex(ctor, p));

	return 0;
}

static int register_isr_secondary(const int lvec, void (*handler)(const int))
{
	int nvec = abs(get_primary_vector(lvec));

	if (nvec < NVECTOR_IRQ)
		return -EACCES;

	if ((nvec >= PRIMARY_IRQ_MAX)
			|| (get_secondary_vector(lvec) >= SECONDARY_IRQ_MAX))
		return -ERANGE;

	int (*f)(const int, void (*)(const int));

	if ((f = secondary_isr_table[nvec - NVECTOR_IRQ]))
		return f(lvec, handler);

	debug("no irq register for %d:%d", nvec, get_secondary_vector(lvec));

	return -ENOENT;
}

int register_isr(const int lvec, void (*handler)(const int))
{
	int ret;

	if (!is_honored())
		return -EPERM;

	if (!handler)
		handler = ISR_null;

	if (abs(lvec) < PRIMARY_IRQ_MAX)
		ret = register_isr_primary(lvec, handler);
	else
		ret = register_isr_secondary(lvec, handler);

	dsb();
	isb();

	return ret;
}

/**
 * @brief Unregister ISR
 * @param lvec Logical vector number
 * @return Return 0 on success or refer to @p errno.h
 */
int unregister_isr(const int lvec)
{
	int ret;

	if (!is_honored())
		return -EPERM;

	if (abs(lvec) < PRIMARY_IRQ_MAX) {
		/* unregister all the secondaries of it */
		for (int i = 0; i < SECONDARY_IRQ_MAX; i++)
			register_isr_secondary(mkvector(lvec, i), ISR_null);

		ret = register_isr_primary(lvec, ISR_null);
		register_isr_register(lvec, NULL, 1);
	} else {
		ret = register_isr_secondary(lvec, ISR_null);
	}

	dsb();
	isb();

	return ret;
}

/**
 * @brief enable or disable nvic interrupts
 * @param nvec vector number
 * @param on @p true for enabling, @p false for disabling
 * @details @p nvec is not IRQ number but exception number
 */
void nvic_set(const int nvec, const bool on)
{
	reg_t *reg;
	unsigned long bit, base;
	unsigned int nirq;
	unsigned int n = abs(nvec);

	if ((n < NVECTOR_IRQ)
			|| (n >= PRIMARY_IRQ_MAX)) {
		debug("%d is out of range", nvec);
		return;
	}

	nirq = vec2irq(n);
	bit  = nirq % 32;
	nirq = nirq / 32 * 4;
	base = on ? NVIC_BASE : NVIC_BASE + 0x80UL;
	reg  = (reg_t *)(base + nirq);

	*reg = 1UL << bit;

	dsb();
}

/**
 * @brief set interrupt priority
 * @param nvec vector number
 * @param pri priority, from @p IRQ_PRIORITY_LOWEST to @p IRQ_PRIORITY_HIGHEST
 */
void nvic_set_pri(const int nvec, const int pri)
{
	reg_t *reg;
	unsigned long bit, val;
	unsigned int n = abs(nvec);

	if (n < NVECTOR_IRQ) {
		reg = (reg_t *)SCB_SHPR;
		reg = &reg[(n >> 2) - 1];
		bit = (n & 3) * 8;
	} else if (n < PRIMARY_IRQ_MAX) {
		unsigned int nirq = vec2irq(n);
		bit = nirq % 4 * 8;
		reg = (reg_t *)((NVIC_BASE + 0x300) + (nirq / 4 * 4));
	} else {
		debug("%d is out of range", nvec);
		return;
	}

	do {
		val = __ldrex(reg);
		val &= ~(0xffUL << bit);
		val |= (pri & 0xfUL) << (bit + 4);
	} while (__strex(val, reg));

	dsb();
}

static inline void __irq_init(void)
{
	for (int i = 0; i < PRIMARY_IRQ_MAX; i++)
		secondary_isr_table[i] = NULL;
}

#include "kernel/init.h"

#ifdef CONFIG_COMMON_IRQ_FRAMEWORK
void __init irq_init(void)
{
	for (int i = 0; i < PRIMARY_IRQ_MAX; i++)
		primary_isr_table[i] = ISR_null;

	__irq_init();

#if !defined(TEST)
	SCB_VTOR = (unsigned long)&_rom_start;
#endif
	dsb();
	isb();
}
#else /* !CONFIG_COMMON_IRQ_FRAMWORK */
void __init irq_init(void)
{
	/* copy interrupt vector table to sram */
	const long *s;
	long *d;

	s = (const long *)&_rom_start;
	d = (long *)&_ram_start;

	while (*s)
		*d++ = *s++;

	__irq_init();

#if !defined(TEST)
	/* activate vector table in sram */
	SCB_VTOR = (unsigned long)&_ram_start;
#endif

	dsb();
	isb();
}
#endif
