#ifndef __FNN_H__
#define __FNN_H__

#include "stm32f4xx.h"
#include "clib.h"

#define num 	3
#define rule_num 	num*num
/* rule number */

typedef struct MBF_Struct{
	float c;
	float b;
} MBF_t;

void initMBF(MBF_t *mf, float c, float b);
void initFNN();
void fzyNeuCtrl(int en_l, int en_r, float setpoint);
float compute_MBF(MBF_t *mf, float err);
float referModel(float xm, float rm);
void referState(float v11, float v21, float v1, float v2);
void switching(float e1, float e2);
void fzyNeu_update(float e1, float e2);

#endif
