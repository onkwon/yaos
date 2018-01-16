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
	gpio_init(PIN_LCD_RESET, GPIO_MODE_OUTPUT);
}

static void lcd_backlight_init()
{
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
	gpio_init(PIN_LCD_D0, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D1, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D2, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D3, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D4, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D5, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D6, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D7, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D8, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D9, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D10, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D11, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D12, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D13, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D14, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_D15, GPIO_MODE_ALT | GPIO_SPD_FASTER);

	gpio_init(PIN_LCD_RS, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_RD, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_WR, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_CS, GPIO_MODE_ALT | GPIO_SPD_FASTER);

#if 0
	gpio_init(PIN_LCD_NBL0, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(PIN_LCD_NBL1, GPIO_MODE_ALT | GPIO_SPD_FASTER);

	gpio_init(59, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(60, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(80, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(81, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(82, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(83, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(84, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(85, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(92, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(93, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(94, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(95, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(96, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(97, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(98, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(99, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(100, GPIO_MODE_ALT | GPIO_SPD_FASTER);
	gpio_init(101, GPIO_MODE_ALT | GPIO_SPD_FASTER);
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

enum ORIENTATION {
	PORTRATE	= 0,
	PORTRATE_REV,
	LANDSCAPE,
	LANDSCAPE_REV,
};

static void lcd_rotate(enum ORIENTATION orientation)
{
	int driveout, entrymode, gatescan;

	driveout = 0;
	entrymode = 0x1000;
	gatescan = 0x2700;

	switch (orientation) {
	case LANDSCAPE:
		driveout = 0x0100;
		entrymode |= 0x0038;
		gatescan |= 0x8000;
		break;
	case LANDSCAPE_REV:
		driveout = 0x0000;
		entrymode |= 0x0038;
		gatescan |= 0x0000;
		break;
	case PORTRATE_REV:
		driveout = 0x0000;
		entrymode |= 0x0030;
		gatescan |= 0x8000;
		break;
	case PORTRATE:
	default:
		driveout = 0x0100;
		entrymode |= 0x0030;
		gatescan |= 0x0000;
		break;
	}

	lcd_write_reg(0x0001, driveout);
	lcd_write_reg(0x0003, entrymode);
	lcd_write_reg(0x0060, gatescan);
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

	lcd_write_reg(0x0007, 0x0133); /* display on */
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

static void lcd_terminate()
{
	lcd_backlight(OFF);
	lcd_reset(LOW);
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
	int x, y;
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

static inline void put_pixel(pos_t *pos, color_t *color)
{
	lcd_write_at(pos2pxl(pos->x, pos->y));
	lcd_write_data(color->fg);
}

static pos_t draw_line(pos_t p1, pos_t p2, color_t *color)
{
	int slope, x, y;
	pos_t q;

	slope = (int)(p2.y - p1.y) / (p2.x - p1.x);

	for (x = p1.x; x <= p2.x; x++) {
		y = slope * (x - p2.x) + p2.y;
		q.x = x;
		q.y = y;
		put_pixel(&q, color);
	}

	return q;
}

#include <timer.h>
#include <asm/timer.h>
#include <stdlib.h>
#include <string.h>

static int pwm_input_init(int psc)
{
	int fd;
	timer_t tim;

	fd = open("/dev/tim2", O_RDONLY);
	printf("fd = %x\n", fd);

	memset(&tim, 0, sizeof(tim));
	tim.channel = TIM_IO_CH2;
	tim.pin = PIN_TIM2CH2;
	tim.iomode = TIM_IO_PWM;
	tim.interrupt = true;
	tim.prescale = psc - 1;
	ioctl(fd, C_SET, &tim);

	return fd;
}

#define xy2pos(x, y)	((pos_t){x, y})

static void test_lcd()
{
	lcd_init();

	clear(0x0000);

	char buf[80];
	pos_t pos = {0, 0};
	color_t hz_color = {0xf800, 0x0000};
	color_t tcolor = {0xaaaa, 0x0000};
	color_t bcolor = {0x6666, 0x0000};
	color_t color = {0xffff, 0x0000};

	unsigned int capture[4], prescale, clk;
	volatile int t1, t2;
	int fd;

	clk = get_hclk();
	prescale = 35999;
	fd = pwm_input_init(prescale);

	while (1) {
		if (!read(fd, capture, sizeof(capture)))
			continue;

		t1 = capture[0];
		t2 = capture[1];

		sprintf(buf, "%13dHz", clk / prescale / t2);
		pos = xy2pos(0, 0);
		lcd_puts(buf, &pos, &hz_color);

		sprintf(buf, "%08x:%d", systick, systick / HZ);
		pos = xy2pos(0, NR_PIXELS_ROW - FONT_ROW);
		lcd_puts(buf, &pos, &tcolor);

		draw_line(xy2pos(0, 50), xy2pos(NR_PIXELS_COL, 50), &bcolor);
		/* graph here */
		draw_line(xy2pos(0, 150), xy2pos(NR_PIXELS_COL, 150), &bcolor);

		sprintf(buf, "t1:%-12d\nt2:%-12d\nratio:%d%%  \n",
				t1, t2, t1 * 100 / t2);
		pos = xy2pos(0, 180);
		lcd_puts(buf, &pos, &color);

		sprintf(buf, "scale:%-9d", clk / prescale);
		lcd_puts(buf, &pos, &color);
#if 0
		printf("%d : %d, f=%dHz %3d%%\n",
				t1, t2,
				clk / prescale / t2,
				t1 * 100 / t2);
		sleep(1);
#endif
	}

	close(fd);
}
REGISTER_TASK(test_lcd, TASK_KERNEL, DEFAULT_PRIORITY);
