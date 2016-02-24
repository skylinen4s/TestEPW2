#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "stm32f4xx_gpio.h"
#include "FreeRTOS.h"
#include "timers.h"

#define ENCODER_PORT 		GPIOA

#define ENCODER_LEFT_A_PIN	GPIO_Pin_0
#define ENCODER_RIGHT_A_PIN	GPIO_Pin_1
#define ENCODER_LEFT_B_PIN	GPIO_Pin_2
#define ENCODER_RIGHT_B_PIN	GPIO_Pin_3

typedef enum {
	STOP,
	CW,
	CCW
}Rotate_t;

typedef struct {
	int count;
	uint8_t state;
	Rotate_t roatation;
	uint16_t phaseA;
	uint16_t phaseB;
}Encoder_t;
void Encoder_Polling();
void init_encoder(void);
void init_encoder_exti(uint8_t EXTI_PinX, uint8_t EXTIx_IRQn, uint32_t EXTI_LineX);
void EXTI0_IRQHandler();
void EXTI1_IRQHandler();
void EXTI2_IRQHandler();
void EXTI3_IRQHandler();
void detachEXTI(uint32_t EXTI_LineX);
void attachEXTI(uint32_t EXTI_LineX);

static void getEncoderState(Encoder_t* encoder);
void getEncoder(void);

#endif /* __ENCODER_H__ */
