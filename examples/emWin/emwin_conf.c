#include "GUI.h"
#include "GUIDRV_Lin.h"

#include "lcd.h"
#include "dma2d.h"
#include <foundation.h>

#define GUI_NUMBYTES  0x200000

void GUI_X_Config()
{
	GUI_ALLOC_AssignMemory((void *)LCD_HEAP, GUI_NUMBYTES);
	GUI_SetDefaultFont(GUI_FONT_6X8);
}

GUI_TIMER_TIME GUI_X_GetTime()
{ 
	extern volatile unsigned int systick;
	return (int)*(volatile unsigned int *)&systick;
}

void GUI_X_Delay(int ms)
{ 
	extern void mdelay(unsigned int ms);
	mdelay(ms);
}

void GUI_X_Init(void) {}
void GUI_X_ExecIdle(void) {}
void GUI_X_Log     (const char *s) { GUI_USE_PARA(s); }
void GUI_X_Warn    (const char *s) { GUI_USE_PARA(s); }
void GUI_X_ErrorOut(const char *s) { GUI_USE_PARA(s); }

#define DISPLAY_DRIVER	GUIDRV_LIN_16

#define NUM_BUFFERS	3
#define NUM_VSCREENS	1
#define NUM_VSCREENS	1

typedef struct
{
	unsigned int address;          
	volatile int pending_buffer;
	int buffer_index;
	int xSize;
	int ySize;
	int BytesPerPixel;
	int pf;
	LCD_API_COLOR_CONV *pColorConvAPI;
} LCD_LayerPropTypedef;

static LCD_LayerPropTypedef _layer[2];

static void DMA2D_CopyBuffer(int LayerIndex, void *pSrc, void *pDst,
		unsigned int xSize, unsigned int ySize,
		unsigned int OffLineSrc, unsigned int OffLineDst);
static void CUSTOM_CopyBuffer(int LayerIndex, int IndexSrc, int IndexDst);
static void CUSTOM_CopyRect(int LayerIndex, int x0, int y0, int x1, int y1,
		int xSize, int ySize);
static void DMA2D_FillBuffer(int LayerIndex, void *pDst,
		unsigned int xSize, unsigned int ySize,
		unsigned int OffLine, unsigned int ColorIndex);
static void CUSTOM_FillRect(int LayerIndex, int x0, int y0, int x1, int y1,
		unsigned int PixelIndex);
static void BSP_LCD_DrawBitmap16bpp(int LayerIndex, int x, int y,
		void *p, int xSize, int ySize, int BytesPerLine);

void LCD_X_Config(void)
{
#if (NUM_BUFFERS > 1)
	GUI_MULTIBUF_Config(NUM_BUFFERS);
#endif

	_layer[0].address = LCD_LAYER1_FB;
	_layer[0].pColorConvAPI = (LCD_API_COLOR_CONV *)GUICC_M565;
	_layer[0].pending_buffer = -1;
	_layer[0].BytesPerPixel = 2;
	_layer[0].pf = PF_RGB565;
	_layer[0].xSize = LCD_WIDTH;
	_layer[0].ySize = LCD_HEIGHT;

	_layer[1].address = LCD_LAYER2_FB;
	_layer[1].pColorConvAPI = (LCD_API_COLOR_CONV *)GUICC_M1555I;
	_layer[1].pending_buffer = -1;
	_layer[1].BytesPerPixel = 2;
	_layer[1].pf = PF_ARGB1555;
	_layer[1].xSize = LCD_WIDTH;
	_layer[1].ySize = LCD_HEIGHT;

	GUI_DEVICE_CreateAndLink(GUIDRV_LIN_16, GUICC_M565, 0, 0);
	LCD_SetVRAMAddrEx(0, (void *)_layer[0].address);
	if (LCD_GetSwapXYEx(0)) {
		LCD_SetSizeEx (0, LCD_HEIGHT, LCD_WIDTH);
		LCD_SetVSizeEx(0, LCD_HEIGHT * NUM_VSCREENS, LCD_WIDTH);
	} else {
		LCD_SetSizeEx (0, LCD_WIDTH, LCD_HEIGHT);
		LCD_SetVSizeEx(0, LCD_WIDTH, LCD_HEIGHT * NUM_VSCREENS);
	}

	GUI_DEVICE_CreateAndLink(GUIDRV_LIN_16, GUICC_M1555I, 0, 1);
	LCD_SetVRAMAddrEx(1, (void *)_layer[1].address);
	if (LCD_GetSwapXYEx(1)) {
		LCD_SetSizeEx (1, LCD_HEIGHT, LCD_WIDTH);
		LCD_SetVSizeEx(1, LCD_HEIGHT * NUM_VSCREENS, LCD_WIDTH);
	} else {
		LCD_SetSizeEx (1, LCD_WIDTH, LCD_HEIGHT);
		LCD_SetVSizeEx(1, LCD_WIDTH, LCD_HEIGHT * NUM_VSCREENS);
	}

	LCD_SetDevFunc(0, LCD_DEVFUNC_COPYBUFFER, (void(*)(void))CUSTOM_CopyBuffer);
	LCD_SetDevFunc(0, LCD_DEVFUNC_COPYRECT,   (void(*)(void))CUSTOM_CopyRect);
	LCD_SetDevFunc(0, LCD_DEVFUNC_FILLRECT, (void(*)(void))CUSTOM_FillRect);
	LCD_SetDevFunc(0, LCD_DEVFUNC_DRAWBMP_16BPP, (void(*)(void))BSP_LCD_DrawBitmap16bpp);
	LCD_SetDevFunc(1, LCD_DEVFUNC_COPYBUFFER, (void(*)(void))CUSTOM_CopyBuffer);
	LCD_SetDevFunc(1, LCD_DEVFUNC_COPYRECT,   (void(*)(void))CUSTOM_CopyRect);
	LCD_SetDevFunc(1, LCD_DEVFUNC_FILLRECT, (void(*)(void))CUSTOM_FillRect);
	LCD_SetDevFunc(1, LCD_DEVFUNC_DRAWBMP_16BPP, (void(*)(void))BSP_LCD_DrawBitmap16bpp);
}

int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData)
{
	unsigned int addr;
	int x, y;

	switch (Cmd) {
	case LCD_X_SHOWBUFFER:
		_layer[LayerIndex].pending_buffer = ((LCD_X_SHOWBUFFER_INFO *)pData)->Index;
		break;
	case LCD_X_SETSIZE:
		GUI_GetLayerPosEx(LayerIndex, &x, &y);
		_layer[LayerIndex].xSize = ((LCD_X_SETSIZE_INFO *)pData)->xSize;
		_layer[LayerIndex].ySize = ((LCD_X_SETSIZE_INFO *)pData)->ySize;
		lcd_layer_pos_set(LayerIndex, x, x + LCD_WIDTH, y, y + LCD_HEIGHT);
		break;
	case LCD_X_SETALPHA:
		lcd_layer_alpha_set(LayerIndex, ((LCD_X_SETALPHA_INFO *)pData)->Alpha);
		break;
	case LCD_X_SETVIS:
		if(((LCD_X_SETVIS_INFO *)pData)->OnOff  == ENABLE)
			lcd_layer_set(LayerIndex, ENABLE);
		else
			lcd_layer_set(LayerIndex, DISABLE);
		lcd_reload();
		break;
	case LCD_X_SETORG:
		addr = _layer[LayerIndex].address +
			((LCD_X_SETORG_INFO *)pData)->yPos *
			_layer[LayerIndex].xSize *
			_layer[LayerIndex].BytesPerPixel;
		lcd_layer_fb_set(LayerIndex, addr);
		break;
	case LCD_X_SETPOS:
		lcd_layer_pos_set(LayerIndex,
				((LCD_X_SETPOS_INFO *)pData)->xPos,
				_layer[LayerIndex].xSize,
				((LCD_X_SETPOS_INFO *)pData)->yPos,
				_layer[LayerIndex].ySize);
		break;
	case LCD_X_SETVRAMADDR:
	case LCD_X_ON:
	case LCD_X_OFF:
	case LCD_X_INITCONTROLLER:
	default:
		notice("EMWIN: %x, %d %x", Cmd, LayerIndex, pData);
		break;
	}

	return 0;
}

static void DMA2D_CopyBuffer(int LayerIndex, void *pSrc, void *pDst,
		unsigned int xSize, unsigned int ySize,
		unsigned int OffLineSrc, unsigned int OffLineDst)
{
	DMA2D->CR      = 0x00000000UL | (1 << 9);
	DMA2D->FGMAR   = (unsigned int)pSrc;
	DMA2D->OMAR    = (unsigned int)pDst;
	DMA2D->FGOR    = OffLineSrc;
	DMA2D->OOR     = OffLineDst;
	DMA2D->FGPFCCR = _layer[LayerIndex].pf;
	DMA2D->NLR     = (unsigned int)(xSize << 16) | ySize;
	DMA2D->CR     |= 1; // START
	while (DMA2D->CR & 1);
}

static void CUSTOM_CopyBuffer(int LayerIndex, int IndexSrc, int IndexDst)
{
	unsigned int BufferSize, AddrSrc, AddrDst;

	BufferSize = _layer[LayerIndex].xSize * _layer[LayerIndex].ySize *
		_layer[LayerIndex].BytesPerPixel;
	AddrSrc    = _layer[LayerIndex].address + BufferSize * IndexSrc;
	AddrDst    = _layer[LayerIndex].address + BufferSize * IndexDst;
	DMA2D_CopyBuffer(LayerIndex,
			(void *)AddrSrc, (void *)AddrDst,
			_layer[LayerIndex].xSize, _layer[LayerIndex].ySize,
			0, 0);
	_layer[LayerIndex].buffer_index = IndexDst;
}

static void CUSTOM_CopyRect(int LayerIndex, int x0, int y0, int x1, int y1,
		int xSize, int ySize) 
{
	unsigned int AddrSrc, AddrDst;
	
	AddrSrc = _layer[LayerIndex].address +
		(y0 * _layer[LayerIndex].xSize + x0) *
		_layer[LayerIndex].BytesPerPixel;
	AddrDst = _layer[LayerIndex].address +
		(y1 * _layer[LayerIndex].xSize + x1) *
		_layer[LayerIndex].BytesPerPixel;

	DMA2D_CopyBuffer(LayerIndex,
			(void *)AddrSrc, (void *)AddrDst,
			xSize, ySize,
			_layer[LayerIndex].xSize - xSize,
			_layer[LayerIndex].xSize - xSize);
}

static void DMA2D_FillBuffer(int LayerIndex, void *pDst,
		unsigned int xSize, unsigned int ySize,
		unsigned int OffLine, unsigned int ColorIndex) 
{
	DMA2D->CR      = 0x00030000UL | (1 << 9);
	DMA2D->OCOLR   = ColorIndex;
	DMA2D->OMAR    = (unsigned int)pDst;
	DMA2D->OOR     = OffLine;
	DMA2D->OPFCCR  = _layer[LayerIndex].pf;
	DMA2D->NLR     = (unsigned int)(xSize << 16) | ySize;
	DMA2D->CR     |= 1; 
	while (DMA2D->CR & 1);
}

static void CUSTOM_FillRect(int LayerIndex, int x0, int y0, int x1, int y1,
		unsigned int PixelIndex) 
{
	unsigned int BufferSize, AddrDst;
	int xSize, ySize;

	if (GUI_GetDrawMode() == GUI_DM_XOR) {
		LCD_SetDevFunc(LayerIndex, LCD_DEVFUNC_FILLRECT, NULL);
		LCD_FillRect(x0, y0, x1, y1);
		LCD_SetDevFunc(LayerIndex, LCD_DEVFUNC_FILLRECT, (void(*)(void))CUSTOM_FillRect);
	} else {
		xSize = x1 - x0 + 1;
		ySize = y1 - y0 + 1;
		BufferSize = _layer[LayerIndex].xSize * _layer[LayerIndex].ySize *
			_layer[LayerIndex].BytesPerPixel;
		AddrDst = _layer[LayerIndex].address +
			BufferSize * _layer[LayerIndex].buffer_index +
			(y0 * _layer[LayerIndex].xSize + x0) *
			_layer[LayerIndex].BytesPerPixel;
		DMA2D_FillBuffer(LayerIndex, (void *)AddrDst, xSize, ySize,
				_layer[LayerIndex].xSize - xSize, PixelIndex);
	}
}

static void BSP_LCD_DrawBitmap16bpp(int LayerIndex, int x, int y,
		void *p, int xSize, int ySize, int BytesPerLine)
{
	unsigned int BufferSize, AddrDst;
	int OffLineSrc, OffLineDst;

	BufferSize = _layer[LayerIndex].xSize * _layer[LayerIndex].ySize *
		_layer[LayerIndex].BytesPerPixel;
	AddrDst = _layer[LayerIndex].address +
		BufferSize * _layer[LayerIndex].buffer_index +
		(y * _layer[LayerIndex].xSize + x) *
		_layer[LayerIndex].BytesPerPixel;
	OffLineSrc = (BytesPerLine / 2) - xSize;
	OffLineDst = _layer[LayerIndex].xSize - xSize;
	DMA2D_CopyBuffer(LayerIndex, (void *)p, (void *)AddrDst, xSize, ySize, OffLineSrc, OffLineDst);
}

void lcd_vsync_callback()
{
	unsigned int addr;
	int i;

	for (i = 0; i < 2; i++) {
		if (_layer[i].pending_buffer < 0)
			continue;

		addr = _layer[i].address +
			_layer[i].xSize *
			_layer[i].ySize *
			_layer[i].pending_buffer *
			_layer[i].BytesPerPixel;
		lcd_layer_fb_set(i, addr);
		GUI_MULTIBUF_ConfirmEx(i, _layer[i].pending_buffer);
		_layer[i].pending_buffer = -1;
	}
	lcd_reload();
}

#if 1
//////////////////////////////////////////////////////////////////////////////
void bcopy(const void *src, void *dest, int len)
{
	if (dest < src) {
		const char *firsts = (const char *)src;
		char *firstd = (char *)dest;
		while (len--)
			*firstd++ = *firsts++;
	} else {
		const char *lasts = (const char *)src + (len-1);
		char *lastd = (char *)dest + (len-1);
		while (len--)
			*lastd-- = *lasts--;
	}
}

void *memmove(void *s1, const void *s2, int n)
{
	bcopy(s2, s1, n);
	return s1;
}
#endif
