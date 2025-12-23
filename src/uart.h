#ifndef UART_H
#define UART_H

//Uart Communication
void uart_init(void);
void serial_ISR(void);
void uart_send_char(unsigned char c);
void uart_send_string(const char code *s);

#endif