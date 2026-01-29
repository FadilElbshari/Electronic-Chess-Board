#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"

bit compare_boards(Bitboard *a, Bitboard *b);
void CLEAR_LEGAL_MOVES(void);


// ===================================================================================
// ============================= Board State Data ====================================
// Reading Starts from a1 to e5
// First bit is a1 (LSB)
extern Bitboard PolledBoard;
extern Bitboard CurrentBoard;
extern Bitboard LeftMask;
extern Bitboard EnteredMask;
extern Bitboard LegalMoves;
extern Bitboard xdata IntermediateBoard;
extern Bitboard xdata MoveBoard;

extern const Bitboard code ZeroBoard;
extern const Bitboard code OneBoard;

extern bit MATCH;
extern bit TURN;
extern bit COLOR;

extern U8 KingSquares[2];
extern U8 xdata RookMoved[2][2];
extern U8 xdata KingMoved[2];
extern U8 xdata BoardState[BOARD_W * BOARD_W];

extern U8 LiftedPieceSquare;

// ===================================================================================

#endif