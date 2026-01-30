#ifndef SHIFT_REGISTERS_H
#define SHIFT_REGISTERS_H

#include "types.h"
#include "config.h"
#include "hardware.h"
#include "bitboard.h"

#define LIFT_DELAY 5
#define SLDING_DELAY 300

void clock_gen_serial();
void clock_gen_parallel();
void set_leds(const Bitboard *to_light_up);
void read_hall_effect_sensors(Bitboard *board);

bit read_and_verify_sensors(void);
void get_left_entered(Bitboard *pos_board, Bitboard *new_board);

#endif