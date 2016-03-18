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
uint32_t fl, fr;

State_t EPW_State = EPW_UNREADY;

void checkState(){
	if(EPW_State == EPW_IDLE){
		/* accept all epw command */
	}
	else if(EPW_State == EPW_STOP){
		/* check epw status and return to IDLE */
	}
	else if(EPW_State == EPW_FORWARD){
		/* forward */
	}
	else if(EPW_State == EPW_BACKWARD){
		/* backward */
	}
	else if(EPW_State == EPW_LEFT){
		/* turn left */
	}
	else if(EPW_State == EPW_RIGHT){
		/* turn right*/
	}
	else if(EPW_State == EPW_UNREADY){
		/* after PowerOn, check the status of motors and test motor */
	}
	else if(EPW_State == EPW_BUSY){
		/* use linear actuator would be in this state */
	}
	else if(EPW_State == EPW_ERROR){
		/* something wrong */
	}

}
void check(){
	fl = 0;
	fr = 0;
	cmd_cnt = 30;
	ctrlTimer = xTimerCreate("motor checking", (75), pdTRUE, (void *)6, checkMotor);
	xTimerStart(ctrlTimer, 0);
}

void checkMotor(){
	if(!fl){
		if(!getEncoderLeft()){
			++SpeedValue_left;
			mMove(SpeedValue_left, SpeedValue_right);
		}
		else{
			fl = SpeedValue_left;
			mStop(mLeft);
		}
	}
	if(!fr){
		if(!getEncoderRight()){
			++SpeedValue_right;
			mMove(SpeedValue_left, SpeedValue_right);
		}
		else{
			fr = SpeedValue_right;
			mStop(mRight);
		}
	}

	if(fl && fr){
			mStop(mBoth);
			xTimerDelete(ctrlTimer, 0);

			USART_puts(USART3, "cmd_cnt:");
			USART_putd(USART3, cmd_cnt);
			USART_puts(USART3, " l:");
			USART_putd(USART3, fl);
			USART_puts(USART3, " r:");
			USART_putd(USART3, fr);
	}

	if(!(--cmd_cnt)){
		xTimerDelete(ctrlTimer, 0);
		USART_puts(USART3, "EPW_UNREADY\r\n");
	}
}

void test_forward(){
	if(xTimerIsTimerActive(ctrlTimer) != pdTRUE){
		ctrlTimer = xTimerCreate("forward control", (Period), pdTRUE, (void *) 1, forward);
		xTimerStart(ctrlTimer, 0);
	}
	else{
		cmd_cnt = 0;
		xTimerReset(ctrlTimer, 0);
	}
}

void forward(){
	int cnt[2];
	cnt[0] = getEncoderLeft();
	cnt[1] = getEncoderRight();

	if(cnt[0] || cnt[1]) ++cmd_cnt;

	if(cnt[0] < 100) SpeedValue_left++;
	else if(cnt[0] > 100) SpeedValue_left--;
	
	if(cnt[1] < 100) SpeedValue_right++;
	else if(cnt[1] > 100) SpeedValue_right--;

	mMove(SpeedValue_left, SpeedValue_right);
	if(cmd_cnt > cmd_times){
		mStop(mBoth);
		xTimerStop(ctrlTimer, 0);
		cmd_cnt = 0;
	}
}
