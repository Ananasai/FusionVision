/*
 * LCD_ILI9486.c
 *
 *  Created on: Dec 15, 2023
 *      Author: auris
 */


#include <string.h>
#include "main.h"
#include "LCD_ILI9486.h"

static  uint16_t  yStart, yEnd;
#define RD(a)                 __REVSH(a)

#define  LCD_IO_WriteData16_to_2x8(dt)    {LCD_IO_WriteData8((dt) >> 8); LCD_IO_WriteData8(dt); }

#define ILI9486_NOP            0x00
#define ILI9486_SWRESET        0x01
#define ILI9486_RDDID          0x04
#define ILI9486_RDDST          0x09
#define ILI9486_RDMODE         0x0A
#define ILI9486_RDMADCTL       0x0B
#define ILI9486_RDPIXFMT       0x0C
#define ILI9486_RDIMGFMT       0x0D
#define ILI9486_RDSELFDIAG     0x0F
#define ILI9486_SLPIN          0x10
#define ILI9486_SLPOUT         0x11
#define ILI9486_PTLON          0x12
#define ILI9486_NORON          0x13
#define ILI9486_INVOFF         0x20
#define ILI9486_INVON          0x21
#define ILI9486_GAMMASET       0x26
#define ILI9486_DISPOFF        0x28
#define ILI9486_DISPON         0x29
#define ILI9486_CASET          0x2A
#define ILI9486_PASET          0x2B
#define ILI9486_RAMWR          0x2C
#define ILI9486_RAMRD          0x2E
#define ILI9486_PTLAR          0x30
#define ILI9486_VSCRDEF        0x33
#define ILI9486_MADCTL         0x36
#define ILI9486_VSCRSADD       0x37
#define ILI9486_PIXFMT         0x3A
#define ILI9486_RGB_INTERFACE  0xB0
#define ILI9486_FRMCTR1        0xB1
#define ILI9486_FRMCTR2        0xB2
#define ILI9486_FRMCTR3        0xB3
#define ILI9486_INVCTR         0xB4
#define ILI9486_DFUNCTR        0xB6
#define ILI9486_PWCTR1         0xC0
#define ILI9486_PWCTR2         0xC1
#define ILI9486_PWCTR3         0xC2
#define ILI9486_PWCTR4         0xC3
#define ILI9486_PWCTR5         0xC4
#define ILI9486_VMCTR1         0xC5
#define ILI9486_VMCTR2         0xC7
#define ILI9486_RDID1          0xDA
#define ILI9486_RDID2          0xDB
#define ILI9486_RDID3          0xDC
#define ILI9486_RDID4          0xDD
#define ILI9486_GMCTRP1        0xE0
#define ILI9486_GMCTRN1        0xE1
#define ILI9486_DGCTR1         0xE2
#define ILI9486_DGCTR2         0xE3

#define ILI9486_MAD_RGB        0x08
#define ILI9486_MAD_BGR        0x00
#define ILI9486_MAD_VERTICAL   0x20
#define ILI9486_MAD_X_LEFT     0x00
#define ILI9486_MAD_X_RIGHT    0x40
#define ILI9486_MAD_Y_UP       0x80
#define ILI9486_MAD_Y_DOWN     0x00

#if ILI9486_COLORMODE == 0
#define ILI9486_MAD_COLORMODE  ILI9486_MAD_RGB
#else
#define ILI9486_MAD_COLORMODE  ILI9486_MAD_BGR
#endif

#if (ILI9486_ORIENTATION == 0)
#define ILI9486_SIZE_X                     ILI9486_LCD_PIXEL_WIDTH
#define ILI9486_SIZE_Y                     ILI9486_LCD_PIXEL_HEIGHT
#define ILI9486_MAD_DATA_RIGHT_THEN_UP     ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_UP
#define ILI9486_MAD_DATA_RIGHT_THEN_DOWN   ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_DOWN
#define ILI9486_MAD_DATA_RGBMODE           ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_DOWN
#elif (ILI9486_ORIENTATION == 1)
#define ILI9486_SIZE_X                     ILI9486_LCD_PIXEL_HEIGHT
#define ILI9486_SIZE_Y                     ILI9486_LCD_PIXEL_WIDTH
#define ILI9486_MAD_DATA_RIGHT_THEN_UP     ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_DOWN | ILI9486_MAD_VERTICAL
#define ILI9486_MAD_DATA_RIGHT_THEN_DOWN   ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_DOWN | ILI9486_MAD_VERTICAL
#define ILI9486_MAD_DATA_RGBMODE           ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_DOWN
#elif (ILI9486_ORIENTATION == 2)
#define ILI9486_SIZE_X                     ILI9486_LCD_PIXEL_WIDTH
#define ILI9486_SIZE_Y                     ILI9486_LCD_PIXEL_HEIGHT
#define ILI9486_MAD_DATA_RIGHT_THEN_UP     ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_DOWN
#define ILI9486_MAD_DATA_RIGHT_THEN_DOWN   ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_UP
#define ILI9486_MAD_DATA_RGBMODE           ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_UP
#elif (ILI9486_ORIENTATION == 3)
#define ILI9486_SIZE_X                     ILI9486_LCD_PIXEL_HEIGHT
#define ILI9486_SIZE_Y                     ILI9486_LCD_PIXEL_WIDTH
#define ILI9486_MAD_DATA_RIGHT_THEN_UP     ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_UP   | ILI9486_MAD_VERTICAL
#define ILI9486_MAD_DATA_RIGHT_THEN_DOWN   ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_UP   | ILI9486_MAD_VERTICAL
#define ILI9486_MAD_DATA_RGBMODE           ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_UP
#endif

#define ILI9486_SETCURSOR(x, y)            {LCD_IO_WriteCmd8(ILI9486_CASET); LCD_IO_WriteData16_to_2x8(x); LCD_IO_WriteData16_to_2x8(x); \
                                            LCD_IO_WriteCmd8(ILI9486_PASET); LCD_IO_WriteData16_to_2x8(y); LCD_IO_WriteData16_to_2x8(y); }

#define ILI9486_LCD_INITIALIZED    0x01
#define ILI9486_IO_INITIALIZED     0x02

#define  ILI9486_LCDMUTEX_PUSH()
#define  ILI9486_LCDMUTEX_POP()

// LCD write commands
void LCD_IO_WriteCmd8(uint8_t Cmd)
{
  *(uint8_t *)LCD_ADDR_BASE = Cmd;
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd16(uint16_t Cmd)
{
  *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteData8(uint8_t Data)
{
  *(volatile uint8_t *)LCD_ADDR_DATA = Data;
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteData16(uint16_t Data)
{
  *(volatile uint16_t *)LCD_ADDR_DATA = RD(Data);
}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd8DataFill16(uint8_t Cmd, uint16_t Data, uint32_t Size)
{
  uint16_t d = RD(Data);
  *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;

  while(Size--)
    *(volatile uint16_t *)LCD_ADDR_DATA = d;

}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd8MultipleData8(uint8_t Cmd, uint8_t *pData, uint32_t Size)
{
  *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;

  while(Size--)
  {
    *(volatile uint8_t *)LCD_ADDR_DATA =*pData;
    pData++;
  }

}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd8MultipleData16(uint8_t Cmd, uint16_t *pData, uint32_t Size)
{
  *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;

  while(Size--)
  {
    *(volatile uint16_t *)LCD_ADDR_DATA = RD(*pData);
    pData++;
  }

}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd16DataFill16(uint16_t Cmd, uint16_t Data, uint32_t Size)
{
  *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);
  uint16_t d = RD(Data);

  while(Size--)
    *(volatile uint16_t *)LCD_ADDR_DATA = d;

}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd16MultipleData8(uint16_t Cmd, uint8_t *pData, uint32_t Size)
{
  *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);

  while(Size--)
  {
    *(volatile uint8_t *)LCD_ADDR_DATA =*pData;
    pData++;
  }

}

//-----------------------------------------------------------------------------
void LCD_IO_WriteCmd16MultipleData16(uint16_t Cmd, uint16_t *pData, uint32_t Size)
{
  *(volatile uint16_t *)LCD_ADDR_BASE = RD(Cmd);

  while(Size--)
  {
    *(volatile uint16_t *)LCD_ADDR_DATA = RD(*pData);
    pData++;
  }

}

//LCD general commands
void ili9486_DisplayOn(void)
{
  ILI9486_LCDMUTEX_PUSH();
  LCD_IO_WriteCmd8(ILI9486_SLPOUT);    // Exit Sleep
  ILI9486_LCDMUTEX_POP();
}

void ili9486_DisplayOff(void)
{
  ILI9486_LCDMUTEX_PUSH();
  LCD_IO_WriteCmd8(ILI9486_SLPIN);    // Sleep
  ILI9486_LCDMUTEX_POP();
}

void ili9486_Init(void)
{
  HAL_Delay(10);
  LCD_IO_WriteCmd8(ILI9486_SWRESET);
  HAL_Delay(100);

  LCD_IO_WriteCmd8MultipleData8(ILI9486_RGB_INTERFACE, (uint8_t *)"\x00", 1); // RGB mode off (0xB0)
  LCD_IO_WriteCmd8(ILI9486_SLPOUT);    // Exit Sleep (0x11)
  HAL_Delay(10);

  LCD_IO_WriteCmd8MultipleData8(ILI9486_PIXFMT, (uint8_t *)"\x55", 1); // interface format (0x3A)

  LCD_IO_WriteCmd8(ILI9486_MADCTL); LCD_IO_WriteData8(ILI9486_MAD_DATA_RIGHT_THEN_DOWN);

  LCD_IO_WriteCmd8MultipleData8(ILI9486_PWCTR3, (uint8_t *)"\x44", 1); // 0xC2
  LCD_IO_WriteCmd8MultipleData8(ILI9486_VMCTR1, (uint8_t *)"\x00\x00\x00\x00", 4); // 0xC5

  // positive gamma control (0xE0)
  LCD_IO_WriteCmd8MultipleData8(ILI9486_GMCTRP1, (uint8_t *)"\x0F\x1F\x1C\x0C\x0F\x08\x48\x98\x37\x0A\x13\x04\x11\x0D\x00", 15);

  // negative gamma control (0xE1)
  LCD_IO_WriteCmd8MultipleData8(ILI9486_GMCTRN1, (uint8_t *)"\x0F\x32\x2E\x0B\x0D\x05\x47\x75\x37\x06\x10\x03\x24\x20\x00", 15);

  // Digital gamma control1 (0xE2)
  LCD_IO_WriteCmd8MultipleData8(ILI9486_DGCTR1, (uint8_t *)"\x0F\x32\x2E\x0B\x0D\x05\x47\x75\x37\x06\x10\x03\x24\x20\x00", 15);

  LCD_IO_WriteCmd8(ILI9486_NORON);     // Normal display on (0x13)
  LCD_IO_WriteCmd8(ILI9486_INVOFF);    // Display inversion off (0x20)
  LCD_IO_WriteCmd8(ILI9486_SLPOUT);    // Exit Sleep (0x11)
  HAL_Delay(200);
  LCD_IO_WriteCmd8(ILI9486_DISPON);    // Display on (0x29)
  HAL_Delay(10);
}

void ili9486_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGBCode)
{
  ILI9486_LCDMUTEX_PUSH();
  ILI9486_SETCURSOR(Xpos, Ypos);
  LCD_IO_WriteCmd8(ILI9486_RAMWR);
  LCD_IO_WriteData16(RGBCode);
  ILI9486_LCDMUTEX_POP();
}

void ili9486_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  yStart = Ypos; yEnd = Ypos + Height - 1;
  ILI9486_LCDMUTEX_PUSH();
  LCD_IO_WriteCmd8(ILI9486_CASET); LCD_IO_WriteData16_to_2x8(Xpos); LCD_IO_WriteData16_to_2x8(Xpos + Width - 1);
  LCD_IO_WriteCmd8(ILI9486_PASET); LCD_IO_WriteData16_to_2x8(Ypos); LCD_IO_WriteData16_to_2x8(Ypos + Height - 1);
  ILI9486_LCDMUTEX_POP();
}

void ili9486_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  ILI9486_LCDMUTEX_PUSH();
  LCD_IO_WriteCmd8(ILI9486_CASET); LCD_IO_WriteData16_to_2x8(Xpos); LCD_IO_WriteData16_to_2x8(Xpos + Length - 1);
  LCD_IO_WriteCmd8(ILI9486_PASET); LCD_IO_WriteData16_to_2x8(Ypos); LCD_IO_WriteData16_to_2x8(Ypos);
  LCD_IO_WriteCmd8DataFill16(ILI9486_RAMWR, RGBCode, Length);
  ILI9486_LCDMUTEX_POP();
}

void ili9486_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  ILI9486_LCDMUTEX_PUSH();
  LCD_IO_WriteCmd8(ILI9486_CASET); LCD_IO_WriteData16_to_2x8(Xpos); LCD_IO_WriteData16_to_2x8(Xpos);
  LCD_IO_WriteCmd8(ILI9486_PASET); LCD_IO_WriteData16_to_2x8(Ypos); LCD_IO_WriteData16_to_2x8(Ypos + Length - 1);
  LCD_IO_WriteCmd8DataFill16(ILI9486_RAMWR, RGBCode, Length);
  ILI9486_LCDMUTEX_POP();
}

void ili9486_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode)
{
  ILI9486_LCDMUTEX_PUSH();
  LCD_IO_WriteCmd8(ILI9486_CASET); LCD_IO_WriteData16_to_2x8(Xpos); LCD_IO_WriteData16_to_2x8(Xpos + Xsize - 1);
  LCD_IO_WriteCmd8(ILI9486_PASET); LCD_IO_WriteData16_to_2x8(Ypos); LCD_IO_WriteData16_to_2x8(Ypos + Ysize - 1);
  LCD_IO_WriteCmd8DataFill16(ILI9486_RAMWR, RGBCode, Xsize * Ysize);
  ILI9486_LCDMUTEX_POP();
}

void ili9486_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData)
{
  ili9486_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
  ILI9486_LCDMUTEX_PUSH();
  LCD_IO_WriteCmd8MultipleData16(ILI9486_RAMWR, pData, Xsize * Ysize);
  ILI9486_LCDMUTEX_POP();
}

