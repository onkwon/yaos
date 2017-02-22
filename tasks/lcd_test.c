#include <foundation.h>
#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/systick.h>
#include <asm/pinmap.h>

#define NR_PIXELS		(NR_PIXELS_ROW * NR_PIXELS_COL) /* 76800 */
#define NR_PIXELS_COL		240
#define NR_PIXELS_ROW		320

#define LCD_ALINE_ADDR		0x60000000 /* index register(IR) */
#define LCD_DLINE_ADDR		(0x60000000 | (1 << (19+1))) /* RS(A19) */

#define REG_GRAM_ADDRL		0x20
#define REG_GRAM_ADDRH		0x21
#define REG_GRAM_DATA		0x22

#define MWID			4
#define WREN			12
#define EXTMOD			14
#define CBURSTRW		19

#define ADDSET			0
#define ADDHDL			4
#define DATAST			8

static void lcd_reset(bool high)
{
	gpio_put(PIN_LCD_RESET, !!high);
}

static void lcd_backlight(bool on)
{
	gpio_put(PIN_LCD_BLIGHT, !on);
}

static void lcd_reset_init()
{
	// TODO: init as lowest speed(2MHz) for power consumtion
	gpio_init(PIN_LCD_RESET, GPIO_MODE_OUTPUT);
}

static void lcd_backlight_init()
{
	// init as lowest speed(2MHz)
	gpio_init(PIN_LCD_BLIGHT, GPIO_MODE_OUTPUT);
}

static inline void set_reg_addr(unsigned short int nreg)
{
	volatile unsigned short int *reg =
		(volatile unsigned short int *)LCD_ALINE_ADDR;

	*reg = nreg;
}

static inline void lcd_put_data(int data)
{
	volatile unsigned short int *reg =
		(volatile unsigned short int *)LCD_DLINE_ADDR;

	*reg = data;
}

static inline unsigned short int lcd_get_data()
{
	return *(volatile unsigned short int *)LCD_DLINE_ADDR;
}

static inline unsigned short int lcd_read_reg(unsigned short int nreg)
{
	set_reg_addr(nreg);
	return lcd_get_data();
}

static inline void lcd_write_reg(unsigned short int nreg, unsigned short int v)
{
	set_reg_addr(nreg);
	lcd_put_data(v);
}

static inline void set_gram_addr(unsigned int addr)
{
	lcd_write_reg(REG_GRAM_ADDRL, addr & 0xff);
	lcd_write_reg(REG_GRAM_ADDRH, (addr >> 8) & 0x1ff);
}

static inline void lcd_write_at(unsigned int addr)
{
	set_gram_addr(addr);
	set_reg_addr(REG_GRAM_DATA);
}

static inline void lcd_write_data(int data)
{
	lcd_put_data(data);
}

static void lcd_gpio_init()
{
	gpio_init(PIN_LCD_D0, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D1, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D2, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D3, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D4, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D5, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D6, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D7, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D8, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D9, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D10, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D11, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D12, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D13, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D14, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_D15, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);

	gpio_init(PIN_LCD_RS, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_RD, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_WR, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_CS, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);

#if 0
	gpio_init(PIN_LCD_NBL0, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(PIN_LCD_NBL1, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);

	gpio_init(59, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(60, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(80, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(81, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(82, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(83, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(84, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(85, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(92, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(93, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(94, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(95, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(96, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(97, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(98, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(99, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(100, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
	gpio_init(101, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);
#endif
}

static void lcd_fsmc_init()
{
	lcd_gpio_init();

	__turn_ahb1_clock(8, ENABLE); /* FSMC */

	FSMC2_BCR = (1 << MWID) | (1 << WREN) | (1 << EXTMOD) | (1 << CBURSTRW);
	FSMC2_BTR = (1 << ADDSET) | (1 << ADDHDL) | (8 << DATAST);
	FSMC2_BWTR = (1 << ADDSET) | (1 << ADDHDL) | (3 << DATAST);
	FSMC2_BCR |= 1; /* enable */

	/* ns = 14 @ 72MHz
	 * - BTR
	 * address setup time = 5 / ns + 1
	 * address hold time  = 5 / ns + 1
	 * data setup time = 100 / ns + 1
	 * access mode = A
	 * - BWTR
	 * address setup time = 5 / ns + 1
	 * address hold time  = 5 / ns + 1
	 * data setup time = (20+15) / ns + 1
	 * access mode = A
	 */
}

static inline void __lcd_func_init()
{
	lcd_write_reg(0, 1); /* Start Oscillation */
	mdelay(50);
	lcd_write_reg(0x0001, 0x0100);
	lcd_write_reg(0x0002, 0x0700);
	lcd_write_reg(0x0003, 0x1030); /* Entry mode: AM = 0 */
	lcd_write_reg(0x0004, 0x0000); /* Resize control */
	lcd_write_reg(0x0008, 0x0202); /* Display control 2 */
	lcd_write_reg(0x0009, 0x0000); /* Display control 3 */
	lcd_write_reg(0x000a, 0x0000); /* Display control 4 */
	lcd_write_reg(0x000c, 0x0000);
	lcd_write_reg(0x000d, 0x0000);
	lcd_write_reg(0x000f, 0x0000);
	lcd_write_reg(0x0010, 0x0000);
	lcd_write_reg(0x0011, 0x0000);
	lcd_write_reg(0x0012, 0x0000);
	lcd_write_reg(0x0013, 0x0000);
	mdelay(50);
	lcd_write_reg(0x0010, 0x17b0);
	lcd_write_reg(0x0011, 0x0137);
	mdelay(50);
	lcd_write_reg(0x0012, 0x0139);
	mdelay(50);
	lcd_write_reg(0x0013, 0x1d00);
	lcd_write_reg(0x0029, 0x0011);
	mdelay(50);
	lcd_write_reg(0x0030, 0x0007);
	lcd_write_reg(0x0031, 0x0403);
	lcd_write_reg(0x0032, 0x0404);
	lcd_write_reg(0x0035, 0x0002);
	lcd_write_reg(0x0036, 0x0707);
	lcd_write_reg(0x0037, 0x0606);
	lcd_write_reg(0x0038, 0x0106);
	lcd_write_reg(0x0039, 0x0007);
	lcd_write_reg(0x003c, 0x0700);
	lcd_write_reg(0x003d, 0x0707);

	lcd_write_reg(0x0020, 0x0000);
	lcd_write_reg(0x0021, 0x0000);
	/* window address area; refer to page 72 */
	lcd_write_reg(0x0050, 0x0000); /* Horizontal GRAM start position */
	lcd_write_reg(0x0051, 0x00ef); /* horizontal GRAM end position */
	lcd_write_reg(0x0052, 0x0000); /* Vertical GRAM start position */
	lcd_write_reg(0x0053, 0x013f); /* Vertical GRAM end position */

	lcd_write_reg(0x0060, 0x2700);
	lcd_write_reg(0x0061, 0x0001);
	lcd_write_reg(0x006a, 0x0000); /* VLE: Vertical scroll display enable */
	lcd_write_reg(0x0090, 0x0010);
	lcd_write_reg(0x0092, 0x0000);
	lcd_write_reg(0x0093, 0x0003);
	lcd_write_reg(0x0095, 0x0110);
	lcd_write_reg(0x0097, 0x0110);
	lcd_write_reg(0x0098, 0x0110);

	lcd_write_reg(0x0007, 0x0173); /* display on */
}

static void lcd_init()
{
	lcd_fsmc_init();
	lcd_reset_init();
	lcd_backlight_init();

	lcd_backlight(ON);

	lcd_reset(HIGH);
	mdelay(100);
	lcd_reset(LOW);
	mdelay(100);
	lcd_reset(HIGH);
	mdelay(100);
	mdelay(50);

	__lcd_func_init();
	//lcd_backlight(OFF);
}

static void clear(int color)
{
	int nr_pixel = NR_PIXELS;

	lcd_write_at(0);

	while (nr_pixel--)
		lcd_write_data(color);

	lcd_write_at(0);
}

#include "font.h"

#define pos2pxl(x, y)		(y * 0x100 + x)

typedef struct {
	unsigned short int fg, bg;
} color_t;

typedef struct {
	unsigned int x, y;
} pos_t;

static void lcd_putc(char c, pos_t *pos, color_t *color)
{
	unsigned short int *p;
	int i, j, rgb, y;

	if (c == '\n') {
		pos->y += FONT_ROW;
		pos->x = 0;
		return;
	}

	if (c < 0x20 || c > 0x7e)
		c = '.';

	c -= 32;
	p = &font_table[c * FONT_ROW];

	if ((pos->x + FONT_COL) > NR_PIXELS_COL) {
		pos->x = 0;
		pos->y += FONT_ROW;
	}

	if ((pos->y + FONT_ROW) > NR_PIXELS_ROW)
		pos->y = 0;

	y = pos->y;
	for (i = 0; i < FONT_ROW; i++) {
		lcd_write_at(pos2pxl(pos->x, y));
		for (j = 0; j < FONT_COL; j++) {
			rgb = *p & (1 << j)? color->fg : color->bg;
			lcd_write_data(rgb);
		}

		p++;
		y++;
	}

	pos->x += FONT_COL;
}

static void lcd_puts(const char *s, pos_t *pos, color_t *color)
{
	if (!s)
		return;

	while (*s)
		lcd_putc(*s++, pos, color);
}

static void test_lcd()
{
	lcd_init();

	///////////////////////////////////
	int i = 0;
	pos_t pos, saved;
	color_t color;
	char buf[80];

	pos = (pos_t){0, 0};
	color.fg = 0xffff;
	color.bg = 0x0000;

	clear(0x0000);

	while (1) {
		sprintf(buf, "lcd : %x\n", lcd_read_reg(0));
		lcd_puts(buf, &pos, &color);

		saved = pos;
		pos = (pos_t){NR_PIXELS_COL - FONT_COL, 0};
		color.fg = 0xf800;
		lcd_putc(i + '0', &pos, &color);
		color.fg = 0xffff;
		if (++i > 9) i = 0;
		pos = saved;

		sleep(1);
	}
}
REGISTER_TASK(test_lcd, TASK_KERNEL, DEFAULT_PRIORITY);
