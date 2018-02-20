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

#include <asm/mach/board/lcd.h>
#include <asm/mach/board/dma2d.h>
#include <foundation.h>
#include <kernel/timer.h>
#include <drivers/spi.h>
#include <stdint.h>

#define HSYNC		10 /* Horizontal sync */
#define HBP		20 /* Horizontal back porch */
#define HFP		10 /* Horizontal front porch */
#define VSYNC		 2 /* Vertical sync */
#define VBP		 2 /* Vertical back porch */
#define VFP		 4 /* Vertical front porch */

#define HACTIVE		(HSYNC + LCD_WIDTH + HBP - 1)
#define VACTIVE		(VSYNC + LCD_HEIGHT + VBP - 1)

#define HTOTAL		(HSYNC + HBP + LCD_WIDTH + HFP - 1)
#define VTOTAL		(VSYNC + VBP + LCD_HEIGHT + VFP - 1)

#define LCD_NCS(lv)	gpio_put(PIN_LCD_MCU_NCS, lv)
#define LCD_RDX(lv)	gpio_put(PIN_LCD_MCU_RDX, lv)
#define LCD_WRX(lv)	gpio_put(PIN_LCD_MCU_WRX, lv)

struct ltdc_t *LTDC = (struct ltdc_t *)LTDC_BASEADDR;

static inline void lcd_spi_init()
{
	__turn_apb2_clock(20, ON); /* SPI5 clock enable */

	gpio_init(PIN_SPI5_SCK, GPIO_MODE_ALT | gpio_altfunc(5) |
			GPIO_CONF_PULLDOWN | GPIO_SPD_MID);
	gpio_init(PIN_SPI5_MISO, GPIO_MODE_ALT | gpio_altfunc(5) |
			GPIO_CONF_PULLDOWN | GPIO_SPD_MID);
	gpio_init(PIN_SPI5_MOSI, GPIO_MODE_ALT | gpio_altfunc(5) |
			GPIO_CONF_PULLDOWN | GPIO_SPD_MID);

	// should be less or equal to 6.66MHz as ili9341 spi max clock is
	// 6.66MHz for read while 10MHz for write
	// 3.75MHz = 60MHz(plck2) / 16
	spi_init(SPI5, SPI_MASTER, 3750, SPI_NSS_SOFT);
}

void __attribute__((weak)) lcd_vsync_callback() {}

static void ISR_VSYNC(int nvector)
{
	lcd_vsync_callback();
	(void)nvector;
}

static inline void lcd_gpio_init()
{
	int vector;

	gpio_init(PIN_LCD_HSYNC, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_VSYNC, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_CLK, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_DE, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_R2, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_R3, GPIO_MODE_ALT | gpio_altfunc(9) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_R4, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_R5, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_R6, GPIO_MODE_ALT | gpio_altfunc(9) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_R7, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_G2, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_G3, GPIO_MODE_ALT | gpio_altfunc(9) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_G4, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_G5, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_G6, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_G7, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_B2, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_B3, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_B4, GPIO_MODE_ALT | gpio_altfunc(9) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_B5, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_B6, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_B7, GPIO_MODE_ALT | gpio_altfunc(14) | GPIO_SPD_FAST);

	vector = gpio_init(PIN_LCD_TE, GPIO_MODE_INPUT | GPIO_INT_RISING);
	register_isr(vector, ISR_VSYNC);
}

static inline void lcd_cntl_init()
{
	gpio_init(PIN_LCD_MCU_NCS, GPIO_MODE_OUTPUT | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_MCU_RDX, GPIO_MODE_OUTPUT | GPIO_SPD_FAST);
	gpio_init(PIN_LCD_MCU_WRX, GPIO_MODE_OUTPUT | GPIO_SPD_FAST);
	LCD_NCS(HIGH);

	/* IM[0..3] = 0110 --> 4-wire 8-bit serial I, SDA:In/Out */

	lcd_spi_init();
	ili9341_init();
}

void lcd_write_reg(uint8_t reg)
{
	LCD_WRX(LOW);
	LCD_NCS(LOW);
	spi_write_byte(SPI5, reg);
	LCD_NCS(HIGH);
}

void lcd_write_data(uint8_t v)
{
	LCD_WRX(HIGH);
	LCD_NCS(LOW);
	spi_write_byte(SPI5, v);
	LCD_NCS(HIGH);
}

void lcd_layer_pos_set(int layer, int x0, int x1, int y0, int y1)
{
	LTDC->layer[layer].WHPCR = (HBP + x0) | ((HBP + x1 - 1) << 16);
	LTDC->layer[layer].WVPCR = (VBP + y0) | ((VBP + y1 - 1) << 16);
}

void lcd_layer_pf_set(int layer, enum pixel_format_t pf)
{
	uint32_t pixelsize;

	switch (pf) {
	case PF_ARGB8888:
		pixelsize = 4;
		break;
	case PF_RGB888:
		pixelsize = 3;
		break;
	case PF_RGB565:
	case PF_ARGB1555:
	case PF_ARGB4444:
	case PF_AL88:
	default:
		pixelsize = 2;
		break;
	case PF_L8:
	case PF_AL44:
		pixelsize = 1;
		break;
	}

	LTDC->layer[layer].PFCR = pf;
	LTDC->layer[layer].CFBLR = ((LCD_WIDTH * pixelsize) << 16) | ((LCD_WIDTH * pixelsize) + 3);
	LTDC->layer[layer].CFBLNR = LCD_HEIGHT;
}

void lcd_layer_fb_set(int layer, unsigned int addr)
{
	LTDC->layer[layer].CFBAR = addr;
}

void lcd_layer_alpha_set(int layer, uint8_t alpha)
{
	LTDC->layer[layer].CACR = alpha;
}

void lcd_layer_set(int layer, bool on)
{
	LTDC->layer[layer].CR = (LTDC->layer[layer].CR & ~1U) | on;
}

void lcd_reload()
{
	LTDC->SRCR = 1;
}

void lcd_init()
{
	lcd_gpio_init();
	lcd_cntl_init();

	// SAI input(HSE/PLLM) = 8MHz / 4 = 2MHz
	// SAI output(2MHz * SAIN) = 2*96 = 192MHz
	// VCO = 192MHz / 4(SAIR) = 48MHz
	// lcdclk = 48MHz / 8(SAIDIVR) = 6MHz
	RCC_CR &= ~(1 << 28); // PLLSAION
	RCC_PLLSAICFGR = (96 << 6) | (4 << 28); // SAIN = 96, SAIR = 4
	RCC_PLLDCKCFGR = (2 << 16); // SAIDIVR = 8
	RCC_CR |= (1 << 28); // PLLSAION
	while (!(RCC_CR & (1 << 29)));

	__turn_apb2_clock(26, ON); /* LTDC clock enable */

	LTDC->SSCR = ((HSYNC - 1) << 16) | (VSYNC - 1);
	LTDC->BPCR = ((HBP - 1) << 16) | (VBP - 1);
	LTDC->AWCR = (HACTIVE << 16) | VACTIVE;
	LTDC->TWCR = (HTOTAL << 16) | VTOTAL;

	LTDC->BCCR = 0; // Background color
	LTDC->GCR |= 1 << 16; // DEN, enable dithering

	LTDC->GCR |= 1;

	dma2d_init();
	__turn_ahb1_clock(12, ON); /* CRC clock enable */
}

#if 0
static void lcd()
{
	extern void sdram_init();
	sdram_init();
	lcd_init();

	volatile uint8_t *fb = (uint8_t *)0xd0000000;
	volatile uint8_t *fb2 = (uint8_t *)0xd0040000;
	int i, j = 160;

	notice("lcd ready");

	lcd_layer_pos_set(0, 0, LCD_WIDTH, 0, LCD_HEIGHT);
	lcd_layer_pf_set(0, PF_RGB888);
	lcd_layer_fb_set(0, 0xd0000000);
	lcd_layer_set(0, ON);

	lcd_layer_pos_set(1, 0, LCD_WIDTH, 160, LCD_HEIGHT);
	lcd_layer_pf_set(1, PF_ARGB8888);
	lcd_layer_fb_set(1, 0xd0040000);
	lcd_layer_set(1, ON);

	lcd_reload();

	while (1) {
		sleep(1);
		for (i = 0; i < 320*240*3; i+=3)
			fb[i] = 0, fb[i+1] = 0, fb[i+2] = 0xff;
		sleep(1);
		for (i = 0; i < 320*240*4; i+=4)
			fb2[i] = 0xff, fb2[i+1] = 0, fb2[i+2] = 0, fb2[i+3] = 0x80;

		lcd_layer_pos_set(1, 0, LCD_WIDTH, j, LCD_HEIGHT);
		lcd_reload();
		j++;
		if (j >= 320)
			j = 160;
	}

}
REGISTER_TASK(lcd, TASK_KERNEL, DEFAULT_PRIORITY, STACK_SIZE_DEFAULT);
#endif
