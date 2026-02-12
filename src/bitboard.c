#include "bitboard.h"
#include "types.h"
#include "config.h"


// Reading Starts from a1 to e5
// First bit is a1 (LSB)
Bitboard PolledBoard;
Bitboard CurrentBoard;
Bitboard LeftMask;
Bitboard EnteredMask;
Bitboard LegalMoves;
Bitboard IntermediateBoard;

U8 LiftedPieceSquare = 0;
U8 LiftedCaptureSquare = 0;
U8 GameOverInfo = 0;

#if BOARD_W == 4
	const Bitboard code ZeroBoard = {{0x00, 0x00, 0x00, 0x00}};
	const Bitboard code OneBoard = {{0xFF, 0xFF, 0xFF, 0xFF}};

#elif BOARD_W == 8
	const Bitboard code ZeroBoard = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
	const Bitboard code OneBoard = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
	
#else
#error "Unsupported BOARD_W: must be 4 or 8"
#endif

bit MATCH;
bit TURN;
bit COLOR;

U8 KingSquares[2] = {0, 0};
U8 RookMoved[2][2] = {{0, 0}, {0, 0}};
U8 KingMoved[2] = {0, 0};
U8 idata BoardState[BOARD_W * BOARD_W];


// Compare two bitboards by checking the RANKs from one against the other
bit compare_boards(Bitboard *a, Bitboard *b) {
    U8 i;
    for (i = 0; i < BOARD_W; i++) {
        if (a->RANK[i] != b->RANK[i])
            return 0;
    }
    return 1;
}

void clear_legal_moves() {
	LegalMoves.RANK[0] = 0;
	LegalMoves.RANK[1] = 0;
	LegalMoves.RANK[2] = 0;
	LegalMoves.RANK[3] = 0;
	LegalMoves.RANK[4] = 0;
	LegalMoves.RANK[5] = 0;
	LegalMoves.RANK[6] = 0;
	LegalMoves.RANK[7] = 0;
}