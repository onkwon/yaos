#include "lcd.h"
#include <foundation.h>
#include <kernel/timer.h>
#include <stdint.h>
#include "spi.h"

#include "GUI.h"
//#include "WM.h"

#include "../examples/emWin/fonts/BebasNeue80B.h"

#define AA_FACTOR		8

#define SCREEN_SIZE_X		LCD_WIDTH
#define SCREEN_SIZE_Y		LCD_HEIGHT
#define ACTIVE_SIZE_X		SCREEN_SIZE_X
#define ACTIVE_SIZE_Y		SCREEN_SIZE_Y
#define POS_X_START		0
#define POS_Y_START		0
#define POS_X_END		(ACTIVE_SIZE_X + POS_X_START)
#define POS_Y_END		(ACTIVE_SIZE_Y + POS_Y_START)
#define POS_X_CENTER		((ACTIVE_SIZE_X >> 1) + POS_X_START)
#define POS_Y_CENTER		((ACTIVE_SIZE_Y >> 1) + POS_Y_START)

static inline void hw_init()
{
	extern void sdram_init();
	sdram_init();
	lcd_init();

	GUI_Init();
	GUI_Clear();

	lcd_layer_pos_set(0, 0, LCD_WIDTH, 0, LCD_HEIGHT);
	lcd_layer_pf_set(0, PF_RGB565);
	lcd_layer_fb_set(0, LCD_LAYER1_FB);
	lcd_layer_alpha_set(0, 0xff);
	lcd_layer_set(0, ON);
	lcd_layer_pos_set(1, 0, LCD_WIDTH, 0, LCD_HEIGHT);
	lcd_layer_pf_set(1, PF_ARGB1555);
	lcd_layer_fb_set(1, LCD_LAYER2_FB);
	lcd_layer_alpha_set(1, 0xff);
	lcd_layer_set(1, ON);
}

static inline void display_init()
{
	GUI_AA_SetFactor(AA_FACTOR);
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);
	GUI_SetBkColor(GUI_BLACK);
	GUI_MEMDEV_MULTIBUF_Enable(true);
	//WM_MULTIBUF_Enable(true);
}

static void lcd()
{
	hw_init();
	display_init();

	int x, y, pensize, show = 1;
	unsigned int tout = 0;

	GUI_MEMDEV_Handle guage;
	guage = GUI_MEMDEV_Create(POS_X_START, POS_Y_START, POS_X_END, POS_Y_START + 30);

	GUI_Clear();
	GUI_DispStringAt("A template", POS_X_START + 5, POS_Y_START + 5);
	GUI_SetFont(&GUI_FontBebasNeue80B);
	GUI_DispStringHCenterAt("Layer 0", POS_X_CENTER, ACTIVE_SIZE_Y * 50 / 100);

	GUI_SelectLayer(1);
	GUI_SetLayerPosEx(1, POS_X_START, POS_Y_START + 20);
	GUI_SetLayerAlphaEx(1, 0x80);
	GUI_SetBkColor(GUI_RED);
	GUI_Clear();
	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_Font6x8);
	GUI_DispStringHCenterAt("Layer 1", POS_X_CENTER, POS_Y_END - 60);

	pensize = GUI_SetPenSize(5);
	GUI_AA_DrawLine(POS_X_START + 30, POS_Y_END - 40, POS_X_END - 30, POS_Y_END - 40);
	GUI_SetPenSize(pensize);

	GUI_SetColor(GUI_BLACK);
	GUI_AA_FillCircle(POS_X_START + 10, POS_Y_START + 10, 10);

	GUI_SelectLayer(0);
	GUI_SetColor(GUI_GRAY);
	GUI_SetBkColor(GUI_BLACK);

	x = 10, y = 30;

	while(1) {
		if (is_timeout(tout)) {
			set_timeout(&tout, msec_to_ticks(500));
			show ^= 1;
			GUI_SetLayerVisEx(1, show);
		}

		x = ACTIVE_SIZE_X * y / 100 + POS_X_START;
		y = x > POS_X_END? 0 : y + 1;

		GUI_MULTIBUF_Begin();

		GUI_MEMDEV_Select(guage);
		GUI_Clear();
		GUI_FillRect(POS_X_START, POS_Y_START, x, POS_Y_START + 30);
		GUI_MEMDEV_CopyToLCD(guage);
#if 0
		GUI_SetColor(GUI_BLACK);
		GUI_AA_FillCircle(x, y, 10);
		x += 10;
		y = x * x / 80;
		notice("x %d y %d", x, y);
		if (x > POS_X_END || y > POS_Y_END)
			x = 10, y = 30;
		GUI_SetColor(GUI_GRAY);
		GUI_AA_FillCircle(x, y, 10);
#endif
		GUI_MULTIBUF_End();

		GUI_MEMDEV_Select(0);
	}
}
REGISTER_TASK(lcd, TASK_KERNEL, DEFAULT_PRIORITY, STACK_SIZE_DEFAULT);
