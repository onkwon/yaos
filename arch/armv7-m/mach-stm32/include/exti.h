#ifndef __STM32_EXTI_H__
#define __STM32_EXTI_H__

#include <io.h>
#include <asm/lock.h>

static inline unsigned int get_exti_pending()
{
	return EXTI_PR;
}

static inline void clear_exti_pending(int pin)
{
	BITBAND(&EXTI_PR, pin2portpin(pin), ON);
}

void exti_enable(int pin, bool enable);

#endif /* __STM32_EXTI_H__ */
