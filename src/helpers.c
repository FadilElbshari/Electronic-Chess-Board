#include "helpers.h"

#include <REG52.H>
#include <intrins.h>

// Time counting variables - marked as volatile as managed by ISR of T0
volatile U16 sys_ms = 0; 
volatile U16 settle_timer = 0;
volatile U16 ui_timer = 0;

// Number of set bits for numbers 0-15 (4bit numbers)
const U8 code pop4[16] = {
    0,1,1,2,1,2,2,3,
    1,2,2,3,2,3,3,4
};

const U8 code BitMask[8]    = {1,2,4,8,16,32,64,128};
const U8 code BitMaskClr[8] = {0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F};

// Initialise T0 by loading proper values into relevant registers
void timer0_init(void) {
	TMOD &= 0xF0;
	TMOD |= 0x01;
    
	TH0 = 0xFC;
	TL0 = 0x67;
	
	ET0 = 1;
	EA = 1;
	TR0 = 1;
}

// T0 interrupt handler
void timer0_ISR(void) interrupt 1 {
    TH0 = 0xFC;
    TL0 = 0x67;
    sys_ms++;

    if (settle_timer) settle_timer--;
    if (ui_timer)     ui_timer--;
}

// Optimised function to retrieve number of set bits per bitboard (8x8 bits)
U8 get_bit_count(const Bitboard *board) {
		return BITCOUNT8(board->RANK[0]) +
				BITCOUNT8(board->RANK[1]) +
				BITCOUNT8(board->RANK[2]) +
				BITCOUNT8(board->RANK[3]) +
				BITCOUNT8(board->RANK[4]) +
				BITCOUNT8(board->RANK[5]) +
				BITCOUNT8(board->RANK[6]) +
				BITCOUNT8(board->RANK[7]); 
}

// A function to generate delays in order of microseconds, nop() uses one machine cycle ~1.08usf at 11.0592MHz crystal
void delay_us(U16 d) {
    while(d--) _nop_();
}

void delay_ms(U16 ms) {
    U16 start = sys_ms;
    while ((U16)(sys_ms - start) < ms);
}

void board_copy(Bitboard *dst, const Bitboard *src) {
    U8 i;
    for (i = 0; i < 8; i++) dst->RANK[i] = src->RANK[i];
}