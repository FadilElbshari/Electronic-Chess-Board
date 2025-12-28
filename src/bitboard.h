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
extern Bitboard xdata TempBoard;
extern Bitboard xdata DiffMask;
extern Bitboard xdata LeftMask;
extern Bitboard xdata EnteredMask;
extern Bitboard xdata LegalMoves;

extern const Bitboard xdata ZeroBoard;
extern const Bitboard xdata OneBoard;

extern bit MATCH;
extern bit TURN;

extern U8 xdata KingSquares[2];
extern U8 xdata BoardState[16];

extern U8 xdata LiftedPieceSquare;

// ===================================================================================

#endif