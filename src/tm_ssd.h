#ifndef TM_SSD_H
#define TM_SSD_H

#include "types.h"
#include <intrins.h>
// SSD Communication
#define tm_delay_us()  _nop_()

void tm_start(void);
void tm_stop(void);
void tm_send_byte(U8 b);
void tm_send_command(U8 cmd);
void tm_display_digits(U8 d0, U8 d1, U8 d2, U8 d3);


#endif