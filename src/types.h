#ifndef TYPES_H
#define TYPES_H

// Type Definitions
typedef unsigned char U8;
typedef struct {
	U8 RANK[4];
} Bitboard;

typedef struct {
	U8 RANK[8];
} Bitboard_8x8;

typedef enum {
	TURNED_ON,
	AWAIT_POSITION_SET,
	DETECTING,
	CHANGE_DETECTED,
	ERROR_FLASH_ON,
	ERROR_FLASH_OFF
} MainState;

typedef enum {
	NONE,
	LIFT
} DetectionState;

typedef enum {
	NOT_CONNECTED,
	RECEIVING_POSITION,
	WAITING
} ISRState;

#endif