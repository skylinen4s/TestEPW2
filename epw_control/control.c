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

State_t EPW_State = EPW_NOTRDY;
State_t CMD_State;

void processCMD(uint8_t id, uint8_t value){
	USART_putd(USART3, EPW_State);
	switch(id)
	{
		case CMD_STOP:
			mStop(mBoth);
			//PowerOFF
			CMD_State = EPW_STOP;
			break;
		case CMD_FORWARD:
			if((EPW_State == EPW_IDLE) || (EPW_State == EPW_FORWARD))
			{
				test_forward();
				CMD_State = EPW_FORWARD;
			}
			USART_puts(USART3, "forward");
			break;
		case CMD_BACKWARD:
			if((EPW_State == EPW_IDLE) || (EPW_State == EPW_BACKWARD))
			{
				CMD_State = EPW_BACKWARD;
			}
			USART_puts(USART3, "back");
			break;
		case CMD_LEFT:
			if((EPW_State == EPW_IDLE) || (EPW_State == EPW_LEFT))
			{
				CMD_State = EPW_LEFT;
			}
			USART_puts(USART3, "left");
			break;
		case CMD_RIGHT:
			if((EPW_State == EPW_IDLE) || (EPW_State == EPW_RIGHT))
			{
				CMD_State = EPW_RIGHT;
			}
			USART_puts(USART3, "right");
			break;
		case CMD_ACTU_A:
			if(EPW_State == EPW_IDLE)
			USART_puts(USART3, "ActuA");
			USART_putd(USART3, value);
			set_linearActuator_A_cmd(LINEAR_ACTU_CW);
			break;
		case CMD_ACTU_B:
			if(EPW_State == EPW_IDLE)
			USART_puts(USART3, "ActuB");
			USART_putd(USART3, value);
			set_linearActuator_B_cmd(LINEAR_ACTU_CW);
			break;
		default:
			break;
	}
}

void checkState(){
	switch(CMD_State)
	{
		case EPW_IDLE:
			break;
		case EPW_STOP:
		case EPW_FORWARD:
		case EPW_BACKWARD:
		case EPW_LEFT:
		case EPW_RIGHT:
			EPW_State = getEPWState();
			break;
		case EPW_NOTRDY:
		case EPW_BUSY:
		case EPW_ERROR:
			break;
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

			EPW_State = EPW_IDLE;
			USART_puts(USART3, "cmd_cnt:");
			USART_putd(USART3, cmd_cnt);
			USART_puts(USART3, " l:");
			USART_putd(USART3, fl);
			USART_puts(USART3, " r:");
			USART_putd(USART3, fr);
	}

	if(!(--cmd_cnt)){
		xTimerDelete(ctrlTimer, 0);
		EPW_State = EPW_NOTRDY;
		USART_puts(USART3, "EPW_NOT_READY\r\n");
	}
}

uint32_t mvl, mvr;

void test_forward(){
	cmd_cnt = 50;

	if(mvl && mvr){
		SpeedValue_left = mvl;
		SpeedValue_right = mvr;
	}

	if(xTimerIsTimerActive(ctrlTimer) != pdTRUE){
		ctrlTimer = xTimerCreate("forward control", (Period), pdTRUE, (void *) 1, forward);
		xTimerStart(ctrlTimer, 0);
	}
	else{
		xTimerReset(ctrlTimer, 0);
	}
}

void forward(){
	int cnt[2];
	cnt[0] = getEncoderLeft();
	cnt[1] = getEncoderRight();

	if(cmd_cnt){
		/* start counting only if encoder get data(motor moving)
		 * moving period = cmd_cnt * Period */
		if(cnt[0] || cnt[1]) --cmd_cnt;
		SpeedValue_left += (cnt[0] < 90)? 1: (cnt[0] > 100)? -1: 0;
		SpeedValue_right += (cnt[1] < 90)? 1: (cnt[1] > 100)? -1: 0;

		mMove(SpeedValue_left, SpeedValue_right);
		/* record the value for next forward command */
		mvl = SpeedValue_left;
		mvr = SpeedValue_right;
	}
	else{
		mStop(mBoth);
		if(!(cnt[0] || cnt[1])){
			xTimerDelete(ctrlTimer, 0);
		}
	}

	USART_puts(USART3, "fl:");
	USART_putd(USART3, SpeedValue_left);
	USART_puts(USART3, " fr:");
	USART_putd(USART3, SpeedValue_right);
	USART_puts(USART3, "\r\nel:");
	USART_putd(USART3, cnt[0]);
	USART_puts(USART3, " rl:");
	USART_putd(USART3, cnt[1]);
	USART_puts(USART3, "\r\n");

	recControlData(SpeedValue_left, SpeedValue_right, cnt[0], cnt[1]);
}
