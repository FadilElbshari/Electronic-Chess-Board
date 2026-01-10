#ifndef SHIFT_REGISTERS_H
#define SHIFT_REGISTERS_H

#include "types.h"
#include "config.h"
#include "hardware.h"
#include "bitboard.h"

void clock_gen_serial();
void clock_gen_parallel();
void set_leds(Bitboard *to_light_up);
Bitboard read_hall_effect_sensors(void);

bit read_and_verify_sensors(void);
void get_left_entered(Bitboard *pos_board, Bitboard *new_board);

#endif