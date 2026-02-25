#include "lcd.h"
#include "types.h"
#include "helpers.h"
#include "hardware.h"

#define LCD_PRINT(str)      \
{                           \
    const char code *p = str; \
    while (*p)              \
        lcd_putc(*p++);     \
}

void lcd_enable_pulse() {
  LCD_EN = 1;
  delay_us(1);
  LCD_EN = 0;
  delay_us(50);
}

void lcd_send_nibble(U8 nibble) {
  LCD_D4 = (nibble >> 4) & 0x01;
  LCD_D5 = (nibble >> 5) & 0x01;
  LCD_D6 = (nibble >> 6) & 0x01;
  LCD_D7 = (nibble >> 7) & 0x01;

  lcd_enable_pulse();
}

void lcd_send_byte(U8 value, bit isData) {
  LCD_RS = isData;

  lcd_send_nibble(value & 0xF0);         // High nibble
  lcd_send_nibble((value << 4) & 0xF0);  // Low nibble
	
	delay_us(50);
}

void lcd_init() {
  delay_ms(20);

  LCD_RS = 0;
  LCD_EN = 0;

  lcd_send_nibble(0x30);
  delay_ms(5);

  lcd_send_nibble(0x30);
  delay_us(200);

  lcd_send_nibble(0x30);
  delay_us(200);

  // Switch to 4-bit mode
  lcd_send_nibble(0x20);
  delay_us(200);

  lcd_send_byte(0x28, 0);

  // Display ON, Cursor OFF
  lcd_send_byte(0x0C, 0);

  // Entry mode set
  lcd_send_byte(0x06, 0);

  // Clear display
  lcd_send_byte(0x01, 0);
  delay_ms(2);
}

void lcd_putc(char c) {
	lcd_send_byte(c, 1);
}

void lcd_print(const char code *str) {
    while (*str) lcd_putc(*str++);
}

void lcd_set_cursor(U8 row, U8 col) {
  U8 address;

  if (row == 0)
    address = 0x80 + col;
  else
    address = 0xC0 + col;

  lcd_send_byte(address, 0);
}