#include "sensors.h"

#define CUR_TRANS_SIZE		3
uint16_t CurValue[CUR_TRANS_SIZE];

void cCurAll(){
	float all = (float)CurValue[0];
	all = 5-2*3*(all/4096);
	all*=1000;

	USART_puts(USART3, "ALL:");
	USART_putd(USART3, CurValue[0]);
	USART_puts(USART3, " (v):");
	USART_putd(USART3, (uint32_t)all);
	USART_puts(USART3, " (a):");
	USART_putd(USART3, (uint32_t)(all*5));
	USART_puts(USART3, "\r\n");
}

void cCurMotor(){
	float left = (float)CurValue[1];
	float right = (float)CurValue[2];
	left = left*3/4; //Vall = (ADCvalue / 4096)*3(V)*1000(mV)
	right = right*3/4;


	USART_puts(USART3, "Left:");
	USART_putd(USART3, CurValue[1]);
	USART_puts(USART3, " (v):");
	USART_putd(USART3, (uint32_t)left);
	USART_puts(USART3, " (a):");
	left*=2.5; // component: HX-10P 1V=2.5A
	USART_putd(USART3, (uint32_t)left);
	USART_puts(USART3, "\r\n");

	USART_puts(USART3, "Right:");
	USART_putd(USART3, CurValue[2]);
	USART_puts(USART3, " (v):");
	USART_putd(USART3, (uint32_t)right);
	USART_puts(USART3, " (a:");
	right*=2.5;
	USART_putd(USART3, (uint32_t)right);
	USART_puts(USART3, "\r\n");
}

void getCurData(){
	cCurAll();
	cCurMotor();
}

int getCurAll(){
	float all = (float)CurValue[0];
	all = 5-2*3*(all/4096);
	all*=1000;
	return all;
}

int getCurLeft(){
	int left = CurValue[1];
	static int last_left = 0;

	/* low-pass filter */
	left = (left + (last_left << 3) - last_left) >> 3;
	last_left = left;

	/* Vall = (ADCvalue / 4096)*3(V)*1000(mV) */
	left = ((left << 2) - left) >> 2;
	return left;
}

int getCurRight(){
	int right = CurValue[2];
	static int last_right = 0;

	/* low-pass filter */
	right = (right + (last_right << 3) - last_right) >> 3;
	last_right = right;

	/* Vall = (ADCvalue / 4096)*3(V)*1000(mV) ~= ADC*3/4 */
	right = ((right << 2) - right) >> 2;
	return right;
}

void init_CurTransducer(){
	init_CurDMA();
	init_CurADC();

	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);

	ADC_SoftwareStartConv(ADC1);
}

static void init_CurADC(){
	GPIO_InitTypeDef GPIO_InitStruct;
	ADC_InitTypeDef ADC_InitStruct;
	ADC_CommonInitTypeDef ADC_CommonInitStruct;

	/* Enable GPIO C clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	GPIO_InitStruct.GPIO_Pin = CUR_ALL_PIN | CUR_LEFT_PIN | CUR_RIGHT_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(CUR_TRANS_PORT, &GPIO_InitStruct);

	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStruct.ADC_ScanConvMode = ENABLE;
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_NbrOfConversion = CUR_TRANS_SIZE;
	ADC_Init(ADC1, &ADC_InitStruct);

	ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div4; //f = 84/4=21MHz
	ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStruct);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_15Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 2, ADC_SampleTime_15Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 3, ADC_SampleTime_15Cycles);

	ADC_Cmd(ADC1, ENABLE);
}

static void init_CurDMA(){
	DMA_InitTypeDef DMA_InitStruct;
	/* ADC1 refer to DMA2_Channel0 (Stream0 or Stream4)*/
	/* ENABLE DMA2 clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	DMA_InitStruct.DMA_Channel = DMA_Channel_0; /*ADC1*/
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)0x4001204C; /* this value equals to ADC1->DR */
	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)&CurValue[0];
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStruct.DMA_BufferSize = CUR_TRANS_SIZE;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStruct.DMA_Priority = DMA_Priority_High;
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStruct);
	DMA_Cmd(DMA2_Stream0, ENABLE);
}

char getIndicator(void)
{
	char indi_state;
	indi_state = GPIO_ReadInputDataBit(GPIOE, IND_LEFT_PIN)? 0x01 : 0x00;
	indi_state |= GPIO_ReadInputDataBit(GPIOC, IND_RIGHT_PIN)? 0x02 : 0x00;
	/*
	 * 00 : all pass
	 * 01 : left fail
	 * 10 : right fail
	 * 11 : all fail
	 */
	return indi_state;
}

void init_Indicator(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_InitStruct.GPIO_Pin = IND_LEFT_PIN;
	GPIO_Init(GPIOE, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = IND_RIGHT_PIN;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
}
