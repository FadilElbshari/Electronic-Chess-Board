#ifndef HARDWARE_H
#define HARDWARE_H

#include <REG52.H>

/* ================================================
											 PORT 0 - 8/8
	 ================================================ */
	 	
		// Seven-Segment-Display
		sbit SSD_DIO = P0^0;
		sbit SSD_CLK = P0^1;
		
		sbit LCD_RS = P0^2;
		sbit LCD_EN = P0^3;
		sbit LCD_D4 = P0^4;
		sbit LCD_D5 = P0^5;
		sbit LCD_D6 = P0^6;
		sbit LCD_D7 = P0^7;

/* ================================================
											 PORT 1 - 0/8
	 ================================================ */
	 
	 sbit BTN_RESYNC = P1^0;
	 sbit BTN_ADJUST_ENGINE = P1^1;
		

/* ================================================
											 PORT 2 - 8/8
	 ================================================ */
	 
	 	 // SERIAL IN PARALLEL OUT
		sbit SERIAL_DATA_OUT = P2^0;
		sbit SERIAL_CLK = P2^1;
		sbit LATCH_RELEASE = P2^2;
		sbit MASTER_RESET = P2^3;

		// PARALLEL IN SERIAL OUT
		sbit SERIAL_DATA_IN = P2^4; // SDA From Shift Reg to MCU
		sbit PARALLEL_CLK = P2^5; // CLK
		sbit SHIFT_nLOAD = P2^6; // PL'
		sbit CLK_ENABLE = P2^7; // CE'


/* ================================================
											 PORT 3 - 4/8
	 ================================================ */
	 



#endif