#include "clib.h"

int math_round(float num){
	return (num > 0) ? num + 0.5f : num - 0.5f;
}

int math_abs(float num){
	return (num > 0)? num : -(num);
}

float f_sign(float num){
	return (num > 0.0f)? 1.0f : -1.0f;
}

float f_abs(float num){
	return (num > 0.0f)? num : -(num);
}

float exponential(float ld){
	float result = 1.0;
	float term = ld;
	int diaminator = 2;
	int count = 0;
	int test = 0;

	while(count < 10){
		result = result + term;
		term = term * ld;
		term = term / (float)diaminator;
		(int)diaminator++;
		count++;
		test = math_abs(term*1000);
		if(test < 10) break;
	}
	return result;
}

float math_pow(float num, int times){
	int i = 0;
	float tmp = 1.0f;
	for(i = 0; i < times; ++i) tmp = num * tmp;

	return tmp;
}