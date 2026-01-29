#ifndef HELPERS_H
#define HELPERS_H

#include "types.h"
// Function Prototypes
void timer0_delay1ms(void);
void delay_ms(unsigned int ms);
U8 get_bit_count(const Bitboard* board);



extern const U8 code pop4[16];

#define BITCOUNT8(x) (pop4[(x) & 0x0F] + (pop4[((x) & 0xF0) >> 4]))



#endif