#include "PID.h"
#include "uart.h"

void init_PID(PID_t *pid, float p, float i, float d){
	pid->Kp = p;
	pid->Ki = i;
	pid->Kd = d;
	pid->errSum = 0.0f;
	pid->errLast = 0.0f;
	pid->output = 0.0f;
}

/*************************************************************************
 * The Formula of Incremental PID controller:
 * e(n) =   x(n)   -  y(n)
 *	    = Setpoint - Input
 * output:
 * u(n) = Kp * e(n) + Ki * SUM(e(i), i:from 0 to n) + Kd * (e(n) - e(n-1)) 
 ************************************************************************/
int computePID(PID_t *pid, float setpoint, float input){
	
	/* Compute all the working error variables*/
	float err = setpoint - input;
	float dErr = (err - pid->errLast);
	pid->errSum += err;

	/* Compute PID output */
	pid->output = pid->Kp * err \
				+ pid->Ki * pid->errSum \
				+ pid->Kd * dErr;

	//if((pid->output < 900.0f) && (pid->output > 180.0f)) pid->output = pid->output;
	if(pid->output > 600.0f) pid->output = 600.0f;
	else if(pid->output < 0.0f) pid->output = 0.0f;

	/* Update the error for next calculation */
	pid->errLast = err;

	return math_round(pid->output);
}