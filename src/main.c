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
#include "move_gen.h"
#include "tasks.h"
#include "detection.h"


int main(void) {
	
	timer0_init();
	uart_init();
	
	// Initialise port 1 pins as outputs except P1.3 (Input)
	P1 = 0xFF;
	P1 &= 0x08;
	
	reset_game();
	clear_leds();
	
	ui_timer = 100;
	while (1) {
		
		task_handle_flags();
		
		switch (CurrentMainState) {
			
			case TURNED_ON:
				task_turnon();
				break;
			
			case AWAIT_INITIAL_POSITION_SET:
				task_await_initpos();
				break;
				
			case AWAIT_MOVE_SET:
				task_await_moveset();
				break;
			
			case DETECTING:
				task_detecting_state();
				break;
				
			case CHANGE_DETECTED:
				task_handle_change();
				break;
				
			case GAME_IS_OVER:
				task_gameover();
				break;
									
			case ERROR_FLASH_ON:
				task_error_on();
				break;

			case ERROR_FLASH_OFF:
				task_error_off();
				break;
		
		}
	}
}

