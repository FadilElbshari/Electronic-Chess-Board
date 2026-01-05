#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"

 // ============================= Serial ISR Variable =================================
extern volatile bit LED_READY;
extern volatile bit CONNECTED;
extern volatile bit POSITION_DONE;
extern volatile bit MOVE_RECEIVED;

extern volatile U8 xdata ReceivingMoveCounter;
extern volatile U8 xdata ReceivingPositionCounter;
extern volatile U8 xdata MoveSquares[4];
extern volatile Bitboard xdata DisplayBoardLEDs;

extern volatile ISRState CurrentISRState;
// ===================================================================================

#endif