#ifndef TASKS_H
#define TASKS_H

#include "types.h"

void reset_game();
void task_handle_flags();
void task_turnon();
void task_await_initpos();
void task_await_moveset();
void task_gameover();
void task_error_on();
void task_error_off();


extern FLAG JUST_ENTERED_STATE;
extern FLAG IN_ERROR;
extern U8 CurrentMainState;
extern U8 CurrentDetectionState;
extern U8 ErrorFlashCount;

#endif