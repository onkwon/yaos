#ifndef __STM32F1_FLASH_H__
#define __STM32F1_FLASH_H__

#if 0
/* low-density */
#define FLASH_ADDR_END				0x08007fff /* sector no. 32 */
#elif 1
/* conn or mid-density */
#define FLASH_ADDR_END				0x0801ffff /* sector no. 128 */
#else
/* high-density */
#define FLASH_ADDR_END				0x0807ffff /* sector no. 256 */
#endif
#define NSECTORS				FLASH_ADDR_END

#define FLASH_OPT_UNLOCK_KEY1			0x45670123
#define FLASH_OPT_UNLOCK_KEY2			0xCDEF89AB

enum flash_control_bits {
	BIT_FLASH_PROGRAM			= 0,
	BIT_FLASH_SECTOR_ERASE			= 1,
	BIT_FLASH_MASS_ERASE			= 2,
	BIT_FLASH_OPT_PROGRAM			= 4,
	BIT_FLASH_OPT_BYTE_ERASE		= 5,
	BIT_FLASH_START				= 6,
	BIT_FLASH_LOCK				= 7,
	BIT_FLASH_OPT_BYTE_WRITE_ENABLE		= 9,
	BIT_FLASH_ERR_INT			= 10,
	BIT_FLASH_END_OP_INT			= 12,
};

enum flash_status_bits {
	BIT_FLASH_BUSY				= 0,
	BIT_FLASH_PROG_ERR			= 2,
	BIT_FLASH_WRITE_PROTECTION_ERR		= 4,
	BIT_FLASH_EOP				= 5,
	FLASH_STATUS_MASK			= 0x34,
	FLASH_STATUS_ERROR_MASK			= 0x14,
};

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
	//return 0x08007c00; /* low-density */
	return 0x0801fc00; /* medium-density */
	return 0x0807f800; /* high-density */
	return 0x0803f800; /* connectivity line devices */
	(void)size;
}

static inline void flash_lock_opt()
{
}

static inline void flash_writesize_set(int bits)
{
	(void)bits;
}

static inline int get_sector_size_kb(int sector)
{
	return 1; /* low or medium density */
	return 2; /* high-density or connectivity line devices */
	(void)sector;
}

static inline int addr2sector(void *p)
{
	return (int)p;
}

#endif /* __STM32F1_FLASH_H__ */
