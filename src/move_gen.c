#include "move_gen.h"

#include "types.h"
#include "bitboard.h"
#include "interrupts.h"
#include "types.h"

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


U8 GAME_OVER_INFO = 0;


void apply_move(U8 FromSquare, U8 ToSquare, bit emit) {
	U8 FromRank, FromFile;
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
			
			DisplayBoardLEDs.RANK[0] = 0;
			DisplayBoardLEDs.RANK[1] = 0;
			DisplayBoardLEDs.RANK[2] = 0;
			DisplayBoardLEDs.RANK[3] = 0;
			DisplayBoardLEDs.RANK[4] = 0;
			DisplayBoardLEDs.RANK[5] = 0;
			DisplayBoardLEDs.RANK[6] = 0;
			DisplayBoardLEDs.RANK[7] = 0;
			
			DisplayBoardLEDs.RANK[MoveSquares[0]] |= 1 << MoveSquares[1];
			DisplayBoardLEDs.RANK[MoveSquares[2]] |= 1 << MoveSquares[3];
		
			
			MOVE_RECEIVED = 1;
		}
			
		KingSquares[TURN] = (ToRank << SHIFT) | ToFile;
		KingMoved[TURN] = 1;
	}
	
	if ((BoardState[(FromRank << SHIFT) | FromFile] & TYPE_MASK) == TYPE_ROOK) {
		RookMoved[TURN][FromFile > 4 ? LONG_ROOK : SHORT_ROOK] = 1;
	}
	
	BoardState[(ToRank << SHIFT) | ToFile] = BoardState[(FromRank << SHIFT) | FromFile];
	BoardState[(FromRank << SHIFT) | FromFile] = EMPTY;
	
	CurrentBoard.RANK[FromRank] &= ~(1 << FromFile);
	CurrentBoard.RANK[ToRank] |= 1 << ToFile;
	
	// Toggle turn
	if (!isCastling) TURN = !TURN;
	if (isCastling) {
			MoveBoard = CurrentBoard;
		
			MoveBoard.RANK[MoveSquares[0]] &= ~(1 << MoveSquares[1]);
			MoveBoard.RANK[MoveSquares[2]] |= 1 << MoveSquares[3];
	}
	
	if (!emit) return;
	
	txHeader = HEADER;
	txType = MOVE_PACKET;
	txLen = 0x02;
	
	txBuffer[0] = LiftedPieceSquare;
	txBuffer[1] = ToSquare;
	
	txPacketReady = 1;
}

		
void get_legal_moves(U8 sq, Bitboard *legal_board, bit pass) {

    U8 r, f, to_sq;
		bit piece_turn;
    U8 from_piece, captured_piece;
    U8 old_king_sq;

    Bitboard pseudo_board;

		pseudo_board.RANK[0] = 0;
		pseudo_board.RANK[1] = 0;
		pseudo_board.RANK[2] = 0;
		pseudo_board.RANK[3] = 0;
		pseudo_board.RANK[4] = 0;
		pseudo_board.RANK[5] = 0;
		pseudo_board.RANK[6] = 0;
		pseudo_board.RANK[7] = 0;

    *legal_board  = ZeroBoard;

    from_piece = BoardState[sq];
    old_king_sq = KingSquares[TURN];
	
		if ((from_piece & TYPE_MASK) == TYPE_EMPTY) return;

		piece_turn = (from_piece & COLOR_WHITE) != 0;

		if (piece_turn != TURN) return;
		if (!pass) if (piece_turn != COLOR) return;

    /* --- Generate pseudo-legal moves --- */
    switch (from_piece & TYPE_MASK) {
				case TYPE_PAWN:
						get_pawn_moves(sq, &pseudo_board);
            break;
        case TYPE_KNIGHT:
            get_knight_moves(sq, &pseudo_board);
            break;
        case TYPE_BISHOP:
            get_bishop_moves(sq, &pseudo_board);
            break;
        case TYPE_ROOK:
            get_rook_moves(sq, &pseudo_board);
            break;
        case TYPE_QUEEN:
            get_bishop_moves(sq, &pseudo_board);
            get_rook_moves(sq, &pseudo_board);
            break;
        case TYPE_KING:
            get_king_moves(sq, &pseudo_board);
            break;
        default:
            break;
    }

    /* --- Filter to legal moves --- */
    for (r = 0; r < BOARD_W; r++) {
        for (f = 0; f < BOARD_W; f++) {

            if (!(pseudo_board.RANK[r] & (1 << f))) continue;

            to_sq = (r << SHIFT) | f;

            /* make temporary move */
            captured_piece = BoardState[to_sq];
            BoardState[to_sq] = from_piece;
            BoardState[sq] = TYPE_EMPTY;

            if ((from_piece & TYPE_MASK) == TYPE_KING)
                KingSquares[TURN] = to_sq;

            /* check king safety */
            if (!is_square_attacked(KingSquares[TURN], !TURN)) {
                legal_board->RANK[r] |= (1 << f);
            }

            /* undo move */
            BoardState[sq] = from_piece;
            BoardState[to_sq] = captured_piece;
            KingSquares[TURN] = old_king_sq;
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
		} else {
			dir = -1;
			start_rank = BLACK_PAWN_START_RANK;
		}
	 
		step = dir << SHIFT;
		new_rank = rank + dir;
		
		if (new_rank < BOARD_W && new_rank >=0) {
			new_sq = sq + step;
			
			if ((BoardState[new_sq] & TYPE_MASK) == TYPE_EMPTY) { // VALID SINGLE PUSH
				board->RANK[new_rank] |= (1 << file);
				new_sq += step; 
				
				if (rank == start_rank && (BoardState[new_sq] & TYPE_MASK) == TYPE_EMPTY) board->RANK[new_rank + dir] |= (1 << file); // DOUBLE PUSH
			}
		}
		 
	// CAPTURES
	if (file > 0 && new_rank >= 0 && new_rank < BOARD_W) {
		new_sq = sq + step - 1; // LEFT CAPTURE - WHTIE
		target = BoardState[new_sq];
		if ((target & TYPE_MASK) != TYPE_EMPTY && (target & COLOR_WHITE) != color) board->RANK[new_rank] |= (1 << (file-1)); 
	}
		
	if (file < BOARD_W - 1 && new_rank >= 0 && new_rank < BOARD_W) {
		new_sq = sq + step + 1; // RIGHT CAPTURE - WHTIE
		target = BoardState[new_sq];
		if ((target & TYPE_MASK) != TYPE_EMPTY && (target & COLOR_WHITE) != color) board->RANK[new_rank] |= (1 << (file+1)); 
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

        if (r < 0 || r >= BOARD_W || f < 0 || f >= BOARD_W)
            continue;

        new_sq = (r << SHIFT) + f;
        target = BoardState[new_sq];

        if ((target & TYPE_MASK) == TYPE_EMPTY || (target & COLOR_WHITE) != color) {
            board->RANK[r] |= (1 << f);
        }
    }
		
		if (KingMoved[side]) return;
		
		// Short castling check
		if (!RookMoved[side][SHORT_ROOK]) {
			if (((BoardState[(rank << SHIFT) | (file+1)] & TYPE_MASK) == TYPE_EMPTY) && ((BoardState[(rank << SHIFT) | (file+2)] & TYPE_MASK) == TYPE_EMPTY)) {
				board->RANK[rank] |= (1 << (file+2));
			}
		}
		
		// Long castling check
		if (!RookMoved[side][LONG_ROOK]) {
			if (((BoardState[(rank << SHIFT) | (file-1)] & TYPE_MASK) == TYPE_EMPTY) && ((BoardState[(rank << SHIFT) | (file-2)] & TYPE_MASK) == TYPE_EMPTY) && ((BoardState[(rank << SHIFT) | (file-3)] & TYPE_MASK) == TYPE_EMPTY)) {
				board->RANK[rank] |= (1 << (file-2));
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

            if (r < 0 || r >= BOARD_W || f < 0 || f >= BOARD_W)
                break;

            target = BoardState[(r << SHIFT) + f];

            if ((target & TYPE_MASK) == TYPE_EMPTY) {
                board->RANK[r] |= (1 << f);
            } else {
                if ((target & COLOR_WHITE) != color)
                    board->RANK[r] |= (1 << f);
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

            if (r < 0 || r >= BOARD_W || f < 0 || f >= BOARD_W)
                break;

            target = BoardState[(r << SHIFT) + f];

            if ((target & TYPE_MASK) == TYPE_EMPTY) {
                board->RANK[r] |= (1 << f);
            } else {
                if ((target & COLOR_WHITE) != color)
                    board->RANK[r] |= (1 << f);
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

        if (r < 0 || r >= BOARD_W || f < 0 || f >= BOARD_W)
            continue;

        target = BoardState[(r << SHIFT) + f];

        if ((target & TYPE_MASK) == TYPE_EMPTY || (target & COLOR_WHITE) != color) {
            board->RANK[r] |= (1 << f);
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
    Bitboard moves;

    // 1) King first
    get_legal_moves(KingSquares[TURN], &moves, 1);
    for (r = 0; r < BOARD_W; r++) if (moves.RANK[r]) return 0;

    // 2) All other pieces
    for (sq = 0; sq < BOARD_W*BOARD_W; sq++) {
        if (BoardState[sq] == EMPTY) continue;
        if (((BoardState[sq] & COLOR_WHITE) != 0) != TURN) continue;
        if ((BoardState[sq] & TYPE_MASK) == TYPE_KING) continue;

        get_legal_moves(sq, &moves, 1);
        for (r = 0; r < BOARD_W; r++) if (moves.RANK[r]) return 0;
    }
		
		GAME_OVER_INFO = 0;
		if (is_square_attacked(KingSquares[TURN], !TURN)) GAME_OVER_INFO = 1;

    return 1;
}


