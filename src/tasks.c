#include "tasks.h"
#include "types.h"
#include "interrupts.h"
#include "bitboard.h"
#include "helpers.h"
#include "shift_registers.h"
#include "move_gen.h"
#include "tm_ssd.h"

#define TIME_BETWEEN_READS 5
#define FLASHING_RATE 50

FLAG JUST_ENTERED_STATE;
FLAG IN_ERROR;
U8 CurrentMainState;
U8 CurrentDetectionState;
U8 ErrorFlashCount;

#ifdef ONLINE
void reset_game() {
	JUST_ENTERED_STATE = 1;
	CurrentMainState = TURNED_ON;
	CurrentDetectionState = NONE;
	
	IN_ERROR = 0;

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

#else
void reset_game() {
	U8 i, j;
	
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
			
	TURN = WHITE;
	COLOR = WHITE;
}

#endif


void task_handle_flags() {
	
	if (RX_PACKET_READY) process_rx_packet();
	if (TX_PACKET_READY) process_tx_packet();
	
	if (IS_RESET) {
		IS_RESET = 0;
		reset_game();
	}
	
	if (MOVE_RECEIVED && !IN_ERROR) {
		
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
#ifndef ONLINE
		JUST_ENTERED_STATE = 1;
		CurrentMainState = AWAIT_INITIAL_POSITION_SET;
#endif
	}
		
	if (CONNECTED) {
		JUST_ENTERED_STATE = 1;
		ui_timer = 50;
		CurrentMainState = AWAIT_INITIAL_POSITION_SET;
	}
}

void task_await_initpos() {
	U8 i;
	tm_display_digits(0, 2, 0, 1);
	if (JUST_ENTERED_STATE){
		if (LED_READY) {
			JUST_ENTERED_STATE = 0;
			LED_READY = 0;
			CurrentBoard = DisplayBoardLEDs;
			ui_timer = TIME_BETWEEN_READS;
			return;
		}
		return;
	}
	
	if (ui_timer != 0) return;

	if (!read_and_verify_sensors(SHORT)) {
		ui_timer = TIME_BETWEEN_READS;
		return;
	}
	
	MATCH = compare_boards(&CurrentBoard, &PolledBoard);
	
	for (i=0; i<BOARD_W; i++) DisplayBoardLEDs.RANK[i] = CurrentBoard.RANK[i] & ~PolledBoard.RANK[i];
	
	set_leds(&DisplayBoardLEDs);	
	
	if (!MATCH) {
		ui_timer = TIME_BETWEEN_READS;
		return;
	}
	
	JUST_ENTERED_STATE = 1;
	LED_READY = 0;
	CurrentMainState = DETECTING;
}

void task_await_moveset() {
	
	if (JUST_ENTERED_STATE) {
		JUST_ENTERED_STATE = 0;
		set_leds(&DisplayBoardLEDs);
		ui_timer = TIME_BETWEEN_READS;
		return;
	}

	if (ui_timer != 0) return;
	

	// Try to read sensors
	if (!read_and_verify_sensors(SHORT)) {
			ui_timer = TIME_BETWEEN_READS;
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
	ui_timer = TIME_BETWEEN_READS;
	
}


void task_gameover() {
	if (JUST_ENTERED_STATE) {
		U8 square, i;
		
		JUST_ENTERED_STATE = 0;
		square = KingSquares[TURN];
					
		if (GameOverInfo == 1) {
			for (i = 0; i<BOARD_W; i++) DisplayBoardLEDs.RANK[i] = 0;
			DisplayBoardLEDs.RANK[square >> SHIFT] |= BitMask[square & MASK];
			
		} else if (GameOverInfo == 0) {
			for (i = 0; i<BOARD_W; i++) DisplayBoardLEDs.RANK[i] = 0;
			DisplayBoardLEDs.RANK[square >> SHIFT] &= BitMaskClr[square & MASK];
			
		}
		set_leds(&DisplayBoardLEDs);
	}
}

void task_error_on() {
	U8 i;
	if (JUST_ENTERED_STATE) {
		JUST_ENTERED_STATE = 0;
		set_leds(&OneBoard);  // All LEDs on = ERROR
		ErrorFlashCount++;
	}
		
	if (ui_timer != 0) return;
		
	if (ErrorFlashCount >= 4) {
			ErrorFlashCount = 0;
			
			if (read_and_verify_sensors(SHORT)) {
					MATCH = compare_boards(&CurrentBoard, &PolledBoard);
					
					if (MATCH) {
							// Board corrected!
							CurrentMainState = DETECTING;
							CurrentDetectionState = NONE;
							IN_ERROR = 0; 
						
							clear_leds();
							JUST_ENTERED_STATE = 1;
							ui_timer = FLASHING_RATE;
							return;
					}
			}
			
			// Still wrong - show expected position
				for (i=0; i<BOARD_W; i++) DisplayBoardLEDs.RANK[i] = CurrentBoard.RANK[i] & ~PolledBoard.RANK[i];
				set_leds(&DisplayBoardLEDs);	
			
			ui_timer = 500;  // Display for 500ms
			ErrorFlashCount = 0;  // Reset to flash again
			return;
	}
		
	CurrentMainState = ERROR_FLASH_OFF;
	ui_timer = FLASHING_RATE;
	JUST_ENTERED_STATE = 1;
}

void task_error_off() {
	if (JUST_ENTERED_STATE) {
			JUST_ENTERED_STATE = 0;
			clear_leds();
	}
				
	if (ui_timer != 0) return;
	CurrentMainState = ERROR_FLASH_ON;
	ui_timer = FLASHING_RATE;
	JUST_ENTERED_STATE = 1;
}