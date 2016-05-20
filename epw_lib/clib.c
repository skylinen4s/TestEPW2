#include "clib.h"

int math_round(float num){
	return (num > 0) ? num + 0.5f : num - 0.5f;
}

int math_abs(float num){
	return (num > 0)? num : -(num);
}
