#ifndef __SENSORS_H__
#define __SENSORS_H__

#include "stm32f4xx.h"
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

#define IND_LEFT_PIN			GPIO_Pin_6 //PE6
#define IND_RIGHT_PIN			GPIO_Pin_2 //PC2

void cCurAll(void);
void cCurMotor(void);
void getCurData(void);
int getCurAll(void);
int getCurLeft(void);
int getCurRight(void);
void init_CurTransducer(void);
static void init_CurADC(void);
static void init_CurDMA(void);

char getIndicator(void);
void init_Indicator(void);

#endif /* __SENSORS_H__ */
