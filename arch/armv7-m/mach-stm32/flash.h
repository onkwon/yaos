#ifndef __STM32_FLASH_H__
#define __STM32_FLASH_H__

#ifndef stm32f1
#define stm32f1	1
#define stm32f3	3
#define stm32f4	4
#endif

#define PER			1
#define PG			0

#define FLASH_WRITE_START()		(FLASH_CR |=   1 << PG)
#define FLASH_WRITE_END()		(FLASH_CR &= ~(1 << PG))

#if (SOC == stm32f1 || SOC == stm32f3)
#define BLOCK_SIZE		2048

#define STRT			6
#define BSY			0

#define FLASH_WRITE_WORD(addr, data)	{ \
	*(volatile unsigned short int *)(addr) = (unsigned short int)(data); \
	while (FLASH_SR & (1 << BSY)); \
	*(volatile unsigned short int *)((unsigned int)(addr)+2) \
		= (unsigned short int)((data) >> 16); \
	while (FLASH_SR & (1 << BSY)); \
}
#define FLASH_LOCK()			(FLASH_CR |= 0x80)
#define FLASH_UNLOCK() { \
	if (FLASH_CR & 0x80) { \
		FLASH_KEYR = 0x45670123; /* KEY1 */ \
		FLASH_KEYR = 0xcdef89ab; /* KEY2 */ \
	} \
}
#define FLASH_UNLOCK_OPTPG() { \
	FLASH_OPTKEYR = 0x45670123; /* KEY1 */ \
	FLASH_OPTKEYR = 0xcdef89ab; /* KEY2 */ \
}
#define FLASH_LOCK_OPTPG()

#define WRITE_WORD(addr, data)	{ \
	FLASH_SR |= 0x34; \
	FLASH_WRITE_WORD(addr, data); \
}
#elif (SOC == stm32f4) /* stm32f4 */
#define BLOCK_SIZE		16384

#define STRT			16
#define SNB			3
#define PSIZE			8
#define BSY			16

#define FLASH_WRITE_WORD(addr, data)	{ \
	*(reg_t *)(addr) = (unsigned int)(data); \
	while (FLASH_SR & (1 << BSY)); /* Check BSY bit, need timeout */ \
}
#define FLASH_LOCK()			(FLASH_CR |= 0x80000000)
#define FLASH_UNLOCK() { \
	if (FLASH_CR & 0x80000000) { \
		FLASH_KEYR = 0x45670123; /* KEY1 */ \
		FLASH_KEYR = 0xcdef89ab; /* KEY2 */ \
	} \
}
#define FLASH_UNLOCK_OPTPG() { \
	FLASH_OPTKEYR = 0x08192a3b; /* KEY1 */ \
	FLASH_OPTKEYR = 0x4c5d6e7f; /* KEY2 */ \
}
#define FLASH_LOCK_OPTPG()		(FLASH_OPTCR |= 1)

#define WRITE_WORD(addr, data)	{ \
	FLASH_CR &= ~(3 << PSIZE); \
	FLASH_CR |= (2 << PSIZE); \
	FLASH_WRITE_START(); \
	FLASH_SR |= 0xf1; \
	FLASH_WRITE_WORD(addr, data); \
	FLASH_WRITE_END(); \
}
#endif

#endif /* __STM32_FLASH_H__ */
