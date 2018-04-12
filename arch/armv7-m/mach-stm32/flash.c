/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#include <io.h>
#include <kernel/lock.h>
#include <kernel/page.h>
#include <kernel/module.h>
#include <error.h>
#include <stdlib.h>

#include "flash.h"

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

static inline void flash_unlock_opt()
{
	FLASH_OPTKEYR = FLASH_OPT_UNLOCK_KEY1;
	FLASH_OPTKEYR = FLASH_OPT_UNLOCK_KEY2;
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

#if defined(stm32f4)
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

static inline bool flash_write_word(unsigned int *dst, const unsigned int *src)
{
	*dst = *(volatile unsigned int *)src;
	flash_wait();

	if (get_errflags() || *(volatile unsigned int *)dst != *src)
		return false;

	return true;
}
#elif defined(stm32f1) || defined(stm32f3)
static inline void flash_erase_sector(int addr)
{
	FLASH_CR &= ~(1U << BIT_FLASH_PROGRAM);

	flash_wait();
	FLASH_CR |= 1U << BIT_FLASH_SECTOR_ERASE;
	FLASH_AR = (unsigned int)addr;
	FLASH_CR |= 1U << BIT_FLASH_START;
	flash_wait();
	FLASH_CR &= ~(1U << BIT_FLASH_SECTOR_ERASE);

	FLASH_CR |= 1U << BIT_FLASH_PROGRAM;
}

static inline void flash_erase_all()
{
	FLASH_CR &= ~(1U << BIT_FLASH_PROGRAM);

	flash_wait();
	FLASH_CR |= 1U << BIT_FLASH_MASS_ERASE;
	FLASH_CR |= 1U << BIT_FLASH_START;
	flash_wait();
	FLASH_CR &= ~(1U << BIT_FLASH_MASS_ERASE);

	FLASH_CR |= 1U << BIT_FLASH_PROGRAM;
}

static inline bool flash_write_word(unsigned int *dst, const unsigned int *src)
{
	unsigned int addr = (unsigned int)dst;
	unsigned short int t;

	t = (unsigned short int)*src;
	*(volatile unsigned short int *)(addr) = t;
	flash_wait();
	t = (unsigned short int)(*src >> 16);
	*(volatile unsigned short int *)(addr+2) = (unsigned short int)(*src >> 16);
	flash_wait();

	if (get_errflags() || *(volatile unsigned int *)dst != *src)
		return false;

	return true;
}
#else
#error undefined machine
#endif

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
		} else
			clear_flags();

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

#define SECTOR_SIZE		16384

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
#if defined(stm32f1) || defined(stm32f3)
	if (FLASH_OPT_RDP != 0x5aa5)
		return;
#elif defined(stm32f4)
	if (((FLASH_OPTCR >> 8) & 0xff) != 0xaa)
		return;
#else
#error undefined machine
#endif

	warn("Protect flash memory from externel accesses");

	flash_unlock();
	flash_unlock_opt();

#if defined(stm32f1) || defined(stm32f3)
	FLASH_CR |= 1U << BIT_FLASH_OPT_BYTE_ERASE;
	FLASH_CR |= 1U << BIT_FLASH_START;
#elif defined(stm32f4)
	FLASH_OPTCR &= ~(0xffU << 8);
	FLASH_OPTCR |= 2U; /* set start bit */
#else
#error undefined machine
#endif

	while (FLASH_SR & (1U << BIT_FLASH_BUSY));

#if defined(stm32f1) || defined(stm32f3)
	FLASH_CR &= ~(1U << BIT_FLASH_OPT_BYTE_ERASE);
#elif defined(stm32f4)
	FLASH_OPTCR &= ~2U;
#else
#error undefined machine
#endif

	flash_lock_opt();
	flash_lock();

	reboot();
}
