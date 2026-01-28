#include "shift_registers.h"

#include "types.h"
#include "config.h"
#include "hardware.h"
#include "helpers.h"
#include "tm_ssd.h"
#include "bitboard.h"


void get_left_entered(Bitboard *pos_board, Bitboard *new_board) {
	U8 i;
	for (i=0; i<BOARD_W; i++) {
		LeftMask.RANK[i] = pos_board->RANK[i] & (~new_board->RANK[i]);
		EnteredMask.RANK[i] = (~pos_board->RANK[i]) & new_board->RANK[i];
	}
}


bit read_and_verify_sensors(void) {
	Bitboard first, second;
  U8 attempts = 3;
	
	while (attempts--) {
        read_hall_effect_sensors(&first);
        delay_ms(5);
        read_hall_effect_sensors(&second);

        if (compare_boards(&first, &second)) {
            PolledBoard = first;
            return 1;
        }
    }
    return 0;
}


void read_hall_effect_sensors(Bitboard *board) {
	U8 i, j;
	U8 value;
	*board = ZeroBoard;
	
	CLK_ENABLE = 1;
	SHIFT_nLOAD = 0;
	tm_delay_us();
	SHIFT_nLOAD = 1;
	clock_gen_parallel();
	CLK_ENABLE = 0;
	
	// Bottom left board
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++){
			value = SERIAL_DATA_IN;
			value &= 1;
			board->RANK[j] |= ((!value) << i);
			clock_gen_parallel();
		}
	}

#if BOARD_W == 8
	
	// Bottom right board
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++){
			value = SERIAL_DATA_IN;
			value &= 1;
			board->RANK[j] |= ((!value) << (i+4));
			clock_gen_parallel();
		}
	}
	
	// Top right board
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++){
			value = SERIAL_DATA_IN;
			value &= 1;
			board->RANK[j+4] |= ((!value) << (i+4));
			clock_gen_parallel();
		}
	}
	
	// Top left board
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++){
			value = SERIAL_DATA_IN;
			value &= 1;
			board->RANK[j+4] |= ((!value) << i);
			clock_gen_parallel();
		}
	}

#endif
}

void set_leds(const Bitboard *to_light_up) {
	U8 i;
	
	U8 r0 = to_light_up->RANK[0];
  U8 r1 = to_light_up->RANK[1];
  U8 r2 = to_light_up->RANK[2];
  U8 r3 = to_light_up->RANK[3];
	
#if BOARD_W == 8
	U8 r4 = to_light_up->RANK[4];
  U8 r5 = to_light_up->RANK[5];
  U8 r6 = to_light_up->RANK[6];
  U8 r7 = to_light_up->RANK[7];
#endif
	
	MASTER_RESET = 0;
	clock_gen_serial();
	MASTER_RESET = 1;
	
	delay_ms(1);
	
	// Top left board
	for (i = 0; i < 4; i++) {          // i = bit index (your (3-i))
		U8 shift = (U8)(3 - i);

		SERIAL_DATA_OUT = (r7 >> shift) & 1; clock_gen_serial();
		SERIAL_DATA_OUT = (r6 >> shift) & 1; clock_gen_serial();
		SERIAL_DATA_OUT = (r5 >> shift) & 1; clock_gen_serial();
		SERIAL_DATA_OUT = (r4 >> shift) & 1; clock_gen_serial();
		
  }
	
	// Top right board
	for (i = 0; i < 4; i++) {          // i = bit index (your (3-i))
		U8 shift = (U8)(7 - i);

		SERIAL_DATA_OUT = (r7 >> shift) & 1; clock_gen_serial();
		SERIAL_DATA_OUT = (r6 >> shift) & 1; clock_gen_serial();
		SERIAL_DATA_OUT = (r5 >> shift) & 1; clock_gen_serial();
		SERIAL_DATA_OUT = (r4 >> shift) & 1; clock_gen_serial();
		
  }
	
	// Bottom right board
	for (i = 0; i < 4; i++) {          // i = bit index (your (3-i))
		U8 shift = (U8)(7 - i);

		SERIAL_DATA_OUT = (r3 >> shift) & 1; clock_gen_serial();
		SERIAL_DATA_OUT = (r2 >> shift) & 1; clock_gen_serial();
		SERIAL_DATA_OUT = (r1 >> shift) & 1; clock_gen_serial();
		SERIAL_DATA_OUT = (r0 >> shift) & 1; clock_gen_serial();
		
  }
	
	// Bottom left board
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