#include <io.h>
#include <kernel/lock.h>
#include <kernel/page.h>
#include <kernel/module.h>
#include <error.h>
#include <stdlib.h>

#include "flash.h"

#define NSECTORS				24

#define FLASH_UNLOCK_KEY1			0x45670123
#define FLASH_UNLOCK_KEY2			0xCDEF89AB

#define FLASH_MASS_ERASE			0xff

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

/* STM32F429ZIT6 flash */
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
	(void)size;
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

static inline void clear_flags()
{
	FLASH_SR |= FLASH_STATUS_MASK;
}

static inline void clear_errflags()
{
	FLASH_SR |= FLASH_STATUS_ERROR_MASK;
}

static inline int get_errflags()
{
	return FLASH_SR & FLASH_STATUS_ERROR_MASK;
}

static inline void flash_wait()
{
	while (FLASH_SR & (1U << BIT_FLASH_BUSY));
}

static inline void flash_unlock()
{
	flash_wait();

	if (FLASH_CR & (1U << BIT_FLASH_LOCK)) {
		FLASH_KEYR = FLASH_UNLOCK_KEY1;
		FLASH_KEYR = FLASH_UNLOCK_KEY2;
	}

	while (FLASH_CR & (1U << BIT_FLASH_LOCK));
}

static inline void flash_lock()
{
	FLASH_CR |= 1U << BIT_FLASH_LOCK;
}

static inline void flash_erase_sector(int nr)
{
	unsigned int tmp;

	flash_wait();

	if (nr >= 12)
		nr = (nr - 12) | 0x10;

	tmp = FLASH_CR;
	tmp &= ~(0x1f << BIT_FLASH_SECTOR_NR);
	tmp |= (1U << BIT_FLASH_SECTOR_ERASE) | (nr << BIT_FLASH_SECTOR_NR);
	tmp |= 1U << BIT_FLASH_START;
	FLASH_CR = tmp;

	flash_wait();

	debug("erase sector %d", nr);
}

static inline void flash_erase_all()
{
	unsigned int tmp;

	flash_wait();

	tmp = FLASH_CR;
	tmp |= (1U << BIT_FLASH_MASS_ERASE) | (1U << BIT_FLASH_MASS_ERASE2);
	tmp |= 1U << BIT_FLASH_START;
	FLASH_CR = tmp;

	flash_wait();

	debug("erase all banks and sectors");
}

static inline int flash_erase(int nr)
{
	/* FIXME: make sure that no data in the sector is cached */

	if (nr == FLASH_MASS_ERASE) {
		flash_erase_all();
		return get_errflags();
	}

	if (nr >= NSECTORS) {
		debug("no existing sector %d", nr);
		return -ERANGE;
	}

	flash_erase_sector(nr);

	return get_errflags();
}

static inline void flash_prepare()
{
	clear_flags();
	flash_unlock();
	flash_writesize_set(WORD_BITS);
	FLASH_CR |= 1U << BIT_FLASH_PROGRAM;
	flash_wait();
}

static inline void flash_finish()
{
	FLASH_CR &= ~(1U << BIT_FLASH_PROGRAM);
	flash_lock();
}

static inline bool flash_write_word(unsigned int *dst, const unsigned int *src)
{
	*dst = *(volatile unsigned int *)src;
	flash_wait();

	if (get_errflags() || *(volatile unsigned int *)dst != *src)
		return false;

	return true;
}

static size_t __attribute__((section(".iap")))
flash_write_core(void * const addr, const void * const buf, size_t len,
		bool overwrite)
{
	const unsigned int *src, *new, *restore;
	unsigned int *dst;
	unsigned int base, tmp;
	int s, ss, diff, left, t;
	unsigned int new_start, new_end;

	len = (len / 4) + !!(len % 4); /* bytes to word */
	left = len;
	dst = addr;
	src = buf;
	ss = diff = 0;
	new = NULL;
	new_start = new_end = 0;

	flash_prepare();
retry:
	while (left) {
		if ((unsigned int)dst >= new_start &&
				(unsigned int)dst < new_end) {
			if (!flash_write_word(dst, new))
				break;
			new++;
			diff = 0;
		} else if (diff < 0) { /* Restore the fore data */
			if (!flash_write_word(dst, restore))
				break;
			restore++;
		} else {
			if (!flash_write_word(dst, src))
				break;
			src++;
		}

		dst++;
		left--;
	}

	if (left) {
		s = addr2sector(dst);
		ss = get_sector_size_kb(s) << 10; /* error if 0 */
		base = BASE_ALIGN((unsigned int)dst, ss);
		diff = (int)((unsigned int)dst - base) / 4;
		if ((unsigned int)addr > base) {
			new_start = (unsigned int)addr;
			new_end = min(base + ss, new_start + len * 4);
			new = &((unsigned int *)buf)
				[((unsigned int)dst - (unsigned int)addr) / 4];
			diff = (int)(base - (unsigned int)addr) / 4;
			left = len;
		} else {
			new_start = base;
			new_end = min(base + ss, new_start + (left + diff) * 4);
			new = src - diff;
		}
		dst = (unsigned int *)base;
		src = new + (new_end - new_start) / 4;
		left += abs(diff);

		if (!overwrite) { /* Save the sector in a temporal sector */
			tmp = get_temporal_sector_addr(ss);

			if (flash_write_core((void *)tmp, (void *)base, ss, true) != ss)
				goto out;

			restore = (unsigned int *)tmp;
			t = (int)((base + left * 4) - (base + ss));
			if (t < 0) { /* Restore the rear data */
				src = (unsigned int *)(tmp + (ss - abs(t)));
				left += abs(t) / 4;
			}

			flash_prepare();
		}

		if (flash_erase(s))
			goto cleanout;

		goto retry;
	}

cleanout:
	flash_finish();
out:
	dsb();
	isb();

	return (len - left) * 4;
}

size_t flash_program(void * const addr, const void * const buf, size_t len,
		bool overwrite)
{
	size_t written;

	if (!is_honored()) {
		debug("no permission");
		return 0;
	}

	cli();
	written = flash_write_core(addr, buf, len, overwrite);
	sei();

	return written;
}

static size_t flash_write(struct file *file, void *buf, size_t len)
{
	unsigned int addr;

	addr = file->offset;
	return flash_write_core((void * const)addr, (const void * const)buf,
			len, false);
}

static inline size_t flash_read_core(void * const addr, void *buf, size_t len)
{
	unsigned int *s = (unsigned int *)addr;
	unsigned int *d = (unsigned int *)buf;

	len /= WORD_SIZE;

	while (len--)
		*d++ = *s++;

	return (size_t)((unsigned int)s - (unsigned int)addr);
}

static size_t flash_read(struct file *file, void *buf, size_t len)
{
	/* check boundary */

	unsigned int addr;

	addr = file->offset;
	return flash_read_core((void * const)addr, buf, len);
}

static int flash_seek(struct file *file, unsigned int offset, int whence)
{
	unsigned int end;
	struct device *dev;

	switch (whence) {
	case SEEK_SET:
		file->offset = offset;
		break;
	case SEEK_CUR:
		if (file->offset + offset < file->offset)
			return -ERANGE;

		file->offset += offset;
		break;
	case SEEK_END:
		dev = getdev(file->inode->dev);
		end = dev->block_size * dev->nr_blocks - 1;
		end = BASE_WORD(end + dev->base_addr);

		if (end - offset < dev->base_addr)
			return -ERANGE;

		file->offset = end - offset;
		break;
	}

	return 0;
}

static struct file_operations ops = {
	.open  = NULL,
	.read  = flash_read,
	.write = flash_write,
	.close = NULL,
	.seek  = flash_seek,
	.ioctl = NULL,
};

#include <kernel/buffer.h>

static int flash_init()
{
	extern char _rom_size, _rom_start, _etext, _data, _ebss;
	struct device *dev;
	unsigned int end;

	/* whole disk of embedded flash memory */
	if (!(dev = mkdev(0, 0, &ops, "efm")))
		return -ERANGE;

	dev->block_size = SECTOR_SIZE;
	dev->nr_blocks = (unsigned int)&_rom_size / SECTOR_SIZE;

	/* partition for file system */
	if (!(dev = mkdev(MAJOR(dev->id), 1, &ops, "efm")))
		return -ERANGE;

	dev->block_size = SECTOR_SIZE;
	dev->base_addr = (unsigned int)&_etext +
		((unsigned int)&_ebss - (unsigned int)&_data);
	dev->base_addr = ALIGN(dev->base_addr, SECTOR_SIZE);

	end = (unsigned int)&_rom_start + (unsigned int)&_rom_size;
	dev->nr_blocks = (end - dev->base_addr) / SECTOR_SIZE;

	mount(dev, "/", "embedfs");

	return 0;
}
MODULE_INIT(flash_init);

#include <kernel/power.h>

void flash_protect()
{
#if (SOC == stm32f1 || SOC == stm32f3)
	if (FLASH_OPT_RDP != 0x5aa5)
		return;
#else
	if (((FLASH_OPTCR >> 8) & 0xff) != 0xaa)
		return;
#endif

	warn("Protect flash memory from externel accesses");

	while (FLASH_SR & (1 << BSY));

	FLASH_UNLOCK();
	FLASH_UNLOCK_OPTPG();

#if (SOC == stm32f1 || SOC == stm32f3)
	FLASH_CR |= 0x20; /* OPTER */
	FLASH_CR |= 1 << STRT;
#else
	FLASH_OPTCR &= ~(0xff << 8);
	FLASH_OPTCR |= 2; /* set start bit */
#endif

	while (FLASH_SR & (1 << BSY));

#if (SOC == stm32f1 || SOC == stm32f3)
	FLASH_CR &= ~0x20; /* OPTER */
#else
	FLASH_OPTCR &= ~2;
#endif

	FLASH_LOCK_OPTPG();
	FLASH_LOCK();

	reboot();
}
