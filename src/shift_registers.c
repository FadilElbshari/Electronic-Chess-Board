#include "shift_registers.h"

#include "types.h"
#include "config.h"
#include "hardware.h"
#include "helpers.h"
#include "bitboard.h"

// Function to compute difference between two bitboards in terms of set and cleared bits
void get_left_entered(Bitboard *pos_board, Bitboard *new_board) {
	U8 i;
	for (i=0; i<BOARD_W; i++) {
		LeftMask.RANK[i] = pos_board->RANK[i] & (~new_board->RANK[i]);
		EnteredMask.RANK[i] = (~pos_board->RANK[i]) & new_board->RANK[i];
	}
}

// Function tracking board changes, mainly implemented to allow piece sliding by setting LONG_STABLE_TIME to 300ms
bit read_and_verify_sensors(bit isLong) {
    static Bitboard last;
    static bit active = 0; // Retains state across function calls

    read_hall_effect_sensors(&PolledBoard);

    if (!active) {
        board_copy(&last, &PolledBoard);
        settle_timer = isLong ? LONG_STABLE_TIME : SHORT_STABLE_TIME;
        active = 1;
        return 0;
    }

    if (!compare_boards(&last, &PolledBoard)) {
        board_copy(&last, &PolledBoard);
        settle_timer = isLong ? LONG_STABLE_TIME : SHORT_STABLE_TIME;
        return 0;
    }

    if (settle_timer == 0) {
        active = 0;                  // ready for next detection
        return 1;
    }

    return 0;
}


// Function responsible for hall effect sensor data shifting logic
void read_hall_effect_sensors(Bitboard *board) {
	U8 i, j;
	U8 value;
	*board = ZeroBoard;
	
	CLK_ENABLE = 1;
	SHIFT_nLOAD = 0;
	delay_us(1);
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
	
	delay_us(1);
	
	
#if BOARD_W == 8
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
	
#endif
	
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
		delay_us(1);
		LATCH_RELEASE = 0;
}


// Clock generation functions
void clock_gen_serial() {
	SERIAL_CLK = 1;
	delay_us(1);
	SERIAL_CLK = 0;
}
void clock_gen_parallel() {
	PARALLEL_CLK = 1;
	delay_us(1);
	PARALLEL_CLK = 0;
}