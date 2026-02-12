#include "tasks.h"
#include "types.h"
#include "interrupts.h"
#include "bitboard.h"
#include "helpers.h"
#include "shift_registers.h"
#include "move_gen.h"

FLAG JUST_ENTERED_STATE;
U8 CurrentMainState;
U8 CurrentDetectionState;
U8 ErrorFlashCount;

void reset_game() {
	JUST_ENTERED_STATE = 1;
	CurrentMainState = TURNED_ON;
	CurrentDetectionState = NONE;

	ErrorFlashCount = 0;

	ui_timer = 0;
	
	RookMoved[0][0] = 0;
	RookMoved[0][1] = 0;
	RookMoved[1][0] = 0;
	RookMoved[1][1] = 0;
	
	KingMoved[0] = 0;
	KingMoved[1] = 0;
	
	TURN = WHITE;
}


void task_handle_flags() {
	if (RX_PACKET_READY) process_rx_packet();
	if (TX_PACKET_READY) process_tx_packet();
	
	if (IS_RESET) {
		IS_RESET = 0;
		reset_game();
	}
	
	if (MOVE_RECEIVED) {
		
		EA = 0;  // Disable interrupts
		MOVE_RECEIVED = 0;
		EA = 1;  // Re-enable
		
		CurrentMainState = AWAIT_MOVE_SET;
		JUST_ENTERED_STATE = 1;
	}
}


void task_turnon() {
	if (JUST_ENTERED_STATE) {
		JUST_ENTERED_STATE = 0;
		set_leds(&OneBoard);
	}
		
	if (CONNECTED) {
		JUST_ENTERED_STATE = 1;
		ui_timer = 100;
		CurrentMainState = AWAIT_INITIAL_POSITION_SET;
	}
}

void task_await_initpos() {
	if (JUST_ENTERED_STATE){
		if (LED_READY) {
			JUST_ENTERED_STATE = 0;
			LED_READY = 0;
			CurrentBoard = DisplayBoardLEDs;
			set_leds(&DisplayBoardLEDs);
			ui_timer = 20;
		}
			return;
	}
	
	if (ui_timer != 0) return;

	if (!read_and_verify_sensors(SHORT)) {
		ui_timer = 20;
		return;
	}
	MATCH = compare_boards(&CurrentBoard, &PolledBoard); 
	
	if (!MATCH) {
		ui_timer = 20;
		return;
	}
	
	JUST_ENTERED_STATE = 1;
	CurrentMainState = DETECTING;
}

void task_await_moveset() {
	
	if (JUST_ENTERED_STATE) {
		JUST_ENTERED_STATE = 0;
		set_leds(&DisplayBoardLEDs);
		ui_timer = 20;
		return;
	}

	if (ui_timer != 0) return;
	

	// Try to read sensors
	if (!read_and_verify_sensors(SHORT)) {
			ui_timer = 20;
			return;
	}
	
	// DisplayBoardLEDs is used to show supposed position here
	board_copy(&DisplayBoardLEDs, &CurrentBoard);
	
	DisplayBoardLEDs.RANK[MoveSquares[0]] &= BitMaskClr[MoveSquares[1]];
	DisplayBoardLEDs.RANK[MoveSquares[2]] |= BitMask[MoveSquares[3]];

	// Check if board matches expected position
	MATCH = compare_boards(&DisplayBoardLEDs, &PolledBoard);

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
			JUST_ENTERED_STATE = 1;
		}
		
		// Clear LEDs
		clear_leds();
		
		// Back to detecting
		CurrentMainState = DETECTING;
		CurrentDetectionState = NONE;
		JUST_ENTERED_STATE = 1;
		
		return;
	}

	// Board doesn't match yet - keep waiting
	ui_timer = 20;
	
}


void task_gameover() {
	if (JUST_ENTERED_STATE) {
		Bitboard display_over;
		U8 square;
		
		JUST_ENTERED_STATE = 0;
		square = KingSquares[TURN];
					
		if (GameOverInfo == 1) {
			display_over.RANK[0] = 0;
			display_over.RANK[1] = 0;
			display_over.RANK[2] = 0;
			display_over.RANK[3] = 0;
			display_over.RANK[4] = 0;
			display_over.RANK[5] = 0;
			display_over.RANK[6] = 0;
			display_over.RANK[7] = 0;
			display_over.RANK[square >> SHIFT] |= BitMask[square & MASK];
		} else if (GameOverInfo == 0) {
			display_over.RANK[0] = 0xFF;
			display_over.RANK[1] = 0xFF;
			display_over.RANK[2] = 0xFF;
			display_over.RANK[3] = 0xFF;
			display_over.RANK[4] = 0xFF;
			display_over.RANK[5] = 0xFF;
			display_over.RANK[6] = 0xFF;
			display_over.RANK[7] = 0xFF;
			
			display_over.RANK[square >> SHIFT] &= BitMaskClr[square & MASK];
		}
		set_leds(&display_over);
	}
}

void task_error_on() {
	if (JUST_ENTERED_STATE) {
		JUST_ENTERED_STATE = 0;
		set_leds(&OneBoard);  // All LEDs on = ERROR
		ErrorFlashCount++;
	}
		
	if (ui_timer != 0) return;
		
	if (ErrorFlashCount >= 6) {
			ErrorFlashCount = 0;
			
			if (read_and_verify_sensors(SHORT)) {
					MATCH = compare_boards(&CurrentBoard, &PolledBoard);
					
					if (MATCH) {
							// Board corrected!
							CurrentMainState = DETECTING;
							CurrentDetectionState = NONE;
							clear_leds();
							JUST_ENTERED_STATE = 1;
							ui_timer = 100;
							return;
					}
			}
			
			// Still wrong - show expected position
			set_leds(&CurrentBoard);  // Show where pieces SHOULD be
			ui_timer = 500;  // Display for 500ms
			ErrorFlashCount = 0;  // Reset to flash again
			return;
	}
		
	CurrentMainState = ERROR_FLASH_OFF;
	ui_timer = 100;
	JUST_ENTERED_STATE = 1;
}

void task_error_off() {
	if (JUST_ENTERED_STATE) {
			JUST_ENTERED_STATE = 0;
			clear_leds();
	}
				
	if (ui_timer != 0) return;
	CurrentMainState = ERROR_FLASH_ON;
	ui_timer = 100;
	JUST_ENTERED_STATE = 1;
}