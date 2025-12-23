#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"

bit compare_boards(Bitboard *a, Bitboard *b);

// ===================================================================================
// ============================= Board State Data ====================================
// Reading Starts from a1 to e5
// First bit is a1 (LSB)
extern Bitboard xdata PolledBoard;
extern Bitboard xdata CurrentBoard;
extern Bitboard xdata DiffMask;
extern Bitboard xdata LeftMask;
extern Bitboard xdata EnteredMask;
extern Bitboard xdata board;
extern bit MATCH;
// ===================================================================================

#endif