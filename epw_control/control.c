#include "control.h"

#define Vset		100
#define Period		100 //ms
#define cmd_times	50 //times
xTimerHandle ctrlTimer;

extern Encoder_t ENCODER_L;
extern Encoder_t ENCODER_R;

extern uint32_t SpeedValue_left;
extern uint32_t SpeedValue_right;
uint32_t cmd_cnt = 0;
int cnt[2];


void getData(){
	detachEXTI(EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line3);
	
	cnt[0] = ENCODER_L.count;
	cnt[1] = ENCODER_R.count;

	resetEncoder(&ENCODER_L);
	resetEncoder(&ENCODER_R);

	USART_puts(USART3, "le_en:");
	USART_putd(USART3, cnt[0]);
	USART_puts(USART3, " ri_en:");
	USART_putd(USART3, cnt[1]);
	USART_puts(USART3, "\r\n");

	attachEXTI(EXTI_Line0 | EXTI_Line1 | EXTI_Line2 | EXTI_Line3);
}

void test_forward(){
	ctrlTimer = xTimerCreate("encoder Polling", (Period), pdTRUE, (void *) 1, forward);
	xTimerStart(ctrlTimer, 0);
}

void forward(){
	getData();

	if(cnt[0] || cnt[1]) ++cmd_cnt;

	if(cnt[0] < 100) SpeedValue_left++;
	else if(cnt[0] > 100) SpeedValue_left--;
	
	if(cnt[1] < 100) SpeedValue_right++;
	else if(cnt[1] > 100) SpeedValue_right--;

	mMove(SpeedValue_left, SpeedValue_right);
	if(cmd_cnt > cmd_times){
		mStop();
		xTimerStop(ctrlTimer, 0);
		cmd_cnt = 0;
	}
}
