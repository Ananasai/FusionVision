#include <main.h>

/* Orientation
   - 0: 240x320 portrait 0'
   - 1: 320x240 landscape 90'
   - 2: 240x320 portrait 180'
   - 3: 320x240 landscape 270'
*/
#define  ILI9486_ORIENTATION      1

/* Color mode
   - 0: RGB565 (R:bit15..11, G:bit10..5, B:bit4..0)
   - 1: BRG565 (B:bit15..11, G:bit10..5, R:bit4..0)
*/
#define  ILI9486_COLORMODE        0

/* For multi-threaded or interrupt use, Lcd and Touchscreen simultaneous use can cause confusion (since it uses common I/O resources)
   If enabled, the Lcd functions wait until the touchscreen functions are run. The touchscreen query is not executed when Lcd is busy.
   - 0: multi-threaded protection disabled
   - 1: multi-threaded protection enabled
*/
#define  ILI9486_MULTITASK_MUTEX   0

//-----------------------------------------------------------------------------
// ILI9486 physic resolution (in 0 orientation)
#define  ILI9486_LCD_PIXEL_WIDTH  320
#define  ILI9486_LCD_PIXEL_HEIGHT 480

void     ili9486_Init(void);
uint16_t ili9486_ReadID(void);
void     ili9486_DisplayOn(void);
void     ili9486_DisplayOff(void);
void     ili9486_SetCursor(uint16_t Xpos, uint16_t Ypos);
void     ili9486_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGB_Code);
uint16_t ili9486_ReadPixel(uint16_t Xpos, uint16_t Ypos);
void     ili9486_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     ili9486_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void     ili9486_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void     ili9486_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode);
uint16_t ili9486_GetLcdPixelWidth(void);
uint16_t ili9486_GetLcdPixelHeight(void);
void     ili9486_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp);
void     ili9486_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData);
void     ili9486_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData);
void     ili9486_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix);

/* Touchscreen */
void     ili9486_ts_Init(uint16_t DeviceAddr);
uint8_t  ili9486_ts_DetectTouch(uint16_t DeviceAddr);
void     ili9486_ts_GetXY(uint16_t DeviceAddr, uint16_t *X, uint16_t *Y);
