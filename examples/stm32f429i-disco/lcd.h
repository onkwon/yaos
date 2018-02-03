#ifndef __STM32_LCD_H__
#define __STM32_LCD_H__

#include <stdint.h>
#include <types.h>

#define LCD_WIDTH		240
#define LCD_HEIGHT		320

/* DRAM Mapping
 *
 * 240x320x4(bytes per pixel) = 307,200 bytes, 0x4B000
 *
 * 0xD0000000 - 0xD004AFFF : Layer0 Framebuffer0 (4 bytes per pixel)
 * 0xD004B000 - 0xD0095FFF : Layer0 Framebuffer1
 * 0xD0096000 - 0xD00E0FFF : Layer0 Framebuffer2
 * 0xD00E1000 - 0xD012BFFF : Layer1 Framebuffer0
 * 0xD012C000 - 0xD0176FFF : Layer1 Framebuffer1
 * 0xD0177000 - 0xD01C1FFF : Layer1 Framebuffer2
 * 0xD01C2000 - 0xD03C1FFF : emWin heap
 * 0xD03C2000 - 0xD07FFFFF : about 4MiB reserved
 */
#define LCD_LAYER1_FB		0xD0000000U
#define LCD_LAYER2_FB		0xD00E1000U
#define LCD_HEAP		0xD01C2000U

enum pixel_format_t {
	PF_ARGB8888	= 0,
	PF_RGB888	= 1,
	PF_RGB565	= 2,
	PF_ARGB1555	= 3,
	PF_ARGB4444	= 4,
	PF_L8		= 5,
	PF_AL44		= 6,
	PF_AL88		= 7,
	PF_L4		= 8,
	PF_A8		= 9,
	PF_A4		= 10,
};

#define LTDC_BASEADDR		0x40016800U
#define LTDC_LAYER1_BASEADDR	0x40016884U
#define LTDC_LAYER2_BASEADDR	0x40016904U

struct ltdc_layer_t {
	reg_t _reserved[14];	// 0x4c, 0x50, 0x54, 0x58, 0x5c, 0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c, 0x80
	reg_t CR;		// 0x84 or 0x104
	reg_t WHPCR;		// 0x88
	reg_t WVPCR;		// 0x8c
	reg_t CKCR;		// 0x90
	reg_t PFCR;		// 0x94
	reg_t CACR;		// 0x98
	reg_t DCCR;		// 0x9c
	reg_t BFCR;		// 0xa0
	reg_t _reserved1[2];	// 0xa4, 0xa8
	reg_t CFBAR;		// 0xac
	reg_t CFBLR;		// 0xb0
	reg_t CFBLNR;		// 0xb4
	reg_t _reserved2[3];	// 0xb8, 0xbc, 0xc0
	reg_t CLUTWR;		// 0xc4
	reg_t _reserved3;	// 0xc8
} __attribute__((packed, aligned(4)));

struct ltdc_t {
	reg_t _reserved[2];	// 0x00, 0x04
	reg_t SSCR;		// 0x08
	reg_t BPCR;		// 0x0c
	reg_t AWCR;		// 0x10
	reg_t TWCR;		// 0x14
	reg_t GCR;		// 0x18
	reg_t _reserved2[2];	// 0x1c, 0x20
	reg_t SRCR;		// 0x24
	reg_t _reserved3;	// 0x28
	reg_t BCCR;		// 0x2c
	reg_t _reserved4;	// 0x30
	reg_t IER;		// 0x34
	reg_t ISR;		// 0x38
	reg_t ICR;		// 0x3c
	reg_t LIPCR;		// 0x40
	reg_t CPSR;		// 0x44
	reg_t CDSR;		// 0x48
	struct ltdc_layer_t layer[2];
} __attribute__((packed, aligned(4)));

struct ltdc_t *LTDC;

void lcd_write_reg(uint8_t reg);
void lcd_write_data(uint8_t v);
void lcd_init();

void lcd_layer_pos_set(int layer, int x0, int x1, int y0, int y1);
void lcd_layer_pf_set(int layer, enum pixel_format_t pf);
void lcd_layer_fb_set(int layer, unsigned int addr);
void lcd_layer_alpha_set(int layer, uint8_t alpha);
void lcd_layer_set(int layer, bool on);
void lcd_reload();

extern void ili9341_init(); /* ili9341.c */

#endif /* __STM32_LCD_H__ */
