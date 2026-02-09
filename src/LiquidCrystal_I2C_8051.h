#ifndef LIQUIDCRYSTAL_I2C_8051_H
#define LIQUIDCRYSTAL_I2C_8051_H

#include "types.h"

#define COLS 16
#define ROWS 2

#define LCD_RS  (1 << 0)
#define LCD_EN  (1 << 2)
#define LCD_RW  (1 << 1)
#define LCD_D4  (1 << 4)

#define LCD_D5  (1 << 5)
#define LCD_D6  (1 << 6)
#define LCD_D7  (1 << 7)
#define LCD_BL (1 << 3)

#define PCF8574_ADDR 0x4E   

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00


void LCD_expanderWrite(U8 _data);
void LCD_send(U8 value, U8 mode);
void LCD_write4bits(U8 value);
void LCD_pulseEnable(U8 _data);
void LCD_command(U8 value);
void LCD_write(U8 value);

void LCD_begin();
void LCD_clear();
void LCD_home();

void LCD_setCursor(U8 row, U8 col);
void LCD_print(char* s);

	
#endif
