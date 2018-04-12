#ifndef __STM32_FLASH_H__
#define __STM32_FLASH_H__

#define FLASH_UNLOCK_KEY1			0x45670123
#define FLASH_UNLOCK_KEY2			0xCDEF89AB

#define FLASH_MASS_ERASE			0xFFFFFFFF

#if defined(stm32f1) || defined(stm32f3)
#include "flash_f1.h"
#elif defined(stm32f4)
#include "flash_f4.h"
#else
#error undefined machine
#endif

#endif /* __STM32_FLASH_H__ */
