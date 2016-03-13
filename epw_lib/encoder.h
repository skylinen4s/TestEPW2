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
	EPW_IDLE,
	EPW_STOP,
	EPW_FORWARD,
	EPW_BACKWARD,
	EPW_LEFT,
	EPW_RIGHT,
	EPW_UNREADY,
	EPW_BUSY,
	EPW_ERROR
}State_t;

/* encdoer state list */
enum {
	STOP,
	CW,
	CCW,
	ERR
};
static uint8_t encoder_states[] = {	STOP, CW, CCW, ERR, \
									CCW, STOP, ERR, CW, \
									CW,	ERR, STOP, CCW, \
									ERR, CCW, CW, STOP }; 

typedef struct {
	int count;
	uint16_t phaseA;
	uint16_t phaseB;
	uint8_t state;
	uint8_t rotate;
}Encoder_t;

extern Encoder_t ENCODER_L;
extern Encoder_t ENCODER_R;

void init_encoder(void);
void init_encoder_exti(uint8_t EXTI_PinX, uint8_t EXTIx_IRQn, uint32_t EXTI_LineX);
void EXTI0_IRQHandler();
void EXTI1_IRQHandler();
void EXTI2_IRQHandler();
void EXTI3_IRQHandler();
void detachEXTI(uint32_t EXTI_LineX);
void attachEXTI(uint32_t EXTI_LineX);

void resetEncoder(Encoder_t* encoder);

static void getEncoderState(Encoder_t* encoder);
State_t getState(Encoder_t* encoder_L, Encoder_t* encoder_R);
State_t getEPWState(void);
int getEncoderLeft(void);
int getEncoderRight(void);
void getEncoder(void);

#endif /* __ENCODER_H__ */
