#include "move_gen.h"

#include "types.h"
#include "bitboard.h"

// King directions
const signed char dr[8] = {-1,-1,-1,0,0,1,1,1};
const signed char df[8] = {-1,0,1,-1,1,-1,0,1};

// Rook directions
const signed char dr_rook[4] = { -1,  1,  0,  0 };
const signed char df_rook[4] = {  0,  0, -1,  1 };

// Bishop directions
const signed char dr_bishop[4] = { -1, -1,  1,  1 };
const signed char df_bishop[4] = { -1,  1, -1,  1 };

// Knight directions
const signed char dr_knight[8] = { -2, -2, -1, -1,  1,  1,  2,  2 };
const signed char df_knight[8] = { -1,  1, -2,  2, -2,  2, -1,  1 };


		
Bitboard get_legal_moves(U8 sq) {

    U8 r, f, to_sq;
		bit piece_turn;
    U8 from_piece, captured_piece;
    U8 old_king_sq;

    Bitboard pseudo_board;
    Bitboard legal_board;

    pseudo_board = ZeroBoard;
    legal_board  = ZeroBoard;

    from_piece = BoardState[sq];
    old_king_sq = KingSquares[TURN];
	
		if ((from_piece & TYPE_MASK) == TYPE_EMPTY) return legal_board;

		piece_turn = (from_piece & COLOR_WHITE) != 0;

		if ((piece_turn != TURN) || (piece_turn != COLOR)) return legal_board;

    /* --- Generate pseudo-legal moves --- */
    switch (from_piece & TYPE_MASK) {
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

            if (!(pseudo_board.RANK[r] & (1 << f)))
                continue;

            to_sq = (r << 2) | f;

            /* make temporary move */
            captured_piece = BoardState[to_sq];
            BoardState[to_sq] = from_piece;
            BoardState[sq] = TYPE_EMPTY;

            if ((from_piece & TYPE_MASK) == TYPE_KING)
                KingSquares[TURN] = to_sq;

            /* check king safety */
            if (!is_square_attacked(KingSquares[TURN], !TURN)) {
                legal_board.RANK[r] |= (1 << f);
            }

            /* undo move */
            BoardState[sq] = from_piece;
            BoardState[to_sq] = captured_piece;
            KingSquares[TURN] = old_king_sq;
        }
    }

    return legal_board;
}


void get_king_moves(U8 sq, Bitboard *board) {
	U8 i, piece, color, rank, file, new_sq, target;
	signed char r, f;
	
		piece = BoardState[sq];
    color = piece & COLOR_WHITE;
    rank = sq >> 2;
    file = sq & 3;

    // King direction offsets


    for (i=0; i<8; i++) {
        r = rank + dr[i];
        f = file + df[i];

        if (r < 0 || r >= BOARD_W || f < 0 || f >= BOARD_W)
            continue;

        new_sq = r * BOARD_W + f;
        target = BoardState[new_sq];

        if ((target & TYPE_MASK) == TYPE_EMPTY || (target & COLOR_WHITE) != color) {
            board->RANK[r] |= (1 << f);
        }
    }
}

void get_rook_moves(U8 sq, Bitboard *board) {
    U8 i, piece, color, target;
    signed char r, f;
    U8 rank, file;

    piece = BoardState[sq];
    color = piece & COLOR_WHITE;
    rank = sq >> 2;
    file = sq & 3;

    for (i = 0; i < 4; i++) {
        r = rank;
        f = file;

        while (1) {
            r += dr_rook[i];
            f += df_rook[i];

            if (r < 0 || r >= BOARD_W || f < 0 || f >= BOARD_W)
                break;

            target = BoardState[r * BOARD_W + f];

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
    rank = sq >> 2;
    file = sq & 3;

    for (i = 0; i < 4; i++) {
        r = rank;
        f = file;

        while (1) {
            r += dr_bishop[i];
            f += df_bishop[i];

            if (r < 0 || r >= BOARD_W || f < 0 || f >= BOARD_W)
                break;

            target = BoardState[r * BOARD_W + f];

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
    rank = sq >> 2;
    file = sq & 3;

    for (i = 0; i < 8; i++) {
        r = rank + dr_knight[i];
        f = file + df_knight[i];

        if (r < 0 || r >= BOARD_W || f < 0 || f >= BOARD_W)
            continue;

        target = BoardState[r * BOARD_W + f];

        if ((target & TYPE_MASK) == TYPE_EMPTY || (target & COLOR_WHITE) != color) {
            board->RANK[r] |= (1 << f);
        }
    }
}

bit is_square_attacked(U8 sq, bit attacker_color)
{
    U8 r = sq >> 2;
    U8 f = sq & 3;
    signed char nr, nf;
    U8 i, nsq, piece, type;

    /* ---------- Knight attacks ---------- */
    for (i = 0; i < 8; i++) {
        nr = r + dr_knight[i];
        nf = f + df_knight[i];

        if (nr < 0 || nr >= BOARD_W || nf < 0 || nf >= BOARD_W)
            continue;

        nsq = (nr << 2) | nf;
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

            nsq = (nr << 2) | nf;
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

            nsq = (nr << 2) | nf;
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

        nsq = (nr << 2) | nf;
        piece = BoardState[nsq];

        if ((piece & TYPE_MASK) == TYPE_KING && ((piece & COLOR_WHITE) != 0) == attacker_color) return 1;
    }

    return 0;
}


