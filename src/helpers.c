#include "helpers.h"

#include <REG52.H>
#include <intrins.h>

// ==============================================================================
// =========================Delay Functionality START ===========================
// ==============================================================================

unsigned int ms_counter = 0;

const U8 code pop4[16] = {
    0,1,1,2,1,2,2,3,
    1,2,2,3,2,3,3,4
};

void timer0_delay1ms(void) {
    TMOD |= 0x01;
    
    TH0 = 0xFC;
    TL0 = 0x67;

    TF0 = 0;
    TR0 = 1;
    while(TF0 == 0);
    TR0 = 0;
}

//void timer0_init(void) {
//	TMOD &= 0xF0;
//	TMOD |= 0x01;
//    
//	TH0 = 0xFC;
//	TL0 = 0x67;
//	
//	ET0 = 1;
//	EA = 1;
//	TR0 = 1;
//	
//	ms_counter = 0;
//}

//void timer0_ISR(void) interrupt 1 {
//	TH0 = 0xFC;
//	TL0 = 0x67;
//	
//	if (ms_counter > 0) ms_counter--;
//}


void delay_ms(unsigned int ms) {
    unsigned int i;
    for(i = 0; i < ms; i++) {
        timer0_delay1ms();
    }
}

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

void delay_us(unsigned int d)
{
    while(d--) _nop_();
}


// ==============================================================================
// =========================Delay Functionality END =============================
// ==============================================================================