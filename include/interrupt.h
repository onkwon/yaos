#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#define ISR_REGISTER(vector_nr, func)	({ \
		extern unsigned _sram_start; \
		*((unsigned *)&_sram_start + vector_nr) = (unsigned)func; \
		__asm__ __volatile__ ("dsb"); \
	})

#define SET_IRQ(on, irq_nr) ( \
		*(volatile unsigned *)(NVIC_BASE + ((irq_nr) / 32 * 4)) = \
		MASK_RESET(*(volatile unsigned *)(NVIC_BASE + ((irq_nr) / 32 * 4)), 1 << ((irq_nr) % 32)) \
		| (on << ((irq_nr) % 32)) \
	)

#endif /* __INTERRUPT_H__ */
