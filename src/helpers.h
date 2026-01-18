#ifndef HELPERS_H
#define HELPERS_H

#include "types.h"
// Function Prototypes
void timer0_delay1ms(void);
void delay_ms(unsigned int ms);

extern const U8 code pop4[16];

#define BITCOUNT4(x)   (pop4[(x) & 0x0F])

#define get_bit_count(board) \
    ( BITCOUNT4(board.RANK[0]) + \
      BITCOUNT4(board.RANK[1]) + \
      BITCOUNT4(board.RANK[2]) + \
      BITCOUNT4(board.RANK[3]) )

#endif