#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include "types.h"

void get_legal_moves(U8 sq, Bitboard *legal_board, bit pass);

void get_pawn_moves(U8 sq, Bitboard *board);
void get_king_moves(U8 sq, Bitboard *board);
void get_bishop_moves(U8 sq, Bitboard *board);
void get_rook_moves(U8 sq, Bitboard *board);
void get_knight_moves(U8 sq, Bitboard *board);
bit is_square_attacked(U8 sq, bit attacker_color);

bit is_game_over(void);

void apply_move(U8 FromSquare, U8 ToSquare, bit emit);


#endif