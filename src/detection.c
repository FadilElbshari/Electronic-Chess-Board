#include "detection.h"
#include "types.h"
#include "interrupts.h"
#include "bitboard.h"
#include "helpers.h"
#include "shift_registers.h"
#include "move_gen.h"
#include "tasks.h"

static void go_error(void) {
    CurrentMainState = ERROR_FLASH_ON;
    CurrentDetectionState = NONE;
    LiftedPieceSquare = 0;
    clear_legal_moves();
}

static void finish_move(U8 to_sq) {
    CurrentDetectionState = NONE;
    CurrentMainState = DETECTING;
    clear_legal_moves();

    apply_move(LiftedPieceSquare, to_sq, 1);
    LiftedPieceSquare = 0;

    if (is_game_over()) {
        CurrentMainState = GAME_IS_OVER;
        JUST_ENTERED_STATE = 1;
    }
}

static bit find_first_square(const Bitboard *b, U8 *sq) {
	U8 r;
	for (r = 0; r < 8; r++) {
			U8 x = b->RANK[r];
			if (x) {
					U8 f = 0;
					while ((x & 1u) == 0u) { x >>= 1; f++; }
					*sq = (r << SHIFT) | f;
					return 1;
			}
	}
	return 0;
}

static bit any_intersection(const Bitboard *a, const Bitboard *b) {
    U8 r;
    for (r = 0; r < 8; r++) {
        if (a->RANK[r] & b->RANK[r]) return 1;
    }
    return 0;
}

static bit capture_square_is_legal(void) {
		Bitboard other;
    U8 lr = LiftedPieceSquare >> SHIFT;
    U8 lf = LiftedPieceSquare & MASK;
	
	  other = LeftMask;
    other.RANK[lr] &= BitMaskClr[lf];   // remove lifted square

    return any_intersection(&other, &LegalMoves);
}


void task_detecting_state() {
	if (JUST_ENTERED_STATE) {
			JUST_ENTERED_STATE = 0;
			clear_leds();
			ui_timer = 20;
		}
				
		if (LED_READY) {
			LED_READY = 0;
			set_leds(&DisplayBoardLEDs);
			ui_timer = 20;
		}

		if (ui_timer != 0) return;
		
		if (!read_and_verify_sensors(CurrentDetectionState == LIFT ? LONG : SHORT)) return;
						
		MATCH = compare_boards(&CurrentBoard, &PolledBoard);

		if (!MATCH) {
			JUST_ENTERED_STATE = 1;
			CurrentMainState = CHANGE_DETECTED;
			return;
		}
				
		clear_leds();
		switch (CurrentDetectionState) {
			case NONE:
				return;
						
			case LIFT: {
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
								clear_leds();
						} else {
								// Boards match but piece not on original square - ERROR
								go_error();
						}
				}
				return;
		}
	}
				
	ui_timer = 20;
}


void task_handle_change() {
	
	U8 left_cnt, enter_cnt;
	U8 to_sq;
	
	JUST_ENTERED_STATE = 0;
					
	if (CurrentDetectionState == CAPTURE_INTERMEDIATE) {
			get_left_entered(&IntermediateBoard, &PolledBoard);
	} else {
			get_left_entered(&CurrentBoard, &PolledBoard);
	}
	
	left_cnt = get_bit_count(&LeftMask);
	enter_cnt = get_bit_count(&EnteredMask);

					
	switch (CurrentDetectionState){

		case NONE: {
			
			// Single piece lifted indicating a LIFT
			if (left_cnt == 1 && enter_cnt == 0) {
				
				U8 removed_sq;
        U8 removed_piece;
        bit removed_color;
				
				if (!find_first_square(&LeftMask, &removed_sq)) {
					go_error();
					break;
        }
				
				removed_piece = BoardState[removed_sq];
        if ((removed_piece & TYPE_MASK) == TYPE_EMPTY) {
            go_error();
            break;
        }
				
				removed_color = ((removed_piece & COLOR_WHITE) != 0);
				
				// Normal move start: player lifted THEIR own piece
        if (removed_color == TURN && removed_color == COLOR) {
            CurrentDetectionState = LIFT;
            CurrentMainState = DETECTING;
            JUST_ENTERED_STATE = 1;

            LiftedPieceSquare = removed_sq;
            get_legal_moves(LiftedPieceSquare, &LegalMoves, 0);
            DisplayBoardLEDs = LegalMoves;

            LED_READY = 1;
            break;
        }
				// NEW: captured piece removed first (opponent piece removed)
        if (removed_color != TURN) {
					
            if (!is_square_attacked(removed_sq, TURN)) {
                // No legal capturer exists -> illegal removal
                go_error();
                break;
            }

            LiftedCaptureSquare = removed_sq;

            CurrentDetectionState = CAPTURE_REMOVED_FIRST;
            CurrentMainState = DETECTING;
            JUST_ENTERED_STATE = 1;
            break;
        }
				
				go_error();
				break;
				
			}
			// In case bit is never found go to error - this should never happen - safety
			go_error();
			break;
		}
			
	
		case LIFT: {
			
			// Piece still lifted
			if (left_cnt == 1 && enter_cnt == 0) {
				CurrentMainState = DETECTING; 
				break;
			}
			
			// Capture in progress: removed opponent's piece while holding yours
			if (left_cnt == 2 && enter_cnt == 0) {
				
					if (!capture_square_is_legal()) {
						go_error();
						break;
					}
					
					// Valid capture - wait for piece to be placed
					IntermediateBoard = PolledBoard;
					CurrentDetectionState = CAPTURE_INTERMEDIATE;
					CurrentMainState = DETECTING;
					break;
			}
					
			// Piece placed on new square, check if legal
			if (left_cnt == 1 && enter_cnt == 1) {
				
					if (!any_intersection(&EnteredMask, &LegalMoves)) {
						go_error();
						break;
					}
				
					// Extract to-square (entered_cnt==1 so this is safe)
					if (!find_first_square(&EnteredMask, &to_sq)) {
						go_error();
						break;
					}
				
					finish_move(to_sq);
          break;
				
			}
			go_error();
			break;
		}
		
		case CAPTURE_REMOVED_FIRST: {
			// If everything matches again, user put the captured piece back -> cancel
			if (left_cnt == 0 && enter_cnt == 0) {
					CurrentDetectionState = NONE;
					clear_legal_moves();
					clear_leds();
					LiftedCaptureSquare = 0;
					CurrentMainState = DETECTING;
					break;
			}
			
			// Still only the captured piece removed: keep prompting
			if (left_cnt == 1 && enter_cnt == 0) {
					CurrentMainState = DETECTING;
					break;
			}
			
			if (left_cnt == 2 && enter_cnt == 0) {
				EnteredMask.RANK[LiftedCaptureSquare >> SHIFT] &= BitMaskClr[LiftedCaptureSquare & MASK];
				
				if (!find_first_square(&EnteredMask, &LiftedPieceSquare)) {
					go_error();
					break;
				}
				
				// Verify the lifted piece is a valid capturer
				get_legal_moves(LiftedPieceSquare, &LegalMoves, 0);
				if (!(LegalMoves.RANK[LiftedCaptureSquare >> SHIFT] & BitMask[LiftedCaptureSquare & MASK])) {
					go_error();
					break;
				}
				
				LegalMoves = ZeroBoard;
				LegalMoves.RANK[LiftedCaptureSquare >> SHIFT] |= BitMask[LiftedCaptureSquare & MASK];
				DisplayBoardLEDs = LegalMoves;
        LED_READY = 1;
				
				
				// Same capture pipeline
        IntermediateBoard = PolledBoard;              // both squares empty now
        CurrentDetectionState = CAPTURE_INTERMEDIATE; // existing state
        CurrentMainState = DETECTING;
				
				LiftedCaptureSquare = 0;
				break;
				
			}
			
			go_error();
			break;
		}
		
			
		case CAPTURE_INTERMEDIATE: {
		
			// Waiting (both pieces currently off-board)
			if (left_cnt == 0 && enter_cnt == 0) {
					CurrentMainState = DETECTING;
					break;
			}
			
			// Waiting for capturing piece to be placed on new square
			if (left_cnt == 0 && enter_cnt == 1) {
				if (!any_intersection(&EnteredMask, &LegalMoves)) {
							go_error();
							break;
					}

					if (!find_first_square(&EnteredMask, &to_sq)) {
							go_error();
							break;
					}

					finish_move(to_sq);
					break;
			}
			
			// Unexpected state
			go_error();
			break;
		}
			
		default:
			// Should never reach here, but recover gracefully
			go_error();
			break;
		}
}