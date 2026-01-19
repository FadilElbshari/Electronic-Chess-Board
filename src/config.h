#ifndef CONFIG_H
#define CONFIG_H

// Macro Definitions
#define set(pin) pin=1
#define clr(pin) pin=0
#define init_shift_reg() (P1 = 0xD8)

#define WHITE 1
#define BLACK 0

#endif