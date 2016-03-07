#include "encoder.h"

Encoder_t ENCODER_L;
Encoder_t ENCODER_R;

State_t EPW_STATE;

xTimerHandle EncoderTimer;
#define ENCODER_PERIOD 1000 //ms
void Encoder_Polling(){
	getEncoder();
}

void resetEncoder(Encoder_t* encoder){
	encoder->count = 0;
	encoder->state &= ~0xff;
	encoder->rotate = STOP;
}

/* initialize the encoder */
void init_encoder(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	//Enable GPIO clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = ENCODER_LEFT_A_PIN | ENCODER_RIGHT_A_PIN
				 | ENCODER_LEFT_B_PIN | ENCODER_RIGHT_B_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(ENCODER_PORT, &GPIO_InitStruct);

	resetEncoder(&ENCODER_L);
	resetEncoder(&ENCODER_R);
	ENCODER_L.phaseA = ENCODER_LEFT_A_PIN;
	ENCODER_L.phaseB = ENCODER_LEFT_B_PIN;
	ENCODER_R.phaseA = ENCODER_RIGHT_A_PIN;
	ENCODER_R.phaseB = ENCODER_RIGHT_B_PIN;

	init_encoder_exti(EXTI_PinSource0, EXTI0_IRQn, EXTI_Line0);
	init_encoder_exti(EXTI_PinSource1, EXTI1_IRQn, EXTI_Line1);
	init_encoder_exti(EXTI_PinSource2, EXTI2_IRQn, EXTI_Line2);
	init_encoder_exti(EXTI_PinSource3, EXTI3_IRQn, EXTI_Line3);

	/* get encoder data every period */
	//EncoderTimer = xTimerCreate("Encoder Polling", (ENCODER_PERIOD), pdTRUE, (void *) 1, Encoder_Polling);
	//xTimerStart(EncoderTimer, 0);
}

void init_encoder_exti(uint8_t EXTI_PinX, uint8_t EXTIx_IRQn, uint32_t EXTI_LineX){
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* connect EXTI LineX to PAx pin (x = 0,1,2,3)*/
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinX);
	EXTI_InitStruct.EXTI_Line = EXTI_LineX;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	EXTI_ClearITPendingBit(EXTI_LineX);
	NVIC_InitStruct.NVIC_IRQChannel = EXTIx_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

/* ENCODER LEFT phase A */
void EXTI0_IRQHandler(){
	if(EXTI_GetITStatus(EXTI_Line0) != RESET){
		ENCODER_L.count++;
		getEncoderState(&ENCODER_L);
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}

/* ENCODER RIGHT phase A */
void EXTI1_IRQHandler(){
	if(EXTI_GetITStatus(EXTI_Line1) != RESET){
		ENCODER_R.count++;
		getEncoderState(&ENCODER_R);
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}

/* ENCODER LEFT phase B */
void EXTI2_IRQHandler(){
	if(EXTI_GetITStatus(EXTI_Line2) != RESET){
		ENCODER_L.count++;
		getEncoderState(&ENCODER_L);
		EXTI_ClearITPendingBit(EXTI_Line2);
	}
}

/* ENCODER RIGHT phase B */
void EXTI3_IRQHandler(){
	if(EXTI_GetITStatus(EXTI_Line3) != RESET){
		ENCODER_R.count++;
		getEncoderState(&ENCODER_R);
		EXTI_ClearITPendingBit(EXTI_Line3);
	}
}

void detachEXTI(uint32_t EXTI_LineX){
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_LineX;
	EXTI_InitStruct.EXTI_LineCmd = DISABLE;
}

void attachEXTI(uint32_t EXTI_LineX){
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_LineX;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
}

State_t getEPWState(State_t epw_state, Encoder_t* encoder_L, Encoder_t* encoder_R){
	if((encoder_L->rotate == STOP) && (encoder_R->rotate == STOP)) epw_state = EPW_STOP;
	else if((encoder_L->rotate == CCW) && (encoder_R->rotate == CW)) epw_state = EPW_FORWARD;
	else if((encoder_L->rotate == CW) && (encoder_R->rotate == CCW)) epw_state = EPW_BACKWARD;
	else if((encoder_L->rotate == CW) && (encoder_R->rotate == CW)) epw_state = EPW_LEFT;
	else if((encoder_L->rotate == CCW) && (encoder_R->rotate == CCW)) epw_state = EPW_RIGHT;

	return epw_state;
}

static void getEncoderState(Encoder_t* encoder){
	encoder->state = (encoder->state << 2 & 0x0f) | (GPIO_ReadInputDataBit(ENCODER_PORT, encoder->phaseA) << 1) | (GPIO_ReadInputDataBit(ENCODER_PORT, encoder->phaseB));
	encoder->rotate = encoder_states[encoder->state & 0x0f];
	/*if(encoder->state == (0x00 || 0x05 || 0x0a || 0x0f )) encoder->rotate = ENCODER_STOP;
	else if(encoder->state == (0x01 || 0x07 || 0x08 || 0x0e )) encoder->rotate = ENCODER_CW;
	else if(encoder->state == (0x02 || 0x04 || 0x0b || 0x0d )) encoder->rotate = ENCODER_CCW;
	else encoder->rotate = ENCODER_ERR;*/

	EPW_STATE = getEPWState(EPW_STATE, &ENCODER_L, &ENCODER_R);
}

void getEncoder(void){
	detachEXTI(EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line3);

	USART_puts(USART3, "L_state:");
	USART_putd(USART3, ENCODER_L.rotate);

	USART_puts(USART3, " R_state:");
	USART_putd(USART3, ENCODER_R.rotate);

	USART_puts(USART3, " state:");
	USART_putd(USART3, EPW_STATE);
	USART_puts(USART3, "\r\n");

	getEncoderState(&ENCODER_L);
	getEncoderState(&ENCODER_R);

	USART_puts(USART3, " L_state2:");
	USART_putd(USART3, ENCODER_L.rotate);
	USART_puts(USART3, " R_state2:");
	USART_putd(USART3, ENCODER_R.rotate);
	USART_puts(USART3, " state:");
	USART_putd(USART3, EPW_STATE);
	USART_puts(USART3, "\r\n");

	USART_puts(USART3, "le_en:");
	USART_putd(USART3, ENCODER_L.count);
	USART_puts(USART3, " ri_en:");
	USART_putd(USART3, ENCODER_R.count);
	USART_puts(USART3, "\r\n");

	/* clear encoder counter*/
	ENCODER_L.count = 0;
	ENCODER_R.count = 0;

	attachEXTI(EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line3);
}
