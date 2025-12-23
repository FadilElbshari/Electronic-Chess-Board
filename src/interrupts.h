#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"

 // ============================= Serial ISR Variable =================================
extern volatile bit ReceivingLedState;
extern volatile bit LED_READY;
extern volatile bit CONNECTED;

extern volatile U8 xdata ReceivingLEDsCounter;
extern volatile U8 xdata ReceivingPositionCounter;
extern volatile char xdata MoveSquares[4];
extern volatile Bitboard xdata DisplayBoardLEDs;

extern volatile ISRState CurrentISRState;
// ===================================================================================

#endif