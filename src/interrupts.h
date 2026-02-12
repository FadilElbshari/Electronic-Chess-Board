#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"


#define MAX_RX_PAYLOAD (BOARD_W * BOARD_W) + 2
#define MAX_TX_PAYLOAD 5

#define HEADER 0xAA
#define CONNECTION_PACKET 0x01
#define BOARD_PACKET 0x02
#define MOVE_PACKET 0x03
 
 typedef enum {
    RX_WAIT_HEADER,
    RX_WAIT_TYPE,
    RX_WAIT_LEN,
    RX_WAIT_DATA,
    RX_WAIT_CHECKSUM
} RX_STATE;
 
void process_rx_packet(void);
void process_tx_packet(void);
 

extern volatile FLAG LED_READY;
extern volatile FLAG CONNECTED;
extern volatile FLAG POSITION_DONE;
extern volatile FLAG MOVE_RECEIVED;
extern volatile FLAG IS_RESET;

extern volatile U8 MoveSquares[4];
extern volatile Bitboard idata DisplayBoardLEDs;

extern volatile U8 CurrentISRState;

extern volatile U8 rxState;
extern volatile U8 rxType;
extern volatile U8 rxLen;
extern volatile U8 idata rxIndex;
extern volatile U8 idata rxChecksum;
extern volatile U8 xdata rxBuffer[MAX_RX_PAYLOAD];
extern volatile FLAG RX_PACKET_READY;

extern volatile U8 idata txHeader;
extern volatile U8 idata txType;
extern volatile U8 idata txLen;
extern volatile U8 idata txChecksum;
extern volatile U8 idata txBuffer[MAX_TX_PAYLOAD];
extern volatile FLAG TX_PACKET_READY;

#endif