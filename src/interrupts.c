#include "interrupts.h"
#include <REG52.H>

#include "types.h"
#include "config.h"
#include "uart.h"
#include "bitboard.h"
#include "helpers.h"

#define STARTING_SQUARE_WHITE_KING 0x04
#define STARTING_SQUARE_BLACK_KING 0x3C


// Serial interrupt variables - all marked volatile

volatile FLAG LED_READY = 0;
volatile FLAG CONNECTED = 0;
volatile FLAG POSITION_DONE = 0;
volatile FLAG MOVE_RECEIVED = 0;
volatile FLAG IS_RESET = 0;

volatile U8 MoveSquares[4] = {0};

volatile ISRState CurrentISRState = NOT_CONNECTED;
volatile Bitboard idata DisplayBoardLEDs;

volatile RX_STATE rxState = RX_WAIT_HEADER;
volatile U8 rxType;
volatile U8 rxLen;
volatile U8 idata rxIndex;
volatile U8 idata rxChecksum;
volatile U8 xdata rxBuffer[MAX_RX_PAYLOAD];
volatile FLAG RX_PACKET_READY = 0;


volatile U8 idata txHeader;
volatile U8 idata txType;
volatile U8 idata txLen;
volatile U8 idata txChecksum;
volatile U8 idata txBuffer[MAX_TX_PAYLOAD];
volatile FLAG TX_PACKET_READY = 0;



void serial_ISR(void) interrupt 4 {
    if (RI) {
        U8 c;
        c = SBUF;
				RI = 0;
			
			// RX state machine
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
                RX_PACKET_READY = 1;   // signal main loop
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


// Managing received data
void process_rx_packet(void) {
	U8 i, j;
	
	if (!RX_PACKET_READY) return;
	RX_PACKET_READY = 0;

	switch (rxType) {
		case CONNECTION_PACKET:
			IS_RESET = 1;
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
					
					if (i != STARTING_SQUARE_WHITE_KING && i != STARTING_SQUARE_BLACK_KING) KingMoved[piece_turn] = -1;
				}
			}
			TURN = rxBuffer[i++] ? WHITE : BLACK;
			COLOR = rxBuffer[i++] ? WHITE : BLACK;
			
			DisplayBoardLEDs.RANK[0] = 0;
			DisplayBoardLEDs.RANK[1] = 0;
			DisplayBoardLEDs.RANK[2] = 0;
			DisplayBoardLEDs.RANK[3] = 0;
			DisplayBoardLEDs.RANK[4] = 0;
			DisplayBoardLEDs.RANK[5] = 0;
			DisplayBoardLEDs.RANK[6] = 0;
			DisplayBoardLEDs.RANK[7] = 0;
							
			for (i=0; i<BOARD_W; i++){
				for (j=0; j<BOARD_W; j++) {
					if (((BoardState[(i << SHIFT) | j] & TYPE_MASK) != TYPE_EMPTY)) {
						DisplayBoardLEDs.RANK[i] |= BitMask[j];
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
			DisplayBoardLEDs.RANK[4] = 0;
			DisplayBoardLEDs.RANK[5] = 0;
			DisplayBoardLEDs.RANK[6] = 0;
			DisplayBoardLEDs.RANK[7] = 0;
		
			for (i=0; i<4; i++) MoveSquares[i] = rxBuffer[i];
	
			DisplayBoardLEDs.RANK[MoveSquares[0]] |= BitMask[MoveSquares[1]];
			DisplayBoardLEDs.RANK[MoveSquares[2]] |= BitMask[MoveSquares[3]];
		
			MOVE_RECEIVED = 1;
			break;
	}
}


// Managing data to be transmitted
void process_tx_packet(void) {
	U8 i, c;
	if (!TX_PACKET_READY) return;
	TX_PACKET_READY = 0;
	
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