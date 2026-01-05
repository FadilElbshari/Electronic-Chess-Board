#include <REG52.H>


// Local Headers
#include "types.h"
#include "helpers.h"
#include "config.h"
#include "hardware.h"
#include "bitboard.h"
#include "uart.h"
#include "shift_registers.h"
#include "tm_ssd.h"
#include "interrupts.h"
#include "move_gen.h"
		

volatile bit uart_move_pending;
volatile U8 uart_from;
volatile U8 uart_to;
		
bit JustEnteredState = 1;
MainState CurrentMainState = TURNED_ON;
DetectionState CurrentDetectionState = NONE;

U8 DelayCounter = 0;

int main(void) {
	uart_init();
	init_shift_reg();
	set_leds(&ZeroBoard);
	
	TURN = WHITE;
	
	uart_move_pending = 0;
	
	DelayCounter = 10;
	while (1) {
		
		
		if (DelayCounter > 0) DelayCounter--;
		switch (CurrentMainState) {
			
			case TURNED_ON:
				if (JustEnteredState) {
					JustEnteredState = 0;
					set_leds(&OneBoard);
				}
				
				if (CONNECTED) {
					JustEnteredState = 1;
					DelayCounter = 10;
					CurrentMainState = AWAIT_POSITION_SET;
				}
				break;
				
			// AWAIT_POSITION_SET STATE MANAGEMENT
			case AWAIT_POSITION_SET:
				
				if (JustEnteredState && LED_READY) {
					JustEnteredState = 0;
					LED_READY = 0;
					set_leds(&DisplayBoardLEDs);
					DelayCounter = 2;
				}
				
				if (DelayCounter != 0) break;

				if (!read_and_verify_sensors()) {
					DelayCounter = 2;
					break;
				}
				MATCH = compare_boards(&DisplayBoardLEDs, &PolledBoard); 
				
				if (!MATCH) {
					DelayCounter = 2;
					break;
				}
				
				MOVE_RECEIVED = 0;
				CurrentBoard = PolledBoard;
				JustEnteredState = 1;
				CurrentMainState = DETECTING;
				break;
			
			// DETECTION STATE MANAGEMENT
			case DETECTING:
				if (JustEnteredState) {
					JustEnteredState = 0;
					set_leds(&ZeroBoard);
					DelayCounter = 2;
					
				}
				
				if (MOVE_RECEIVED) {
					CurrentMainState = AWAIT_POSITION_SET;
					JustEnteredState = 1;
					break;
				}
				
				if (LED_READY) {
					LED_READY = 0;
					set_leds(&DisplayBoardLEDs);
					DelayCounter = 2;
				}

				if (DelayCounter != 0) break;
				
				if (!read_and_verify_sensors()) {
							DelayCounter = 2;
							break;
				}
						
				MATCH = compare_boards(&CurrentBoard, &PolledBoard);

				if (!MATCH) {
					JustEnteredState = 1;
					CurrentMainState = CHANGE_DETECTED;
					break;
				}
				
				set_leds(&ZeroBoard);
				switch (CurrentDetectionState) {
					case NONE:
						break;
						
					case LIFT:
            // Piece returned
					  get_left_entered(&CurrentBoard, &PolledBoard);
            if (get_bit_count(LeftMask) == 0 && get_bit_count(EnteredMask) == 0) {
							CurrentDetectionState = NONE;
							LiftedPieceSquare = 0;
							set_leds(&ZeroBoard);
						}
            break;
				}
				
				DelayCounter = 2;
				break;
				
			// CHANGE_DETECTED STATE MANAGEMENT	
			case CHANGE_DETECTED:

				if (JustEnteredState) JustEnteredState = 0;
					
					get_left_entered(&CurrentBoard, &PolledBoard);
					
					switch (CurrentDetectionState){
						U8 i, j;
						case NONE:
							if (get_bit_count(LeftMask) == 1 && get_bit_count(EnteredMask) == 0) {
								bit found;
								found = 0;
								
								CurrentDetectionState = LIFT;
								CurrentMainState = DETECTING;
								JustEnteredState = 1;
								
								for (i=0; i<4; i++) {
									if (found) break;
									for (j=0; j<4; j++) {
										if ((LeftMask.RANK[i] >> j) & 1) {
											LegalMoves = get_legal_moves(i*4 + j);
											DisplayBoardLEDs = LegalMoves;
											LiftedPieceSquare = 0;
											LiftedPieceSquare = (i << 4) | j;
											LED_READY = 1;
											found = 1;
											break;
										}
									}
								}
								break;
							}
							CurrentMainState = ERROR_FLASH_ON;
							break;
							
					
						case LIFT:
							// Piece still lifted
							if (get_bit_count(LeftMask) == 1 && get_bit_count(EnteredMask) == 0) {
								CurrentMainState = DETECTING; 
								break;
								
							// Piece placed on new square, check if legal
							} else if (get_bit_count(LeftMask) == 1 && get_bit_count(EnteredMask) == 1) {
								bit legal;
								U8 ToSquare;
								
								U8 FromRank, FromFile, ToRank, ToFile;
								
								ToSquare = 0;
								legal = 0;
								
								for (i=0; i<4; i++) {
									if (EnteredMask.RANK[i] & LegalMoves.RANK[i]) {
										for (j = 0; j<4; j++) if ((EnteredMask.RANK[i] >> j) & 1) break;
											ToSquare = (i << 4) | j;
											legal = 1;
											break;
									}
								}
								
								if (!legal) {
									CurrentMainState = ERROR_FLASH_ON;
									uart_send_char('I');
									break;
								}
								
								FromRank = (LiftedPieceSquare >> 4) & 0x0F;
								FromFile = (LiftedPieceSquare) & 0x0F;
								
								ToRank = (ToSquare >> 4) & 0x0F;
								ToFile = (ToSquare) & 0x0F;
								
								if ((BoardState[FromRank * 4 + FromFile] & TYPE_MASK) == TYPE_KING) {
									KingSquares[TURN] = (ToRank << 2) | ToFile;
								}
								
								
								BoardState[ToRank * 4 + ToFile] = BoardState[FromRank * 4 + FromFile];
								BoardState[FromRank * 4 + FromFile] = EMPTY;
								
								CurrentBoard.RANK[FromRank] &= ~(1 << FromFile);
								CurrentBoard.RANK[ToRank] |= 1 << ToFile;
								
								// Toggle turn
								TURN = !TURN;
								
								CurrentDetectionState = NONE;
								CurrentMainState = DETECTING;
								LegalMoves = ZeroBoard;
								
								uart_move_pending = 1;
								uart_from = LiftedPieceSquare;
								uart_to   = ToSquare;
								
								break;
							}
							
							break;
						}
			
				break;
				
			
			case ERROR_FLASH_ON:
				if (JustEnteredState) {
					JustEnteredState = 0;
					set_leds(&OneBoard);
				}
				if (DelayCounter != 0) break;
				CurrentMainState = ERROR_FLASH_OFF;
				DelayCounter = 10;
				JustEnteredState = 1;
				break;
			
			case ERROR_FLASH_OFF:
				if (JustEnteredState) {
					JustEnteredState = 0;
					set_leds(&ZeroBoard);
				}
				if (DelayCounter != 0) break;
				CurrentMainState = ERROR_FLASH_ON;
				DelayCounter = 10;
				JustEnteredState = 1;
				break;
		}
		
		if (uart_move_pending) {
            uart_move_pending = 0;
            uart_send_char('M');
            uart_send_char(uart_from);
            uart_send_char(uart_to);
        }
		
		delay_ms(10);
	}
}

