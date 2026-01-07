#ifndef UART_H
#define UART_H

#include "types.h"

#define TX_BUF_SIZE 64

extern volatile U8 xdata tx_buf[TX_BUF_SIZE];
extern volatile U8 xdata tx_head;
extern volatile U8 xdata tx_tail;
extern volatile bit tx_busy;

//Uart Communication
void uart_init(void);
void serial_ISR(void);
void uart_send_byte(unsigned char c);
void uart_send_string(const char code *s);

#endif