#include <REG52.H>


// Local Headers
#include "types.h"
#include "helpers.h"
#include "config.h"
#include "hardware.h"
#include "bitboard.h"
#include "uart.h"
#include "shift_registers.h"
#include "tm_ssd.h"
#include "interrupts.h"

// Shift Registers


		
const U8 xdata FILES_5[5] = {'a', 'b', 'c', 'd', 'e'}; // Cols
const U8 xdata RANKS_5[5] = {'1', '2', '3', '4', '5'}; // Rows

const Bitboard xdata ZeroBoard = {{0x00, 0x00, 0x00, 0x00}};
const Bitboard xdata OneBoard = {{0xFF, 0xFF, 0xFF, 0xFF}};


bit JustEnteredState = 1;
MainState CurrentMainState = TURNED_ON;
DetectionState CurrentDetectionState = NONE;

U8 DelayCounter = 0;

int main(void) {
	uart_init();
	init_shift_reg();
	set_leds(&ZeroBoard);
	
	DelayCounter = 10;
	while (1) {
		
		
		if (DelayCounter > 0) DelayCounter--;
		switch (CurrentMainState) {
			
			case TURNED_ON:
				if (JustEnteredState) {
					JustEnteredState = 0;
					set_leds(&OneBoard);
				}
				
				if (CONNECTED) {
					JustEnteredState = 1;
					DelayCounter = 10;
					CurrentMainState = AWAIT_POSITION_SET;
				}
				break;
				
			
			case AWAIT_POSITION_SET:
				
				if (JustEnteredState && LED_READY) {
					JustEnteredState = 0;
					LED_READY = 0;
					set_leds(&DisplayBoardLEDs);
					DelayCounter = 2;
				}
				
				if (DelayCounter != 0) break;

				if (!read_and_verify_sensors()) {
					DelayCounter = 2;
					break;
				}
				MATCH = compare_boards(&DisplayBoardLEDs, &PolledBoard); 
				
				if (!MATCH) {
					DelayCounter = 2;
					break;
				}
				
				CurrentBoard = PolledBoard;
				JustEnteredState = 1;
				CurrentMainState = DETECTING;
				break;
			
			
			case DETECTING:
				if (JustEnteredState) {
					JustEnteredState = 0;
					set_leds(&ZeroBoard);
					DelayCounter = 2;
				}
				
				switch (CurrentDetectionState) {
					case NONE:
						if (DelayCounter != 0) break;
						if (!read_and_verify_sensors()) {
							DelayCounter = 2;
							break;
						}
						
						MATCH = compare_boards(&CurrentBoard, &PolledBoard);

						if (!MATCH) {
							JustEnteredState = 1;
							CurrentMainState = CHANGE_DETECTED;
							break;
						}

						DelayCounter = 2;
						break;
					
					case LIFT:
						if (LED_READY) {
							LED_READY = 0;
							set_leds(&DisplayBoardLEDs);
						}
						break;
						
			}
				break;
				
				
			case CHANGE_DETECTED:
				if (JustEnteredState) {
					U8 i, j;
					JustEnteredState = 0;
					//figure_out_move(&CurrentBoard, &PolledBoard);
					get_left_entered(&CurrentBoard, &PolledBoard);
					
					switch (CurrentDetectionState){
						case NONE:
							if (get_bit_count(LeftMask) == 1) {
								CurrentDetectionState = LIFT;
								JustEnteredState = 1;
								CurrentMainState = DETECTING;
								for (i=0; i<4; i++) {
									for (j=0; j<4; j++) {
										if ((LeftMask.RANK[i] >> j) & 1) {
											uart_send_char(LIFT_FLAG);
											uart_send_char(i+'0');
											uart_send_char(j+'0');
											break;
										}
									}
								}
							} else if (get_bit_count(EnteredMask) > 0) {
									CurrentMainState = ERROR_FLASH_ON;
									break;
							}
						break;
					}
					
				}
			
				break;
				
			
			case ERROR_FLASH_ON:
				if (JustEnteredState) {
					JustEnteredState = 0;
					set_leds(&OneBoard);
				}
				if (DelayCounter != 0) break;
				CurrentMainState = ERROR_FLASH_OFF;
				DelayCounter = 10;
				JustEnteredState = 1;
				break;
			
			case ERROR_FLASH_OFF:
				if (JustEnteredState) {
					JustEnteredState = 0;
					set_leds(&ZeroBoard);
				}
				if (DelayCounter != 0) break;
				CurrentMainState = ERROR_FLASH_ON;
				DelayCounter = 10;
				JustEnteredState = 1;
				break;
		}
		
		delay_ms(10);
	}
}
