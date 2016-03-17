#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "timers.h"

/* include EPW LIB. */
#include "motor.h"
#include "encoder.h"
#include "uart.h"

void check(void);
void checkMotor(void);
void test_forward(void);
void forward(void);

#endif
