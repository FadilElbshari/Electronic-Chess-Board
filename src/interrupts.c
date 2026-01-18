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

volatile U8 ReceivingMoveCounter = 0;
volatile U8 ReceivingPositionCounter = 0;

volatile U8 MoveSquares[4] = {0};

volatile ISRState CurrentISRState = NOT_CONNECTED;
volatile Bitboard xdata DisplayBoardLEDs;

volatile RX_STATE rxState = RX_WAIT_HEADER;
volatile U8 rxType;
volatile U8 rxLen;
volatile U8 rxIndex;
volatile U8 rxChecksum;
volatile U8 xdata rxBuffer[MAX_RX_PAYLOAD];
volatile bit rxPacketReady = 0;


volatile U8 txHeader;
volatile U8 txType;
volatile U8 txLen;
volatile U8 txChecksum;
volatile U8 xdata txBuffer[MAX_TX_PAYLOAD];
volatile bit txPacketReady = 0;

/* ======================================================== */

void serial_ISR(void) interrupt 4 {
    if (RI) {
        U8 c;
        c = SBUF;
				RI = 0;
			
			switch (rxState) {

        case RX_WAIT_HEADER:
            if (c == 0xAA) {
                rxChecksum = 0;
                rxState = RX_WAIT_TYPE;
            }
            break;

        case RX_WAIT_TYPE:
            rxType = c;
            rxChecksum ^= c;
            rxState = RX_WAIT_LEN;
            break;

        case RX_WAIT_LEN:
            rxLen = c;
            rxIndex = 0;
            rxChecksum ^= c;
						
						if (rxLen > 0) {
							rxState = RX_WAIT_DATA;
							break;
							
						}
						rxState = RX_WAIT_CHECKSUM;
						break;
            

        case RX_WAIT_DATA:
					rxBuffer[rxIndex++] = c;
					rxChecksum ^= c;

						if (rxIndex >= rxLen) {
								rxState = RX_WAIT_CHECKSUM;
						}
						break;

        case RX_WAIT_CHECKSUM:
            if (c == rxChecksum) {
                rxPacketReady = 1;   // signal main loop
            }
            rxState = RX_WAIT_HEADER; // always reset
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

void process_rx_packet(void) {
	U8 i, j;
	
	if (!rxPacketReady) return;
	rxPacketReady = 0;

	switch (rxType) {
		case CONNECTION_PACKET:
			CONNECTED = 1;
			break;
		
		case BOARD_PACKET:
			for (i=0; i<BOARD_W*BOARD_W; i++) {
				U8 c;
				c = rxBuffer[i];
				BoardState[i] = c;
				if ((c & TYPE_MASK) == TYPE_KING) {
					bit piece_turn;
					piece_turn = (c & COLOR_WHITE) != 0;
					KingSquares[piece_turn] = i;
				}
			}
			COLOR = rxBuffer[i];
			
			DisplayBoardLEDs.RANK[0] = 0;
			DisplayBoardLEDs.RANK[1] = 0;
			DisplayBoardLEDs.RANK[2] = 0;
			DisplayBoardLEDs.RANK[3] = 0;
							
			for (i=0; i<BOARD_W; i++){
				for (j=0; j<BOARD_W; j++) {
					if (((BoardState[(i << 2) + j] & TYPE_MASK) != TYPE_EMPTY)) {
						DisplayBoardLEDs.RANK[i] |= (1 << j);
					}
				}
			}
			LED_READY = 1;
			
			break;
			
		
		case MOVE_PACKET:	
			DisplayBoardLEDs.RANK[0] = 0;
			DisplayBoardLEDs.RANK[1] = 0;
			DisplayBoardLEDs.RANK[2] = 0;
			DisplayBoardLEDs.RANK[3] = 0;
		
			for (i=0; i<4; i++) MoveSquares[i] = rxBuffer[i];
	
			DisplayBoardLEDs.RANK[MoveSquares[0]] |= 1 << MoveSquares[1];
			DisplayBoardLEDs.RANK[MoveSquares[2]] |= 1 << MoveSquares[3];
		
			MoveBoard = CurrentBoard;
		
			MoveBoard.RANK[MoveSquares[0]] &= ~(1 << MoveSquares[1]);
			MoveBoard.RANK[MoveSquares[2]] |= 1 << MoveSquares[3];
		
			MOVE_RECEIVED = 1;
			break;
	}
}



void process_tx_packet(void) {
	U8 i, c;
	if (!txPacketReady) return;
	txPacketReady = 0;
	
	txChecksum = 0;
	
	uart_send_byte(txHeader);
	uart_send_byte(txType);
	uart_send_byte(txLen);
	
	txChecksum ^= txType;
	txChecksum ^= txLen;
	
	for (i=0; i<txLen; i++) {
		c = txBuffer[i];
		txChecksum ^= c;
		uart_send_byte(c);
	}
	
	uart_send_byte(txChecksum);
}