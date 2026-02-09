#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"


#define MAX_RX_PAYLOAD (BOARD_W * BOARD_W) + 1
#define MAX_TX_PAYLOAD 5

#define HEADER 0xAA
#define CONNECTION_PACKET 0x01
#define BOARD_PACKET 0x02
#define MOVE_PACKET 0x03

// ============================= Serial ISR Variable =================================
 
 typedef enum {
    RX_WAIT_HEADER,
    RX_WAIT_TYPE,
    RX_WAIT_LEN,
    RX_WAIT_DATA,
    RX_WAIT_CHECKSUM
} RX_STATE;
 
void process_rx_packet(void);
void process_tx_packet(void);
 

extern volatile bit LED_READY;
extern volatile bit CONNECTED;
extern volatile bit POSITION_DONE;
extern volatile bit MOVE_RECEIVED;
extern volatile bit IS_RESET;

extern volatile U8 xdata MoveSquares[4];
extern volatile Bitboard xdata DisplayBoardLEDs;

extern volatile U8 CurrentISRState;

extern volatile U8 rxState;
extern volatile U8 rxType;
extern volatile U8 rxLen;
extern volatile U8 rxIndex;
extern volatile U8 rxChecksum;
extern volatile U8 xdata rxBuffer[MAX_RX_PAYLOAD];
extern volatile bit rxPacketReady;

extern volatile U8 txHeader;
extern volatile U8 txType;
extern volatile U8 txLen;
extern volatile U8 txChecksum;
extern volatile U8 xdata txBuffer[MAX_TX_PAYLOAD];
extern volatile bit txPacketReady;
// ===================================================================================

#endif