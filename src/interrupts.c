#include "interrupts.h"
#include <REG52.H>

#include "types.h"
#include "config.h"
#include "uart.h"
#include "bitboard.h"


/* ================= Serial ISR variables ================= */

volatile bit LED_READY = 0;
volatile bit CONNECTED = 0;
volatile bit POSITION_DONE = 0;
volatile bit MOVE_RECEIVED = 0;

volatile U8 xdata ReceivingMoveCounter = 0;
volatile U8 xdata ReceivingPositionCounter = 0;

volatile U8 xdata MoveSquares[4] = {0};

volatile ISRState CurrentISRState = NOT_CONNECTED;
volatile Bitboard xdata DisplayBoardLEDs = {{0x00, 0x00, 0x00, 0x00}};

/* ======================================================== */

void serial_ISR(void) interrupt 4 {
    if (RI) {
        U8 c;
        c = SBUF;
				RI = 0;
				
				switch (CurrentISRState) {
					case NOT_CONNECTED:
						if (c == CONNECTION_FLAG) {
							CONNECTED = 1;
							CurrentISRState = RECEIVING_POSITION;
						}
						break;
						
					case RECEIVING_POSITION:
						if (ReceivingPositionCounter == 16) {
							COLOR = c;
							POSITION_DONE = 1;
						}
						if (!POSITION_DONE) {
							if ((c & TYPE_MASK) == TYPE_KING) {
								bit piece_turn;
								piece_turn = (c & COLOR_WHITE) != 0;
								
								KingSquares[piece_turn] = ReceivingPositionCounter;
							}
							BoardState[ReceivingPositionCounter++] = c;
						}
						if (POSITION_DONE && (ReceivingPositionCounter == 16)) {
							U8 i, j;
							
							DisplayBoardLEDs = ZeroBoard;
							
							for (i=0; i<4; i++){
								for (j=0; j<4; j++) {
									if (((BoardState[i*4 + j] & TYPE_MASK) != TYPE_EMPTY)) {
										DisplayBoardLEDs.RANK[i] |= (1 << j);
									}
								}
							}
							
							ReceivingPositionCounter = 0;
							CurrentISRState = WAITING;
							LED_READY = 1;
						}
						break;
						
					case RECEIVING_MOVE:
						MoveSquares[ReceivingMoveCounter++] = c;
						
						if (ReceivingMoveCounter == 4) {
							DisplayBoardLEDs = ZeroBoard;
							DisplayBoardLEDs.RANK[MoveSquares[0]] |= (1 << MoveSquares[1]);
							DisplayBoardLEDs.RANK[MoveSquares[2]] |= (1 << MoveSquares[3]);
							ReceivingMoveCounter = 0;
							CurrentISRState = WAITING;
							LED_READY = 1;
							MOVE_RECEIVED = 1;
						}
					
						break;
						
					case WAITING:
						if (c == MOVE_FLAG) {
							ReceivingMoveCounter = 0;
							CurrentISRState = RECEIVING_MOVE;
							MOVE_RECEIVED = 0;
						}
						break;
						
				}
				
				return;
        }
		
			if (TI) {
        TI = 0;

        if (tx_tail != tx_head) {
            SBUF = tx_buf[tx_tail];
            tx_tail = (tx_tail + 1) % TX_BUF_SIZE;
        } else {
            tx_busy = 0;   // Nothing left to send
        }
    }
				
}