#ifndef LCD_H
#define LCD_H

#include "types.h"

void lcd_enable_pulse();
void lcd_send_nibble(U8 nibble);
void lcd_send_byte(U8 value, bit isData);
void lcd_init();

void lcd_putc(char c);
void lcd_print(const char* str);
void lcd_set_cursor(U8 row, U8 col);

#endif