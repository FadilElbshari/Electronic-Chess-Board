#include "bitboard.h"
#include "types.h"
#include "config.h"

// ===================================================================================
// ============================= Board State Data ====================================
// Reading Starts from a1 to e5
// First bit is a1 (LSB)
Bitboard PolledBoard;
Bitboard CurrentBoard;
Bitboard LeftMask;
Bitboard EnteredMask;
Bitboard xdata LegalMoves;
Bitboard xdata IntermediateBoard;
Bitboard xdata MoveBoard;

#if BOARD_W == 4
	const Bitboard code ZeroBoard = {{0x00, 0x00, 0x00, 0x00}};
	const Bitboard code OneBoard = {{0xFF, 0xFF, 0xFF, 0xFF}};

#elif BOARD_W == 8
	const Bitboard code ZeroBoard = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
	const Bitboard code OneBoard = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
	
#else
#error "Unsupported BOARD_W: must be 4 or 8"
#endif

bit MATCH = 0;
bit TURN = WHITE;
bit COLOR = WHITE;

U8 KingSquares[2] = {0, 0};
U8 xdata RookMoved[2][2] = {{0, 0}, {0, 0}};
U8 xdata KingMoved[2] = {0, 0};

U8 xdata BoardState[BOARD_W * BOARD_W];

U8 LiftedPieceSquare = 0;


// ===================================================================================
void CLEAR_LEGAL_MOVES() {
	LegalMoves.RANK[0] = 0;
	LegalMoves.RANK[1] = 0;
	LegalMoves.RANK[2] = 0;
	LegalMoves.RANK[3] = 0;
	LegalMoves.RANK[4] = 0;
	LegalMoves.RANK[5] = 0;
	LegalMoves.RANK[6] = 0;
	LegalMoves.RANK[7] = 0;
}

bit compare_boards(Bitboard *a, Bitboard *b) {
    U8 i;
    for (i = 0; i < BOARD_W; i++) {
        if (a->RANK[i] != b->RANK[i])
            return 0;   // mismatch
    }
    return 1;           // all match
}

