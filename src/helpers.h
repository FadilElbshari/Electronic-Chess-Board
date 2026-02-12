#ifndef HELPERS_H
#define HELPERS_H

#include "types.h"
// Function Prototypes
void timer0_delay1ms(void);
void timer0_init(void);
void delay_ms(U16 ms);
void delay_us(U16 d);
U8 get_bit_count(const Bitboard* board);

void board_copy(Bitboard *dst, const Bitboard *src);
extern volatile U16 sys_ms;
 
extern volatile U16 settle_timer;
extern volatile U16 ui_timer;

extern const U8 code pop4[16];

extern const U8 code BitMask[8];
extern const U8 code BitMaskClr[8];

#define BITCOUNT8(x) (pop4[(x) & 0x0F] + (pop4[((x) & 0xF0) >> 4]))
#define clear_leds() set_leds(&ZeroBoard)



#endif