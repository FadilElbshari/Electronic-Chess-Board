#include "interrupts.h"
#include <REG52.H>

#include "types.h"
#include "config.h"


/* ================= Serial ISR variables ================= */

volatile bit ReceivingLedState = 0;
volatile bit LED_READY = 0;
volatile bit CONNECTED = 0;

volatile U8 xdata ReceivingLEDsCounter = 0;
volatile U8 xdata ReceivingPositionCounter = 0;

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
						DisplayBoardLEDs.RANK[3-(ReceivingPositionCounter++)] = c;
						if (ReceivingPositionCounter == 4) {
							ReceivingPositionCounter = 0;
							CurrentISRState = WAITING;
							LED_READY = 1;
						}
						break;
						
					case WAITING:
						if (c == LEGAL_MOVE_FLAG) {
							CurrentISRState = RECEIVING_POSITION;
						}
						break;
						
				}
				
				return;
        }
				
}