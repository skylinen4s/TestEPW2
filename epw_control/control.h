#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "motor.h"
#include "encoder.h"
#include "FreeRTOS.h"
#include "timers.h"

#include "uart.h"

void getData(void);
void test_forward(void);
void forward(void);

#endif
