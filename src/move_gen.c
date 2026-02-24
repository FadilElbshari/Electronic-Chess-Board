#include "move_gen.h"

#include "types.h"
#include "bitboard.h"
#include "interrupts.h"
#include "helpers.h"

// King directions
static const signed char code dr[8] = {-1,-1,-1,0,0,1,1,1};
static const signed char code df[8] = {-1,0,1,-1,1,-1,0,1};

// Rook directions
static const signed char code dr_rook[4] = { -1,  1,  0,  0 };
static const signed char code df_rook[4] = {  0,  0, -1,  1 };

// Bishop directions
static const signed char code dr_bishop[4] = { -1, -1,  1,  1 };
static const signed char code df_bishop[4] = { -1,  1, -1,  1 };

// Knight directions
static const signed char code dr_knight[8] = { -2, -2, -1, -1,  1,  1,  2,  2 };
static const signed char code df_knight[8] = { -1,  1, -2,  2, -2,  2, -1,  1 };


#define WHITE_PAWN_START_RANK 1
#define BLACK_PAWN_START_RANK 6


void apply_move(U8 FromSquare, U8 ToSquare, bit emit) {
	U8 FromRank, FromFile, r;
	U8 ToRank, ToFile;
	
	bit whichCastlingSide, isCastling;
	whichCastlingSide = 0;
	isCastling = 0;
	

	FromRank = FromSquare >> SHIFT;
	FromFile = FromSquare & MASK;
	
	ToRank = ToSquare >> SHIFT;
	ToFile = ToSquare & MASK;
	
	if ((BoardState[(FromRank << SHIFT) | FromFile] & TYPE_MASK) == TYPE_KING) {
		// Check if the move is short castle
		if (!KingMoved[TURN] && ((FromFile==4 && ToFile==6) || (FromFile==4 && ToFile==2))) {
			isCastling = 1;
			if (ToFile == 6) {
				whichCastlingSide = 0; // SHORT
			} else if (ToFile == 2) { 
				whichCastlingSide = 1; // LONG
			}
			
			MoveSquares[0] = FromRank;
			MoveSquares[1] = whichCastlingSide == 0 ? 7 : 0;
			MoveSquares[2] = ToRank;
			MoveSquares[3] = whichCastlingSide == 0 ? 5 : 3;
			
			for (r = 0; r < 8; r++) DisplayBoardLEDs.RANK[r] = 0;
			
			DisplayBoardLEDs.RANK[MoveSquares[0]] |= BitMask[MoveSquares[1]];
			DisplayBoardLEDs.RANK[MoveSquares[2]] |= BitMask[MoveSquares[3]];
		
			
			MOVE_RECEIVED = 1;
		}
			
		KingSquares[TURN] = (ToRank << SHIFT) | ToFile;
		KingMoved[TURN] = 1;
	}
	
	if ((BoardState[FromSquare] & TYPE_MASK) == TYPE_ROOK) {
		RookMoved[TURN][FromFile > 4 ? SHORT_ROOK : LONG_ROOK] = 1;
	}
	
	BoardState[ToSquare] = BoardState[(FromRank << SHIFT) | FromFile];
	BoardState[FromSquare] = EMPTY;
	
	CurrentBoard.RANK[FromRank] &= BitMaskClr[FromFile];
	CurrentBoard.RANK[ToRank] |= BitMask[ToFile];
	
	// Toggle turn
	if (!isCastling) TURN = !TURN;
	
#ifdef ONLINE
	if (!emit) return;
	
	txHeader = HEADER;
	txType = MOVE_PACKET;
	txLen = 0x02;
	
	txBuffer[0] = LiftedPieceSquare;
	txBuffer[1] = ToSquare;
	
	TX_PACKET_READY = 1;
#endif
}

		
void get_legal_moves(U8 sq, Bitboard *legal_board, bit pass) {

    U8 r, f, to_sq;
		bit piece_turn;
    U8 from_piece, captured_piece;
    U8 old_king_sq;

    board_copy(legal_board, &ZeroBoard);

    from_piece = BoardState[sq];
    old_king_sq = KingSquares[TURN];
	
		if ((from_piece & TYPE_MASK) == TYPE_EMPTY) return;

		piece_turn = (from_piece & COLOR_WHITE) != 0;

		if (piece_turn != TURN) return;
		if (!pass) if (piece_turn != COLOR) return;

		// Psuedo legal move generation
    switch (from_piece & TYPE_MASK) {
				case TYPE_PAWN:
						get_pawn_moves(sq, legal_board);
            break;
        case TYPE_KNIGHT:
            get_knight_moves(sq, legal_board);
            break;
        case TYPE_BISHOP:
            get_bishop_moves(sq, legal_board);
            break;
        case TYPE_ROOK:
            get_rook_moves(sq, legal_board);
            break;
        case TYPE_QUEEN:
            get_bishop_moves(sq, legal_board);
            get_rook_moves(sq, legal_board);
            break;
        case TYPE_KING:
            get_king_moves(sq, legal_board);
            break;
        default:
            break;
    }

    // Filter to legal moves only
    for (r = 0; r < BOARD_W; r++) {
        for (f = 0; f < BOARD_W; f++) {

            if (!(legal_board->RANK[r] & BitMask[f])) continue;

            to_sq = (r << SHIFT) | f;

						// Temporarily make the psuedo legal move
            captured_piece = BoardState[to_sq];
            BoardState[to_sq] = from_piece;
            BoardState[sq] = TYPE_EMPTY;

            if ((from_piece & TYPE_MASK) == TYPE_KING)
                KingSquares[TURN] = to_sq;

						// Verify whether the king is in check or not - if yes => illegal move, no => legal move
            if (is_square_attacked(KingSquares[TURN], !TURN)) {
                legal_board->RANK[r] &= BitMaskClr[f];
            }

						// Undo move
            BoardState[sq] = from_piece;
            BoardState[to_sq] = captured_piece;
            KingSquares[TURN] = old_king_sq;
        }
    }
		
		if ((from_piece & TYPE_MASK) == TYPE_KING) {
			// Check if castling is one of legal moves
			r = sq >> SHIFT;
			f = sq & MASK;
			
			// If king in check - remove castling moves directly
			if (is_square_attacked(KingSquares[TURN], !TURN)) {
				legal_board->RANK[r] &= 0xBB;
				return;
			}
			
			// Check short castle
			if (legal_board->RANK[r] & BitMask[f+2]) {
				if (is_square_attacked((r << SHIFT) | (f+1), !TURN)) {
					legal_board->RANK[r] &= 0xBF;
				}
			}
			
			// Check long castle
			if (legal_board->RANK[r] & BitMask[f-2]) {
				if (is_square_attacked((r << SHIFT) | (f-1), !TURN)) {
					legal_board->RANK[r] &= 0xFB;
				}
			}
			
		}

}

void get_pawn_moves(U8 sq, Bitboard *board) { 
		U8 color, rank, file, new_sq, new_rank, target, start_rank;
		signed char dir, step;
		
		rank = sq >> SHIFT;
    file = sq & MASK;
	
		color = BoardState[sq] & COLOR_WHITE;
	
		if (color & COLOR_WHITE) {
			dir = 1;
			start_rank = WHITE_PAWN_START_RANK;
			step = 8;
		} else {
			dir = -1;
			start_rank = BLACK_PAWN_START_RANK;
			step = -8;
		}
	 
		new_rank = rank + dir;
		
		if (new_rank < BOARD_W) {
			new_sq = sq + step;
			
			if ((BoardState[new_sq] & TYPE_MASK) == TYPE_EMPTY) { // VALID SINGLE PUSH
				board->RANK[new_rank] |= BitMask[file];
				new_sq += step; 
				
				if (rank == start_rank && (BoardState[new_sq] & TYPE_MASK) == TYPE_EMPTY) board->RANK[new_rank + dir] |= BitMask[file]; // DOUBLE PUSH
			}
		}
		 
	// CAPTURES
	if (file > 0 && new_rank < BOARD_W) {
		new_sq = sq + step - 1; // LEFT CAPTURE - WHTIE
		target = BoardState[new_sq];
		if ((target & TYPE_MASK) != TYPE_EMPTY && (target & COLOR_WHITE) != color) board->RANK[new_rank] |= BitMask[file-1]; 
	}
		
	if (file < BOARD_W - 1 && new_rank < BOARD_W) {
		new_sq = sq + step + 1; // RIGHT CAPTURE - WHTIE
		target = BoardState[new_sq];
		if ((target & TYPE_MASK) != TYPE_EMPTY && (target & COLOR_WHITE) != color) board->RANK[new_rank] |= BitMask[file+1]; 
	}
	
}

void get_king_moves(U8 sq, Bitboard *board) {
	U8 i, piece, color, rank, file, new_sq, target, side;
	signed char r, f;
	
		piece = BoardState[sq];
    color = piece & COLOR_WHITE;
    rank = sq >> SHIFT;
    file = sq & MASK;
	
		side = (color != 0);

    // King direction offsets
    for (i=0; i<8; i++) {
        r = rank + dr[i];
        f = file + df[i];

        if ((U8)r >= BOARD_W || (U8)f >= BOARD_W) continue;

        new_sq = (r << SHIFT) | f;
        target = BoardState[new_sq];

        if ((target & TYPE_MASK) == TYPE_EMPTY || (target & COLOR_WHITE) != color) {
            board->RANK[r] |= BitMask[f];
        }
    }
		
		if (KingMoved[side]) return;
		
		// Short castling check
		if (!RookMoved[side][SHORT_ROOK]) {
			if (((BoardState[(rank << SHIFT) | (file+1)] & TYPE_MASK) == TYPE_EMPTY) && ((BoardState[(rank << SHIFT) | (file+2)] & TYPE_MASK) == TYPE_EMPTY)) {
				board->RANK[rank] |= BitMask[file+2];
			}
		}
		
		// Long castling check
		if (!RookMoved[side][LONG_ROOK]) {
			if (((BoardState[(rank << SHIFT) | (file-1)] & TYPE_MASK) == TYPE_EMPTY) && ((BoardState[(rank << SHIFT) | (file-2)] & TYPE_MASK) == TYPE_EMPTY) && ((BoardState[(rank << SHIFT) | (file-3)] & TYPE_MASK) == TYPE_EMPTY)) {
				board->RANK[rank] |= BitMask[file-2];
			}
		}
}

void get_rook_moves(U8 sq, Bitboard *board) {
    U8 i, piece, color, target;
    signed char r, f;
    U8 rank, file;

    piece = BoardState[sq];
    color = piece & COLOR_WHITE;
    rank = sq >> SHIFT;
    file = sq & MASK;

    for (i = 0; i < 4; i++) {
        r = rank;
        f = file;

        while (1) {
            r += dr_rook[i];
            f += df_rook[i];

            if ((U8)r >= BOARD_W || (U8)f >= BOARD_W) break;

            target = BoardState[(r << SHIFT) | f];

            if ((target & TYPE_MASK) == TYPE_EMPTY) {
                board->RANK[r] |= BitMask[f];
            } else {
                if ((target & COLOR_WHITE) != color)
                    board->RANK[r] |= BitMask[f];
                break;   // blocked
            }
        }
    }
}

void get_bishop_moves(U8 sq, Bitboard *board) {
    U8 i, piece, color, target;
    signed char r, f;
    U8 rank, file;

    piece = BoardState[sq];
    color = piece & COLOR_WHITE;
    rank = sq >> SHIFT;
    file = sq & MASK;

    for (i = 0; i < 4; i++) {
        r = rank;
        f = file;

        while (1) {
            r += dr_bishop[i];
            f += df_bishop[i];

            if ((U8)r >= BOARD_W || (U8)f >= BOARD_W) break;

            target = BoardState[(r << SHIFT) | f];

            if ((target & TYPE_MASK) == TYPE_EMPTY) {
                board->RANK[r] |= BitMask[f];
            } else {
                if ((target & COLOR_WHITE) != color)
                    board->RANK[r] |= BitMask[f];
                break;   // blocked
            }
        }
    }
}

void get_knight_moves(U8 sq, Bitboard *board) {
    U8 i, piece, color, target;
    signed char r, f;
    U8 rank, file;

    piece = BoardState[sq];
    color = piece & COLOR_WHITE;
    rank = sq >> SHIFT;
    file = sq & MASK;

    for (i = 0; i < 8; i++) {
        r = rank + dr_knight[i];
        f = file + df_knight[i];

        if ((U8)r >= BOARD_W || (U8)f >= BOARD_W) continue;

        target = BoardState[(r << SHIFT) | f];

        if ((target & TYPE_MASK) == TYPE_EMPTY || (target & COLOR_WHITE) != color) {
            board->RANK[r] |= BitMask[f];
        }
    }
}

bit is_square_attacked(U8 sq, bit attacker_color) {
    U8 r = sq >> SHIFT;
    U8 f = sq & MASK;
    signed char nr, nf, pr;
    U8 i, nsq, piece, type;
	
		/* ---------- Pawn attacks ------------ */
		pr = attacker_color ? -1 : 1;

		if (r + pr >= 0 && r + pr < BOARD_W) {
				if (f > 0) {
						piece = BoardState[((r + pr) << SHIFT) | (f - 1)];
						if ((piece & TYPE_MASK) == TYPE_PAWN &&
								((piece & COLOR_WHITE) != 0) == attacker_color)
								return 1;
				}
				if (f < BOARD_W - 1) {
						piece = BoardState[((r + pr) << SHIFT) | (f + 1)];
						if ((piece & TYPE_MASK) == TYPE_PAWN &&
								((piece & COLOR_WHITE) != 0) == attacker_color)
								return 1;
				}
		}


    /* ---------- Knight attacks ---------- */
    for (i = 0; i < 8; i++) {
        nr = r + dr_knight[i];
        nf = f + df_knight[i];

        if (nr < 0 || nr >= BOARD_W || nf < 0 || nf >= BOARD_W)
            continue;

        nsq = (nr << SHIFT) | nf;
        piece = BoardState[nsq];

        if ((piece & TYPE_MASK) == TYPE_KNIGHT && ((piece & COLOR_WHITE) != 0) == attacker_color) return 1;
    }

    /* ---------- Rook / Queen attacks ---------- */
    for (i = 0; i < 4; i++) {
        nr = r;
        nf = f;

        while (1) {
            nr += dr_rook[i];
            nf += df_rook[i];

            if (nr < 0 || nr >= BOARD_W || nf < 0 || nf >= BOARD_W)
                break;

            nsq = (nr << SHIFT) | nf;
            piece = BoardState[nsq];

            if ((piece & TYPE_MASK) == TYPE_EMPTY)
                continue;

            if (((piece & COLOR_WHITE) != 0) == attacker_color) {
                type = piece & TYPE_MASK;
                if (type == TYPE_ROOK || type == TYPE_QUEEN)
                    return 1;
            }
            break;  // blocked
        }
    }

    /* ---------- Bishop / Queen attacks ---------- */
    for (i = 0; i < 4; i++) {
        nr = r;
        nf = f;

        while (1) {
            nr += dr_bishop[i];
            nf += df_bishop[i];

            if (nr < 0 || nr >= BOARD_W || nf < 0 || nf >= BOARD_W)
                break;

            nsq = (nr << SHIFT) | nf;
            piece = BoardState[nsq];

            if ((piece & TYPE_MASK) == TYPE_EMPTY)
                continue;

            if (((piece & COLOR_WHITE) != 0) == attacker_color) {
                type = piece & TYPE_MASK;
                if (type == TYPE_BISHOP || type == TYPE_QUEEN)
                    return 1;
            }
            break;
        }
    }

    /* ---------- King attacks (adjacent squares) ---------- */
    for (i = 0; i < 8; i++) {
        nr = r + dr[i];
        nf = f + df[i];

        if (nr < 0 || nr >= BOARD_W || nf < 0 || nf >= BOARD_W)
            continue;

        nsq = (nr << SHIFT) | nf;
        piece = BoardState[nsq];

        if ((piece & TYPE_MASK) == TYPE_KING && ((piece & COLOR_WHITE) != 0) == attacker_color) return 1;
    }

    return 0;
}

bit is_game_over(void)
{
    U8 sq, r;

    // Check if king has any legal moves
    get_legal_moves(KingSquares[TURN], &LegalMoves, 1);
    for (r = 0; r < BOARD_W; r++) if (LegalMoves.RANK[r]) return 0;

    // Check legal moves for all other pieces (same color)
    for (sq = 0; sq < BOARD_W*BOARD_W; sq++) {
        if (BoardState[sq] == EMPTY) continue;
        if (((BoardState[sq] & COLOR_WHITE) != 0) != TURN) continue;
        if ((BoardState[sq] & TYPE_MASK) == TYPE_KING) continue;

        get_legal_moves(sq, &LegalMoves, 1);
        for (r = 0; r < BOARD_W; r++) if (LegalMoves.RANK[r]) return 0;
    }
		
		GameOverInfo = 0;
		if (is_square_attacked(KingSquares[TURN], !TURN)) GameOverInfo = 1;

    return 1;
}


