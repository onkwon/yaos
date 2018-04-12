#ifndef __STM32F4_FLASH_H__
#define __STM32F4_FLASH_H__

#define NSECTORS				24

#define FLASH_OPT_UNLOCK_KEY1			0x08192A3B
#define FLASH_OPT_UNLOCK_KEY2			0x4C5D6E7F

enum flash_control_bits {
	BIT_FLASH_PROGRAM			= 0,
	BIT_FLASH_SECTOR_ERASE			= 1,
	BIT_FLASH_MASS_ERASE			= 2,
	BIT_FLASH_SECTOR_NR			= 3,
	BIT_FLASH_PROGRAM_SIZE			= 8,
	BIT_FLASH_MASS_ERASE2			= 15,
	BIT_FLASH_START				= 16,
	BIT_FLASH_END_OP_INT			= 24,
	BIT_FLASH_ERR_INT			= 25,
	BIT_FLASH_LOCK				= 31,
};

enum flash_status_bits {
	BIT_FLASH_EOP				= 0,
	BIT_FLASH_OPERATION_ERR			= 1,
	BIT_FLASH_WRITE_PROTECTION_ERR		= 4,
	BIT_FLASH_PROG_ALIGN_ERR		= 5,
	BIT_FLASH_PROG_PARALLELISM_ERR		= 6,
	BIT_FLASH_PROG_SEQ_ERR			= 7,
	BIT_FLASH_READOUT_PROTECTION_ERR	= 8,
	BIT_FLASH_BUSY				= 16,
	FLASH_STATUS_MASK			= 0x1f3,
	FLASH_STATUS_ERROR_MASK			= 0x1f2,
};

/* stm32f429ZIT6 flash */
/* 2 Mbyte dual bank organization
 * [Bank 1]
 * Sector 0  | 0x0800_0000 - 0x0800_3FFF | 16Kbytes
 * Sector 1  | 0x0800_4000 - 0x0800_7FFF | 16Kbytes
 * Sector 2  | 0x0800_8000 - 0x0800_BFFF | 16Kbytes
 * Sector 3  | 0x0800_C000 - 0x0800_FFFF | 16Kbytes
 * Sector 4  | 0x0801_0000 - 0x0801_FFFF | 64Kbytes
 * Sector 5  | 0x0802_0000 - 0x0803_FFFF | 128Kbytes
 * Sector 6  | 0x0804_0000 - 0x0805_FFFF | 128Kbytes
 * Sector 7  | 0x0806_0000 - 0x0807_FFFF | 128Kbytes
 * Sector 8  | 0x0808_0000 - 0x0809_FFFF | 128Kbytes
 * Sector 9  | 0x080A_0000 - 0x080B_FFFF | 128Kbytes
 * Sector 10 | 0x080C_0000 - 0x080D_FFFF | 128Kbytes
 * Sector 11 | 0x080E_0000 - 0x080F_FFFF | 128Kbytes
 * [Bank 2]
 * Sector 12 | 0x0810_0000 - 0x0810_3FFF | 16Kbytes
 * Sector 13 | 0x0810_4000 - 0x0810_7FFF | 16Kbytes
 * Sector 14 | 0x0810_8000 - 0x0810_BFFF | 16Kbytes
 * Sector 15 | 0x0810_C000 - 0x0810_FFFF | 16Kbytes
 * Sector 16 | 0x0811_0000 - 0x0811_FFFF | 64Kbytes
 * Sector 17 | 0x0812_0000 - 0x0813_FFFF | 128Kbytes
 * Sector 18 | 0x0814_0000 - 0x0815_FFFF | 128Kbytes
 * Sector 19 | 0x0816_0000 - 0x0817_FFFF | 128Kbytes
 * Sector 20 | 0x0818_0000 - 0x0819_FFFF | 128Kbytes
 * Sector 21 | 0x081A_0000 - 0x081B_FFFF | 128Kbytes
 * Sector 22 | 0x081C_0000 - 0x081D_FFFF | 128Kbytes
 * Sector 23 | 0x081E_0000 - 0x081F_FFFF | 128Kbytes
 * [System memory]
 * 0x1FFF_0000 - 0x1FFF_77FF | 30 Kbytes
 * [OTP]
 * 0x1FFF_7800 - 0x1FFF_7A0F | 528 bytes
 * [Option bytes]
 * Bank 1 | 0x1FFF_C000 - 0x1FFF_C00F | 16bytes
 * Bank 2 | 0x1FFE_C000 - 0x1FFE_C00F | 16bytes
 */

static inline unsigned int get_temporal_sector_addr(int size)
{
	/* TODO: do management algorithm to maximize lifetime
	 * 1. when booting read all sectors and make a mapping of free sectors
	 * 2. allocate random sectors in the mapping
	 * 3. temporal sector and all the unused sector should be erased to be
	 *    checked as free sector when booting
	 *
	 * or
	 *
	 * Reserve a flash sector to save mapping */
	return 0x081e0000;
	(void)size;
}

static inline int get_sector_size_kb(int sector)
{
	if ((sector >= 0 && sector < 4) || /* bank 1 */
			(sector >= 12 && sector < 16)) /* bank 2 */
		return 16;
	else if (sector == 4 || sector == 16)
		return 64;
	else if ((sector >= 5 && sector <= 11) ||
			(sector >= 17 && sector <= 23))
		return 128;

	return 0;
}

static inline int addr2sector(void *p)
{
	unsigned int addr = (unsigned int)p;
	int sector;

	if (addr & 0xe0000) /* sector[4|16] ~ sector[11|23] */
		sector = ((addr & 0xe0000) >> 17) + 4;
	else
		sector = ((addr & 0x1f000) >> 14);

	if (addr & 0x100000)
		sector += 12;

	return sector;
}

static inline void flash_writesize_set(int bits)
{
	unsigned int tmp;

	tmp = FLASH_CR;
	tmp &= ~(3U << BIT_FLASH_PROGRAM_SIZE);
	tmp |= (bits >> 4) << BIT_FLASH_PROGRAM_SIZE;

	FLASH_CR = tmp;
}

static inline int flash_writesize_get()
{
	int bits;

	switch (FLASH_CR & (3U << BIT_FLASH_PROGRAM_SIZE)) {
	case 1:
		bits = 16;
		break;
	case 2:
		bits = 32;
		break;
	case 3:
		bits = 64;
		break;
	case 0:
	default:
		bits = 8;
		break;
	}

	return bits;
}

static inline void flash_lock_opt()
{
	FLASH_OPTCR |= 1;
}

#endif /* __STM32F4_FLASH_H__ */
