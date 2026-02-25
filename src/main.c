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
#include "lcd.h"


int main(void) {
	
	timer0_init();
	uart_init();
	init_shift_reg();
	reset_game();
	clear_leds();
	lcd_init();
	
	lcd_print("Chess Board");
	
	ui_timer = 100;
	while (1) {
		
		task_handle_flags();
		
		switch (CurrentMainState) {
			
			case TURNED_ON:
				task_turnon();
				break;
			
			case AWAIT_INITIAL_POSITION_SET:
				lcd_print("Set Position");
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

