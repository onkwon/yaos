#ifndef __RPI_INTERRUPT_H__
#define __RPI_INTERRUPT_H__

/*
 * IRQ:
 * 0	- ARM Timer                 -
 * 1	- ARM Mailbox               |
 * 2	- ARM Doorbell 0            |
 * 3	- ARM Doorbell 1            |
 * 4	- GPU0 halted               | basic interrupts
 * 5	- GPU1 halted               |
 * 6	- Illegal access type 1     |
 * 7	- Illegal access type 0     -
 * ...
 * 29	- Aux int(mini uart)
 * ...
 * 43	- i2c_spi_slv_int
 * ...
 * 45	- pwa0
 * 46	- pwa1
 * ...
 * 48	- smi
 * 49	- gpio_int[0]
 * 50	- gpio_int[1]
 * 51	- gpio_int[2]
 * 52	- gpio_int[3]
 * 53	- i2c_int
 * 54	- spi_int
 * 55	- pcm_int
 * ...
 * 57	- uart_int
 */

#define IRQ_TIMER			0
#define IRQ_PEND1			8
#define IRQ_PEND2			9
#define IRQ_AUX				29
#define IRQ_GPIO0			49
#define IRQ_GPIO1			50
#define IRQ_GPIO2			51
#define IRQ_GPIO3			52
#define IRQ_UART			57
#define IRQ_MAX				64

extern inline void __sei();
extern inline void __cli();

#define __dmb()								\
	__asm__ __volatile__("mcr p15, 0, r0, c7, c10, 5" ::: "memory")
#define __dsb()								\
	__asm__ __volatile__("mcr p15, 0, r0, c7, c10, 4" ::: "memory")
#define __isb()								\
	__asm__ __volatile__("mcr p15, 0, r0, c7, c5, 4" ::: "memory")

#define __irq_save(flag)						\
	__asm__ __volatile__("mrs %0, cpsr" : "=r"(flag) :: "memory")
#define __irq_restore(flag)						\
	__asm__ __volatile__("msr cpsr_c, %0" :: "r"(flag) : "memory")

#define __get_active_irq()

#define GET_CNTL()			({				\
	unsigned int __cpsr; 						\
	__asm__ __volatile__("mrs %0, cpsr" : "=r"(__cpsr));		\
	__cpsr; 							\
})
#define GET_PC()			({				\
	unsigned int __pc;						\
	__asm__ __volatile__("mov %0, pc" : "=r"(__pc));		\
	__pc;								\
})
#define GET_SP()			({				\
	unsigned int __sp;						\
	__asm__ __volatile__("mov %0, sp" : "=r"(__sp));		\
	__sp;								\
})
#define GET_KSP()			GET_SP()
#define GET_USP()			({				\
	unsigned int __sp;						\
	if (GET_CNTL() & 0xf) {						\
		__asm__ __volatile__(					\
			"mov	r0, #0xdf	@ system mode	\n\t"	\
			"mrs	r1, cpsr	@ backup	\n\t"	\
			"msr	cpsr_c, r0			\n\t"	\
			"mov	r0, sp				\n\t"	\
			"msr	cpsr, r1	@ return	\n\t"	\
			"mov	%0, r0				\n\t"	\
			: "=r"(__sp) :: "r0", "r1", "memory");		\
	} else {							\
		__asm__ __volatile__(					\
			"mov %0, sp" : "=r"(__sp));			\
	}								\
	__sp;								\
})
#define GET_LR()			({				\
	unsigned int __lr;						\
	__asm__ __volatile__("mov %0, lr" : "=r"(__lr));		\
	__lr;								\
})
#define SET_LR(reg)							\
	__asm__ __volatile__("mov lr, %0" :: "r"(reg))
#define SET_PC(addr)			({				\
	__asm__ __volatile__(						\
		"push {%0}	\n\t"					\
		"pop {pc}	\n\t"					\
		:: "r"(addr) : "memory");				\
})
#define SET_SP(sp)							\
	__asm__ __volatile__("mov sp, %0" :: "r"(sp) : "memory")
#define SET_USP(sp)			({				\
	if (GET_CNTL() & 0xf) {						\
		__asm__ __volatile__(					\
			"mov	r0, #0xdf	@ system mode	\n\t"	\
			"mrs	r1, cpsr	@ backup	\n\t"	\
			"msr	cpsr_c, r0			\n\t"	\
			"mov	sp, %0				\n\t"	\
			"msr	cpsr, r1	@ return	\n\t"	\
			:: "r"(sp) : "r0", "r1", "memory");		\
	} else {							\
		__asm__ __volatile__(					\
			"mov sp, %0" :: "r"(sp) : "memory"); 		\
	}								\
})
#define __set_usp(sp)			SET_USP(sp)
#define __set_ksp(sp)			SET_SP(sp)

#define __get_ret_addr()		GET_LR()
#define __get_pc()			GET_PC()
#define __get_usp()			GET_USP()
#define __get_sp()			GET_SP()
#define __set_sp(sp)			SET_SP(sp)
#define __set_pc(addr)			SET_PC(addr)

#define __nop()								\
	__asm__ __volatile__("nop" ::: "memory")
#define __ret()								\
	__asm__ __volatile__("bx lr" ::: "memory")
#define __ret_from_irq()						\
	__asm__ __volatile__("subs pc, lr, #4" ::: "memory")
#define __ret_from_exc(offset)						\
	__asm__ __volatile__("subs pc, lr, %0" :: "I"(offset): "memory")

#endif /* __RPI_INTERRUPT_H__ */
