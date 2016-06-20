#ifndef CLIB_H__
#define CLIB_H__

#include "stm32f4xx.h"

int math_round(float num);
int math_abs(float num);
float math_pow(float num, int times);
float f_sign(float num);
float f_abs(float num);
float exponential(float ld);

#endif
