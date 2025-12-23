#include "uart.h"
#include <REG52.H>
#include "types.h"

// ================================================================================
// =========================UART Serial Management START===========================
// ================================================================================
void uart_init(void) {
    TMOD &= 0x0F;
		TMOD |= 0x20;
		TR1 = 1;
		TH1 = 0xFF; // was 0xFD for 9600
		SCON = 0x50;	
		PCON |= 0x80;   // SMOD = 1
		ES = 1;
		EA = 1;
}

void uart_send_char(unsigned char c) {
    TI = 0;
    SBUF = c;
    
    while (TI == 0);
}


void uart_send_string(const char code *s) {
    while(*s) {
        uart_send_char(*s++);
    }
}
// ===============================================================================
// =========================UART Serial Management END============================
// ===============================================================================