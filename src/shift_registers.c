#include "shift_registers.h"

#include "types.h"
#include "config.h"
#include "hardware.h"
#include "helpers.h"
#include "tm_ssd.h"
#include "bitboard.h"


void figure_out_move(Bitboard *pos_board, Bitboard *new_board) {
	U8 i;
	for (i=0; i<4; i++) DiffMask.RANK[i] = pos_board->RANK[i] ^ new_board->RANK[i];
}

void get_left_entered(Bitboard *pos_board, Bitboard *new_board) {
	U8 i;
	for (i=0; i<4; i++) {
		LeftMask.RANK[i] = pos_board->RANK[i] & (~new_board->RANK[i]);
		EnteredMask.RANK[i] = (~pos_board->RANK[i]) & new_board->RANK[i];
	}
}


bit read_and_verify_sensors(void) {
	Bitboard xdata first, second;
  U8 attempts = 3;
	
	while (attempts--) {
        first = read_hall_effect_sensors();
        delay_ms(5);
        second = read_hall_effect_sensors();

        if (compare_boards(&first, &second)) {
            PolledBoard = first;
            return 1;
        }
    }
    return 0;
}


Bitboard read_hall_effect_sensors(void) {
	U8 i, j;
	U8 value;
	Bitboard xdata board = {0};
	
	CLK_ENABLE = 1;
	SHIFT_nLOAD = 0;
	tm_delay_us();
	SHIFT_nLOAD = 1;
	clock_gen_parallel();
	CLK_ENABLE = 0;
	
	
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++){
			value = SERIAL_DATA_IN;
			value &= 1;
			board.RANK[j] |= ((!value) << i);
			clock_gen_parallel();
		}
	}
	/*
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++){
			value = SERIAL_DATA_IN;
			value &= 1;
			board.RANK[i+4] |= ((!value) << j);
			clock_gen_parallel();
		}
	}
	
	
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++){
			value = SERIAL_DATA_IN;
			value &= 1;
			board.RANK[i] |= ((!value) << (j+4));
			clock_gen_parallel();
		}
	}
	
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++){
			value = SERIAL_DATA_IN;
			value &= 1;
			board.RANK[i+4] |= ((!value) << (j+4));
			clock_gen_parallel();
		}
	}
	*/
	
	return board;
}

void set_leds(Bitboard *to_light_up) {
	U8 i;
	
	U8 r0 = to_light_up->RANK[0];
  U8 r1 = to_light_up->RANK[1];
  U8 r2 = to_light_up->RANK[2];
  U8 r3 = to_light_up->RANK[3];
	
	MASTER_RESET = 0;
	clock_gen_serial();
	MASTER_RESET = 1;
	
	delay_ms(1);
	
	for (i = 0; i < 4; i++) {          // i = bit index (your (3-i))
        U8 shift = (U8)(3 - i);

        SERIAL_DATA_OUT = (r3 >> shift) & 1; clock_gen_serial();
        SERIAL_DATA_OUT = (r2 >> shift) & 1; clock_gen_serial();
        SERIAL_DATA_OUT = (r1 >> shift) & 1; clock_gen_serial();
        SERIAL_DATA_OUT = (r0 >> shift) & 1; clock_gen_serial();
    }
	
		LATCH_RELEASE = 0;
		LATCH_RELEASE = 1;
		tm_delay_us();
		LATCH_RELEASE = 0;
}


void clock_gen_serial() {
	SERIAL_CLK = 1;
	tm_delay_us();
	SERIAL_CLK = 0;
}
void clock_gen_parallel() {
	PARALLEL_CLK = 1;
	tm_delay_us();
	PARALLEL_CLK = 0;
}