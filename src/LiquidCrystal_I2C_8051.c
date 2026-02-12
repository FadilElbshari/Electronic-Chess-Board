#include <REG52.h>
#include <intrins.h>
#include "LiquidCrystal_I2C_8051.h"
#include "i2c.h"
#include "helpers.h"
#include "types.h"
#include "string.h"


void LCD_expanderWrite(U8 _data) {
	i2c_begin();
	i2c_write(PCF8574_ADDR);
	i2c_write(_data | LCD_BACKLIGHT);
	i2c_stop();
}

void LCD_pulseEnable(U8 _data) {
	LCD_expanderWrite(_data | LCD_EN);
	delay_us(2);

	LCD_expanderWrite(_data & ~LCD_EN);
	delay_us(100);
}

void LCD_write4bits(U8 value) {
	LCD_expanderWrite(value);
	LCD_pulseEnable(value);
}

void LCD_send(U8 value, U8 mode) {
	LCD_write4bits((value&0xf0)|mode);
	LCD_write4bits(((value<<4)&0xf0)|mode);
}

void LCD_command(U8 value) {
	LCD_send(value, 0);
}

void LCD_write(U8 value) {
	LCD_send(value, LCD_RS);
}


void LCD_begin() {

	delay_us(50);

	LCD_expanderWrite(LCD_BACKLIGHT);	// reset expanderand turn backlight off (Bit 8 =1)
	delay_us(1000);

	LCD_write4bits(0x03 << 4);
	delay_us(4500); // wait min 4.1ms

	// second try
	LCD_write4bits(0x03 << 4);
	delay_us(4500); // wait min 4.1ms

	// third go!
	LCD_write4bits(0x03 << 4);
	delay_us(150);

	// finally, set to 4-bit interface
	LCD_write4bits(0x02 << 4);


	LCD_command(0x28); // 4-bit, 2-line
	
	LCD_command(LCD_DISPLAYCONTROL); // display off
	LCD_clear();
	delay_us(2000);
	LCD_command(0x06); // entry mode
	LCD_command(0x0C); // display ON

	LCD_home();
}

void LCD_clear() {
	LCD_command(LCD_CLEARDISPLAY);
	delay_us(2000);

}

void LCD_home(){
	LCD_command(LCD_RETURNHOME);
	delay_us(2000);
}

void LCD_print(char *s) {
	U8 xdata len, xdata i;
	len = strlen(s);
	len = (COLS-len) >> 1;
	for (i=0; i<len; i++) LCD_write('-');
	while(*s) LCD_write(*s++);
	if (len & 0x01) len+=1;
	for (i=0; i<len; i++) LCD_write('-');
}

void LCD_setCursor(U8 row, U8 col){
	LCD_command(LCD_SETDDRAMADDR | (col + row ? 0x40 : 0x00));
}

