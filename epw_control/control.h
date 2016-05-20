#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "timers.h"

/* include EPW LIB. */
#include "motor.h"
#include "encoder.h"
#include "uart.h"
#include "record.h"
#include "linear_actuator.h"

#include "PID.h"

typedef enum {
	CMD_STOP = 115,
	CMD_FORWARD = 102,
	CMD_BACKWARD = 98,
	CMD_LEFT = 108,
	CMD_RIGHT = 114,
	CMD_ACTU_A = 120,
	CMD_ACTU_B = 121
}CMD_STATE;

void processCMD(uint8_t id, uint8_t value);
void checkState(void);
void motorStop(void);
void test_forward(void);
void forward(void);
void test_backward(void);
void backward(void);
void motorTest(void);

#endif
