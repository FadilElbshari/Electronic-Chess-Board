 #ifndef HARDWARE_H
#define HARDWARE_H

#include <REG52.H>

/* ================================================
							PORT 0
	 ================================================ */
	 
	 // SERIAL IN PARALLEL OUT
sbit SERIAL_DATA_OUT = P0^7;
sbit SERIAL_CLK = P0^6;
sbit LATCH_RELEASE = P0^5;
sbit MASTER_RESET = P0^4;

// PARALLEL IN SERIAL OUT
sbit SERIAL_DATA_IN = P0^3; // SDA From Shift Reg to MCU
sbit PARALLEL_CLK = P0^2; // CLK
sbit SHIFT_nLOAD = P0^1; // PL'
sbit CLK_ENABLE = P0^0; // CE'

/* ================================================
											 PORT 1
	 ================================================ */
	 
sbit LCD_RS = P1^0;
sbit LCD_EN = P1^1;
sbit LCD_D4 = P1^2;
sbit LCD_D5 = P1^3;
sbit LCD_D6 = P1^4;
sbit LCD_D7 = P1^5;


/* ================================================
	PORT 3 (P3.0, P3.1: Bluetooth), (P3.6, P3.7: WR-RD)
	 ================================================ */
// Seven-Segment-Display
sbit SSD_CLK = P3^4;
sbit SSD_DIO = P3^5;


#endif