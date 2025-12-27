#ifndef CONFIG_H
#define CONFIG_H

// Macro Definitions
#define set(pin) pin=1
#define clr(pin) pin=0
#define init_shift_reg() (P1 = 0xD8)
#define get_bit_count(board) popcount(board.RANK[0])+popcount(board.RANK[1])+popcount(board.RANK[2])+popcount(board.RANK[3])

#define LIFT_FLAG '!'
#define CONNECTION_FLAG '#'
#define LEGAL_MOVE_FLAG '*'

#define WHITE 1
#define BLACK 0

#endif