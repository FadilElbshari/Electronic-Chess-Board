#include "tm_ssd.h"

#include <REG52.H>
#include <intrins.h>
#include "types.h"
#include "hardware.h"

// ===============================================================================
// =========================Seven Segment Display START===========================
// ===============================================================================

// ============================ Useful Constant Values ===============================
const U8 xdata seg_table[18] = {
        0x3F,0x06,0x5B,0x4F,0x66, // 0-4
        0x6D,0x7D,0x07,0x7F,0x6F, // 5-9
			0x77,0x7C,0x39,0x5E,0x79,0x71,0x3D,0x76, // A-E
};

void tm_delay_us(void) {
    _nop_(); // Instead of a loop
}


void tm_start(void) {
    SSD_DIO = 1;
    SSD_CLK = 1;
    tm_delay_us();
    SSD_DIO = 0;
    tm_delay_us();
    SSD_CLK = 0;
}
void tm_stop(void) {
    SSD_CLK = 0;
    tm_delay_us();
    SSD_DIO = 0;
    tm_delay_us();
    SSD_CLK = 1;
    tm_delay_us();
    SSD_DIO = 1;
}
void tm_send_byte(U8 b) {
    U8 i;
    for (i = 0; i < 8; i++) {
        SSD_CLK = 0;                // clock low before setting data
        SSD_DIO = (b & 0x01);       // send LSB first
        tm_delay_us();
        SSD_CLK = 1;                // clock high: latch bit
        tm_delay_us();
        b >>= 1;
    }

    // ACK bit
    SSD_CLK = 0;
    SSD_DIO = 1; // release data line
    tm_delay_us();
    SSD_CLK = 1;
    tm_delay_us();
    SSD_CLK = 0;
}
void tm_send_command(U8 cmd) {
    tm_start();
    tm_send_byte(cmd);
    tm_stop();
}
void tm_display_digits(U8 d0, U8 d1, U8 d2, U8 d3) {
    // Segment data for 0–9

    tm_send_command(0x40); // set auto-increment mode

    tm_start();
    tm_send_byte(0xC0); // start address 0
    tm_send_byte(seg_table[d0]);	
    tm_send_byte(seg_table[d1]);
    tm_send_byte(seg_table[d2]);
    tm_send_byte(seg_table[d3]);
    tm_stop();

    tm_send_command(0x8F); // display ON, max brightness
}
// ==============================================================================
// =========================Seven Segment Display END ===========================
// ==============================================================================