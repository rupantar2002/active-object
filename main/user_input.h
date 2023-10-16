#ifndef __USER_INPUT_H__
#define __USER_INPUT_H__
#include <stdint.h>

void user_input_Init();

void user_input_Start();

void user_input_Stop();

void user_input_deInit();

void user_input_Button1PressedCallback(void);

void user_input_Switch1StateCallback(uint8_t state);


#endif //__USER_INPUT_H__