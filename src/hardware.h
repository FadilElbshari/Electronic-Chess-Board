#ifndef HARDWARE_H
#define HARDWARE_H

#include <REG52.H>

/* ================================================
							PORT 0, PORT 2 FOR SRAM
	 ================================================ */

/* ================================================
											 PORT 1
	 ================================================ */
// SERIAL IN PARALLEL OUT
sbit SERIAL_DATA_OUT = P1^0;
sbit SERIAL_CLK = P1^1;
sbit LATCH_RELEASE = P1^2;
sbit MASTER_RESET = P1^3;

// PARALLEL IN SERIAL OUT
sbit SERIAL_DATA_IN = P1^4; // SDA From Shift Reg to MCU
sbit PARALLEL_CLK = P1^5; // CLK
sbit SHIFT_nLOAD = P1^6; // PL'
sbit CLK_ENABLE = P1^7; // CE'

/* ================================================
	PORT 3 (P3.0, P3.1: Bluetooth), (P3.6, P3.7: WR-RD)
	 ================================================ */
// Seven-Segment-Display
sbit SSD_CLK = P3^4;
sbit SSD_DIO = P3^5;

#endif