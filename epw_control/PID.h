#ifndef __PID_H__
#define __PID_H__

#include "stm32f4xx.h"
#include "clib.h"

typedef struct PID_Struct{
	float Kp;
	float Ki;
	float Kd;
	float errSum;
	float errLast;
	float output;
} PID_t;

extern inline void init_PID(PID_t *pid, float p, float i, float d); 
extern inline int computePID(PID_t *pid, float setpoint, float input);
#endif
