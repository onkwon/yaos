#include <foundation.h>
#include <kernel/timer.h>
#include "lcd.h"

void ili9341_init()
{
	lcd_write_reg(0xCA);
	lcd_write_data(0xC3);
	lcd_write_data(0x08);
	lcd_write_data(0x50);

	lcd_write_reg(0xCF);
	lcd_write_data(0x00);
	lcd_write_data(0xC1);
	lcd_write_data(0x30);

	lcd_write_reg(0xED);
	lcd_write_data(0x64);
	lcd_write_data(0x03);
	lcd_write_data(0x12);
	lcd_write_data(0x81);

	lcd_write_reg(0xE8);
	lcd_write_data(0x85);
	lcd_write_data(0x00);
	lcd_write_data(0x78);

	lcd_write_reg(0xCB);
	lcd_write_data(0x39);
	lcd_write_data(0x2C);
	lcd_write_data(0x00);
	lcd_write_data(0x34);
	lcd_write_data(0x02);

	lcd_write_reg(0xF7);
	lcd_write_data(0x20);

	lcd_write_reg(0xEA);
	lcd_write_data(0x00);
	lcd_write_data(0x00);

	lcd_write_reg(0xB1);
	lcd_write_data(0x00);
	lcd_write_data(0x1B);

	lcd_write_reg(0xB6);
	lcd_write_data(0x0A);
	lcd_write_data(0xA2);

	lcd_write_reg(0xC0);
	lcd_write_data(0x10);
	lcd_write_reg(0xC1);
	lcd_write_data(0x10);
	lcd_write_reg(0xC5);
	lcd_write_data(0x45);
	lcd_write_data(0x15);
	lcd_write_reg(0xC7);
	lcd_write_data(0x90);

	lcd_write_reg(0x36);
	lcd_write_data(0xC8);

	lcd_write_reg(0xF2);
	lcd_write_data(0x00);

	lcd_write_reg(0xB0);
	lcd_write_data(0xC2);

	lcd_write_reg(0xB6);
	lcd_write_data(0x0A);
	lcd_write_data(0xA7);
	lcd_write_data(0x27);
	lcd_write_data(0x04);

	lcd_write_reg(0x2A);
	lcd_write_data(0x00);
	lcd_write_data(0x00);
	lcd_write_data(0x00);
	lcd_write_data(0xEF);

	lcd_write_reg(0x2B);
	lcd_write_data(0x00);
	lcd_write_data(0x00);
	lcd_write_data(0x01);
	lcd_write_data(0x3F);

	lcd_write_reg(0xF6);
	lcd_write_data(0x01);
	lcd_write_data(0x00);
	lcd_write_data(0x06);

	lcd_write_reg(0x2C);
	udelay(200);

	lcd_write_reg(0x26);
	lcd_write_data(0x01);

	lcd_write_reg(0xE0);
	lcd_write_data(0x0F);
	lcd_write_data(0x29);
	lcd_write_data(0x24);
	lcd_write_data(0x0C);
	lcd_write_data(0x0E);
	lcd_write_data(0x09);
	lcd_write_data(0x4E);
	lcd_write_data(0x78);

	lcd_write_data(0x3C);
	lcd_write_data(0x09);
	lcd_write_data(0x13);
	lcd_write_data(0x05);
	lcd_write_data(0x17);
	lcd_write_data(0x11);
	lcd_write_data(0x00);

	lcd_write_reg(0xE1);
	lcd_write_data(0x00);
	lcd_write_data(0x16);
	lcd_write_data(0x1B);
	lcd_write_data(0x04);
	lcd_write_data(0x11);
	lcd_write_data(0x07);
	lcd_write_data(0x31);
	lcd_write_data(0x33);
	lcd_write_data(0x42);
	lcd_write_data(0x05);
	lcd_write_data(0x0C);
	lcd_write_data(0x0A);
	lcd_write_data(0x28);
	lcd_write_data(0x2F);
	lcd_write_data(0x0F);
	lcd_write_reg(0x11);
	udelay(200);
	lcd_write_reg(0x29);
	lcd_write_reg(0x2C);
}
