/* 20150707 wei han for the test of EPW2 */

#ifndef __COMMAND_H__
#define __COMMAND_H__
#include "stm32f4xx.h"

#define DEBUG_MODE		0
#define USER_MODE		!DEBUG_MODE

extern uint8_t Receive_String_Ready;

extern void receive_task();

static struct RECEIVE_CMD{
	volatile unsigned char _start;
	volatile unsigned char cmd_id;
	volatile unsigned char cmd_value;
	volatile unsigned char _end;
};

#endif /* __COMMAND_H__ */