#include "uart.h"
#include <REG52.H>
#include "types.h"

// ================================================================================
// =========================UART Serial Management START===========================
// ================================================================================

volatile U8 xdata tx_buf[TX_BUF_SIZE];
volatile U8 xdata tx_head = 0;
volatile U8 xdata tx_tail = 0;
volatile bit tx_busy = 0;


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

void uart_send_byte(unsigned char c) {
	U8 next;
	
	next = (tx_head + 1) % TX_BUF_SIZE;
	if (next == tx_tail) return;
	
	tx_buf[tx_head] = c;
	tx_head = next;

	if (!tx_busy) {
			tx_busy = 1;
			SBUF = tx_buf[tx_tail];
			tx_tail = (tx_tail + 1) % TX_BUF_SIZE;
	}
}


void uart_send_string(const char code *s) {
    while(*s) {
        uart_send_byte(*s++);
    }
}
// ===============================================================================
// =========================UART Serial Management END============================
// ===============================================================================