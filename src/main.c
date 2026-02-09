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
#include "LiquidCrystal_I2C_8051.h"
		
		
#define CLEAR_LEDS() set_leds(&ZeroBoard)

		
bit JustEnteredState;
U8 CurrentMainState;
U8 CurrentDetectionState;

U8 ErrorFlashCount;

U8 DelayCounter;

void reset_game() {
	JustEnteredState = 1;
	CurrentMainState = TURNED_ON;
	CurrentDetectionState = NONE;

	ErrorFlashCount = 0;

	DelayCounter = 0;
	
	RookMoved[0][0] = 0;
	RookMoved[0][1] = 0;
	RookMoved[1][0] = 0;
	RookMoved[1][1] = 0;
	
	KingMoved[0] = 0;
	KingMoved[1] = 0;
	
	TURN = WHITE;
}


int main(void) {
	
	uart_init();
	init_shift_reg();
	reset_game();
	CLEAR_LEDS();
	LCD_begin();
	
	
	TURN = WHITE;
	
	DelayCounter = 10;
	while (1) {
		
		if (rxPacketReady) process_rx_packet();
		if (txPacketReady) process_tx_packet();
		
		if (IS_RESET) {
			IS_RESET = 0;
			reset_game();
		}
		
		if (MOVE_RECEIVED) {
			
			EA = 0;  // Disable interrupts
			MOVE_RECEIVED = 0;
			EA = 1;  // Re-enable
			tm_display_digits(MoveSquares[1] + 9 + 1, MoveSquares[0] + 1, MoveSquares[3] + 9 + 1, MoveSquares[2] + 1);
//			LCD_clear();
//			LCD_print("Move Received");
//			LCD_setCursor(1, 0);
//			LCD_print("Move: ");
			
			CurrentMainState = AWAIT_MOVE_SET;
			JustEnteredState = 1;
		}
		
		
		if (DelayCounter > 0) DelayCounter--;
		switch (CurrentMainState) {
			
			case TURNED_ON:
				if (JustEnteredState) {
					JustEnteredState = 0;
					set_leds(&OneBoard);
					LCD_clear();
					LCD_print("Waiting for");
					LCD_setCursor(1, 0);
					LCD_print("Connection..");
					
				}
				
				if (CONNECTED) {
					JustEnteredState = 1;
					DelayCounter = 10;
					CurrentMainState = AWAIT_INITIAL_POSITION_SET;
					LCD_clear();
					LCD_print("Connected..");
				}
				break;
				
			// AWAIT_INITIAL_POSITION_SET STATE MANAGEMENT
			case AWAIT_INITIAL_POSITION_SET:
				
				if (JustEnteredState){
					if (LED_READY) {
						JustEnteredState = 0;
						LED_READY = 0;
						CurrentBoard = DisplayBoardLEDs;
						set_leds(&DisplayBoardLEDs);
						DelayCounter = 2;
					}
						break;
				}
				
				if (DelayCounter != 0) break;

				if (!read_and_verify_sensors()) {
					DelayCounter = 2;
					break;
				}
				MATCH = compare_boards(&CurrentBoard, &PolledBoard); 
				
				if (!MATCH) {
					DelayCounter = 2;
					break;
				}
				
				JustEnteredState = 1;
				CurrentMainState = DETECTING;
				break;
				
				
			case AWAIT_MOVE_SET:
				if (JustEnteredState) {
						JustEnteredState = 0;
						set_leds(&DisplayBoardLEDs);
						DelayCounter = 2;
						break;
				}
				
				if (DelayCounter != 0) break;
				
				// Try to read sensors
				if (!read_and_verify_sensors()) {
						DelayCounter = 2;
						break;
				}
				
				// Check if board matches expected position
				MATCH = compare_boards(&MoveBoard, &PolledBoard);
				
				if (MATCH) {
						// Move completed successfully!
						U8 FromRank, FromFile, ToRank, ToFile;
						
						// Extract move coordinates from MoveSquares
						// MoveSquares format: [FromRank, FromFile, ToRank, ToFile]
						FromRank = MoveSquares[0];
						FromFile = MoveSquares[1];
						ToRank = MoveSquares[2];
						ToFile = MoveSquares[3];
						
						apply_move((MoveSquares[0] << SHIFT) | MoveSquares[1], (MoveSquares[2] << SHIFT) | MoveSquares[3], 0);
						
						if (is_game_over()) {
							CurrentMainState = GAME_IS_OVER;
							JustEnteredState = 1;
						}
						
						// Clear LEDs
						CLEAR_LEDS();
						
						// Back to detecting
						CurrentMainState = DETECTING;
						CurrentDetectionState = NONE;
						JustEnteredState = 1;
						
						break;
				}
				
				// Board doesn't match yet - keep waiting
				DelayCounter = 2;
				break;
			
			// DETECTION STATE MANAGEMENT
			case DETECTING:
				if (JustEnteredState) {
					tm_display_digits(3, 4, 4, 3);
					JustEnteredState = 0;
					CLEAR_LEDS();
					DelayCounter = 2;
					
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
				
				CLEAR_LEDS();
				switch (CurrentDetectionState) {
					case NONE:
						break;
						
					case LIFT:
            // Check if piece returned to original square
						get_left_entered(&CurrentBoard, &PolledBoard);
						
						if (get_bit_count(&LeftMask) == 0 && get_bit_count(&EnteredMask) == 0) {
								// Boards match - piece was returned
								U8 FromRank, FromFile;
								
								FromRank = LiftedPieceSquare >> SHIFT;
								FromFile = LiftedPieceSquare & MASK;

								
								// Verify piece is back on original square
								if ((PolledBoard.RANK[FromRank] >> FromFile) & 1) {
										CurrentDetectionState = NONE;
										LiftedPieceSquare = 0;
										CLEAR_LEDS();
								} else {
										// Boards match but piece not on original square - ERROR
										CurrentMainState = ERROR_FLASH_ON;
										CurrentDetectionState = NONE;
										CLEAR_LEGAL_MOVES();
								}
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

						case NONE:
							tm_display_digits(1, 1, 1, 1);
							if (get_bit_count(&LeftMask) == 1 && get_bit_count(&EnteredMask) == 0) {
								U8 row, col;
								bit found;
								found = 0;
								
								CurrentDetectionState = LIFT;
								CurrentMainState = DETECTING;
								JustEnteredState = 1;
								for (row=0; row<BOARD_W; row++) {
									if (found) break;
									for (col=0; col<BOARD_W; col++) {
										if ((LeftMask.RANK[row] >> col) & 1) {
											get_legal_moves((row << SHIFT) | col, &LegalMoves, 0);
											DisplayBoardLEDs = LegalMoves;
											LiftedPieceSquare = (row << SHIFT) | col;
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
							if (get_bit_count(&LeftMask) == 1 && get_bit_count(&EnteredMask) == 0) {
								CurrentMainState = DETECTING; 
								break;
								
								// Capture in progress: removed opponent's piece while holding yours
							} else if (get_bit_count(&LeftMask) == 2 && get_bit_count(&EnteredMask) == 0) {
									bit valid_capture;
									U8 i, j;
									
									valid_capture = 0;
									
									// Check if one of the left pieces is a legal capture square
									for (i=0; i<BOARD_W; i++) {
											for (j=0; j<BOARD_W; j++) {
													if ((LeftMask.RANK[i] >> j) & 1) {
															// Skip the originally lifted piece
															if (((i<< SHIFT) | j) == ((LiftedPieceSquare >> SHIFT) | (LiftedPieceSquare & MASK))) continue;
															
															// Check if this square is a legal capture
															if ((LegalMoves.RANK[i] >> j) & 1) {
																	valid_capture = 1;
																	break;
															}
													}
											}
											if (valid_capture) break;
									}
									
									if (!valid_capture) {
											CurrentMainState = ERROR_FLASH_ON;
											CurrentDetectionState = NONE;
											CLEAR_LEGAL_MOVES();
											break;
									}
									
									// Valid capture - wait for piece to be placed
									IntermediateBoard = PolledBoard;
									CurrentDetectionState = CAPTURE_INTERMEDIATE;
									CurrentMainState = DETECTING;
									break;
									
							// Piece placed on new square, check if legal
							} else if (get_bit_count(&LeftMask) == 1 && get_bit_count(&EnteredMask) == 1) {
									bit legal;
									U8 ToSquare;
									U8 row, col;
									
									ToSquare = 0;
									legal = 0;
									
									for (row=0; row<BOARD_W; row++) {
										if (EnteredMask.RANK[row] & LegalMoves.RANK[row]) {
											for (col = 0; col<BOARD_W; col++) if ((EnteredMask.RANK[row] >> col) & 1) break;
												ToSquare = (row << SHIFT) | col;
												legal = 1;
												break;
										}
									}
									
									if (!legal) {
										CurrentMainState = ERROR_FLASH_ON;
										CurrentDetectionState = NONE;
										CLEAR_LEGAL_MOVES();
										break;
									}
									
									CurrentDetectionState = NONE;
									CurrentMainState = DETECTING;
									CLEAR_LEGAL_MOVES();
									
									apply_move(LiftedPieceSquare, ToSquare, 1);
									
									if (is_game_over()) {
										CurrentMainState = GAME_IS_OVER;
										JustEnteredState = 1;
									}
									
									break;
								
							} else {
									CurrentMainState = DETECTING; 
								break;
							}
							
							
						case CAPTURE_INTERMEDIATE:
							get_left_entered(&IntermediateBoard, &PolledBoard);
						
							// Waiting for captured piece to be placed on new square
							if (get_bit_count(&LeftMask) == 0 && get_bit_count(&EnteredMask) == 1) {
									bit legal;
									U8 ToSquare;
									U8 i, j;
									
									legal = 0;
									
									// Find where piece was placed
									for (i=0; i<BOARD_W; i++) {
											if (EnteredMask.RANK[i] & LegalMoves.RANK[i]) {
													for (j = 0; j<BOARD_W; j++) {
															if ((EnteredMask.RANK[i] >> j) & 1) {
																	ToSquare = (i << SHIFT) | j;
																	legal = 1;
																	break;
															}
													}
													if (legal) break;
											}
									}
									
									if (!legal) {
											CurrentMainState = ERROR_FLASH_ON;
											CurrentDetectionState = NONE;
											CLEAR_LEGAL_MOVES();
											break;
									}
									
									CurrentDetectionState = NONE;
									CurrentMainState = DETECTING;
									CLEAR_LEGAL_MOVES();
									
									apply_move(LiftedPieceSquare, ToSquare, 1);
									
									if (is_game_over()) {
										CurrentMainState = GAME_IS_OVER;
										JustEnteredState = 1;
									}
									
									break;
							}
							
							// Still in intermediate state
							else if (get_bit_count(&LeftMask) == 0 && get_bit_count(&EnteredMask) == 0) {
									CurrentMainState = DETECTING;
									break;
							}
							
							// Unexpected state
							else {
									CurrentMainState = ERROR_FLASH_ON;
									CurrentDetectionState = NONE;
									CLEAR_LEGAL_MOVES();
									break;
							}
							
							default:
								// Should never reach here, but recover gracefully
								CurrentMainState = ERROR_FLASH_ON;
								CurrentDetectionState = NONE;
								CLEAR_LEGAL_MOVES();
								break;
						}
			
				break;
				
			
			case GAME_IS_OVER:
				if (JustEnteredState) {
					Bitboard display_over;
					U8 square;
					
					JustEnteredState = 0;
					square = KingSquares[TURN];
					
					if (GAME_OVER_INFO == 1) {
						display_over.RANK[0] = 0;
						display_over.RANK[1] = 0;
						display_over.RANK[2] = 0;
						display_over.RANK[3] = 0;
						display_over.RANK[4] = 0;
						display_over.RANK[5] = 0;
						display_over.RANK[6] = 0;
						display_over.RANK[7] = 0;
						display_over.RANK[square >> SHIFT] |= (1 << (square & MASK));
					} else if (GAME_OVER_INFO == 0) {
						display_over.RANK[0] = 0xFF;
						display_over.RANK[1] = 0xFF;
						display_over.RANK[2] = 0xFF;
						display_over.RANK[3] = 0xFF;
						display_over.RANK[4] = 0xFF;
						display_over.RANK[5] = 0xFF;
						display_over.RANK[6] = 0xFF;
						display_over.RANK[7] = 0xFF;
						
						display_over.RANK[square >> SHIFT] &= ~(1 << (square & MASK));
					}
					set_leds(&display_over);
				}
				break;
								
							
			case ERROR_FLASH_ON:
					if (JustEnteredState) {
							JustEnteredState = 0;
							set_leds(&OneBoard);  // All LEDs on = ERROR
							ErrorFlashCount++;
					}
					
					if (DelayCounter != 0) break;
					
					if (ErrorFlashCount >= 6) {
							ErrorFlashCount = 0;
							
							if (read_and_verify_sensors()) {
									MATCH = compare_boards(&CurrentBoard, &PolledBoard);
									
									if (MATCH) {
											// Board corrected!
											CurrentMainState = DETECTING;
											CurrentDetectionState = NONE;
											CLEAR_LEDS();
											JustEnteredState = 1;
											DelayCounter = 10;
											break;
									}
							}
							
							// Still wrong - show expected position
							set_leds(&CurrentBoard);  // Show where pieces SHOULD be
							DelayCounter = 50;  // Display for 500ms
							ErrorFlashCount = 0;  // Reset to flash again
							break;
					}
					
					CurrentMainState = ERROR_FLASH_OFF;
					DelayCounter = 10;
					JustEnteredState = 1;
					break;

			case ERROR_FLASH_OFF:
				if (JustEnteredState) {
						JustEnteredState = 0;
						CLEAR_LEDS();
				}
				
				if (DelayCounter != 0) break;
				CurrentMainState = ERROR_FLASH_ON;
				DelayCounter = 10;
				JustEnteredState = 1;
				break;
		
		}
		
		delay_ms(10);
	}
}

