#include "bitboard.h"
#include "types.h"
#include "config.h"

// ===================================================================================
// ============================= Board State Data ====================================
// Reading Starts from a1 to e5
// First bit is a1 (LSB)
Bitboard xdata PolledBoard = {{0x1F, 0x1B, 0x04, 0x1F}};
Bitboard xdata CurrentBoard = {{0xFF, 0xFF, 0xFF, 0xFF}};
Bitboard xdata DiffMask = {{0x00, 0x00, 0x00, 0x00}};
Bitboard xdata LeftMask = {{0x00, 0x00, 0x00, 0x00}};
Bitboard xdata EnteredMask = {{0x00, 0x00, 0x00, 0x00}};
Bitboard xdata LegalMoves = {{0x00, 0x00, 0x00, 0x00}};// 0101 0101

const Bitboard xdata ZeroBoard = {{0x00, 0x00, 0x00, 0x00}};
const Bitboard xdata OneBoard = {{0xFF, 0xFF, 0xFF, 0xFF}};

bit MATCH = 0;
bit TURN = WHITE;

U8 xdata KingSquares[2] = {0, 0};

U8 xdata BoardState[16] = {EMPTY, EMPTY, EMPTY, EMPTY,
													 EMPTY, EMPTY, EMPTY, EMPTY,
													 EMPTY, EMPTY, EMPTY, EMPTY,
													 EMPTY, EMPTY, EMPTY, EMPTY,};

U8 xdata LiftedPieceSquare = 0;


// ===================================================================================

bit compare_boards(Bitboard *a, Bitboard *b) {
    U8 i;
    for (i = 0; i < BOARD_W; i++) {
        if (a->RANK[i] != b->RANK[i])
            return 0;   // mismatch
    }
    return 1;           // all match
}

