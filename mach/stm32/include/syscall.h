#ifndef __STM32_SYSCALL_H__
#define __STM32_SYSCALL_H__

#define svc(n) \
	__asm__ __volatile__("svc %0" :: "I"(n) : "memory")

#endif /* __STM32_SYSCALL_H__ */
