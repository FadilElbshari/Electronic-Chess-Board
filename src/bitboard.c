#include "bitboard.h"
#include "types.h"

// ===================================================================================
// ============================= Board State Data ====================================
// Reading Starts from a1 to e5
// First bit is a1 (LSB)
Bitboard xdata PolledBoard = {{0x1F, 0x1B, 0x04, 0x1F}};
Bitboard xdata CurrentBoard = {{0xFF, 0xFF, 0xFF, 0xFF}};
Bitboard xdata DiffMask = {{0x00, 0x00, 0x00, 0x00}};
Bitboard xdata LeftMask = {{0x00, 0x00, 0x00, 0x00}};
Bitboard xdata EnteredMask = {{0x00, 0x00, 0x00, 0x00}};
Bitboard xdata board = {{0x1, 0x2, 0x3, 0x4}};// 0101 0101
bit MATCH = 0;
// ===================================================================================

bit compare_boards(Bitboard *a, Bitboard *b) {
    U8 i;
    for (i = 0; i < 4; i++) {
        if (a->RANK[i] != b->RANK[i])
            return 0;   // mismatch
    }
    return 1;           // all match
}