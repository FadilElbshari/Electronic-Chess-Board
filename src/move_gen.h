#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include "types.h"


Bitboard get_legal_moves(U8 sq);

void get_king_moves(U8 sq, Bitboard *board);
void get_bishop_moves(U8 sq, Bitboard *board);
void get_rook_moves(U8 sq, Bitboard *board);
void get_knight_moves(U8 sq, Bitboard *board);
bit is_square_attacked(U8 sq, bit attacker_color);


#endif