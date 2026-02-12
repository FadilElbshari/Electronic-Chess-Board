#include "uart.h"
#include <REG52.H>
#include "types.h"


volatile U8 idata tx_buf[TX_BUF_SIZE];
volatile U8 tx_head = 0;
volatile U8 tx_tail = 0;
volatile bit tx_busy = 0;

// Initialise serial interface for UART at baud-rate = 57600
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

// Handle byte sending logic via serial interface TX
void uart_send_byte(U8 c) {
	U8 next;
	
	next = (tx_head + 1) % TX_BUF_SIZE; // Get the index of next byte - Protection from overflow
	if (next == tx_tail) return; // Return if last byte is reached, head => current data, tail => end of data
	
	tx_buf[tx_head] = c;
	tx_head = next;

	// Check if something is being transmitted
	if (!tx_busy) {
			tx_busy = 1;
			SBUF = tx_buf[tx_tail];
			tx_tail = (tx_tail + 1) % TX_BUF_SIZE;
	}
}