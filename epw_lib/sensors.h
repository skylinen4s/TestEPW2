#ifndef __SENSORS_H__
#define __SENSORS_H__

#include "stm32f4xx_adc.h"
#include "uart.h"

/* pins for three current transducer which detecting the currents */
#define CUR_TRANS_PORT			GPIOC
/* total current from battery */
#define CUR_ALL_PIN				GPIO_Pin_1
/* current of left motor */
#define CUR_LEFT_PIN			GPIO_Pin_4
/* current of right motor */
#define CUR_RIGHT_PIN			GPIO_Pin_5

void cCurAll(void);
void cCurMotor(void);
void getCurData(void);
void init_CurTransducer(void);
static void init_CurADC(void);
static void init_CurDMA(void);

#endif /* __SENSORS_H__ */
