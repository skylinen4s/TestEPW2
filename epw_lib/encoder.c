#include "encoder.h"

Encoder_t ENCODER_L;
Encoder_t ENCODER_R;

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

	ENCODER_L.phaseA = ENCODER_LEFT_A_PIN;
	ENCODER_L.phaseB = ENCODER_LEFT_B_PIN;
	ENCODER_R.phaseA = ENCODER_RIGHT_A_PIN;
	ENCODER_R.phaseB = ENCODER_RIGHT_B_PIN;
}

void init_encoder_exti(void){
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* connect EXTI Line0 to PA0 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	EXTI_InitStruct.EXTI_Line = EXTI_Line0;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	EXTI_ClearITPendingBit(EXTI_Line0);
	NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	/* connect EXTI Line1 to PA1 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource1);
	EXTI_InitStruct.EXTI_Line = EXTI_Line1;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	EXTI_ClearITPendingBit(EXTI_Line1);
	NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	/* connect EXTI Line2 to PA2 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource2);
	EXTI_InitStruct.EXTI_Line = EXTI_Line2;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	EXTI_ClearITPendingBit(EXTI_Line2);
	NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	/* connect EXTI Line3 to PA3 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource3);
	EXTI_InitStruct.EXTI_Line = EXTI_Line3;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	EXTI_ClearITPendingBit(EXTI_Line3);
	NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

static void getEncoderState(Encoder_t* encoder){
	encoder->state = (encoder->state << 2 & 0x0f) | (GPIO_ReadInputDataBit(ENCODER_PORT, encoder->phaseA) << 1) | (GPIO_ReadInputDataBit(ENCODER_PORT, encoder->phaseB));
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

void getEncoder(void){
	detachEXTI(EXTI_Line0);
	detachEXTI(EXTI_Line1);
	detachEXTI(EXTI_Line2);
	detachEXTI(EXTI_Line3);

	USART_puts(USART3, "L_state:");
	USART_putd(USART3, ENCODER_L.state);

	getEncoderState(&ENCODER_L);
	getEncoderState(&ENCODER_R);
	USART_puts(USART3, " L_state stop:");
	USART_putd(USART3, ENCODER_L.state);
	USART_puts(USART3, "\r\n");

	USART_puts(USART3, "le_en:");
	USART_putd(USART3, ENCODER_L.count);
	USART_puts(USART3, " ri_en:");
	USART_putd(USART3, ENCODER_R.count);
	USART_puts(USART3, "\r\n");

	/* clear encoder counter*/
	ENCODER_L.count = 0;
	ENCODER_R.count = 0;

	attachEXTI(EXTI_Line0);
	attachEXTI(EXTI_Line1);
	attachEXTI(EXTI_Line2);
	attachEXTI(EXTI_Line3);
}
