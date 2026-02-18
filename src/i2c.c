#include <REG52.h>
#include "i2c.h"
#include "helpers.h"
#include "hardware.h"
#include "types.h"

void i2c_begin()
{
    LCD_SDA = 1; LCD_SCL = 1; delay_us(10);
    LCD_SDA = 0; delay_us(10);
    LCD_SCL = 0;
}

void i2c_stop()
{
    LCD_SDA = 0; delay_us(10);
    LCD_SCL = 1; delay_us(10);
    LCD_SDA = 1; delay_us(10);
}

void i2c_write(U8 dat)
{
    U8 idata i;
    for(i=0;i<8;i++)
    {
        LCD_SDA = (dat & 0x80) ? 1 : 0;
        dat <<= 1;
        delay_us(10);
        LCD_SCL = 1; delay_us(10);
        LCD_SCL = 0;
    }

    /* ACK (ignored but correct timing) */
    LCD_SDA = 1;
    delay_us(10);
    LCD_SCL = 1; delay_us(10);
    LCD_SCL = 0;
}