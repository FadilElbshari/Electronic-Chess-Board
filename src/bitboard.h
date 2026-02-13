#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"

bit compare_boards(Bitboard *a, Bitboard *b);
void clear_legal_moves(void);

void reset_game(void);

extern Bitboard PolledBoard;
extern Bitboard CurrentBoard;
extern Bitboard LeftMask;
extern Bitboard EnteredMask;
extern Bitboard LegalMoves;
extern Bitboard IntermediateBoard;

extern const Bitboard code ZeroBoard;
extern const Bitboard code OneBoard;

extern bit MATCH;
extern bit TURN;
extern bit COLOR;

extern U8 KingSquares[2];
extern U8 RookMoved[2][2];
extern U8 KingMoved[2];
extern volatile U8 idata BoardState[BOARD_W * BOARD_W];

extern U8 LiftedPieceSquare;
extern U8 LiftedCaptureSquare;
extern U8 GameOverInfo; // 0 - STALEMATE, 1 - CHECKMATE


#endif