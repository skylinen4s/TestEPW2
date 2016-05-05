#include "control.h"

#define Vset		100
#define Period		100 //ms
#define cmd_times	50 //times

xTimerHandle ctrlTimer;
xTimerHandle stateTimer;

extern Encoder_t ENCODER_L;
extern Encoder_t ENCODER_R;

extern uint32_t SpeedValue_left;
extern uint32_t SpeedValue_right;
uint32_t cmd_cnt = 0;

State_t EPW_State = EPW_IDLE;
State_t CMD_State = EPW_IDLE;

void processCMD(uint8_t id, uint8_t value){
	USART_putd(USART3, EPW_State);

	if(id == CMD_ACTU_A) set_linearActuator_A_cmd(value);
	else if(id == CMD_ACTU_B) set_linearActuator_B_cmd(value);

	/* block the command if it doesn't pass the test of motor*/
	//if(EPW_State == EPW_NOTRDY) return;

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
				test_backward();
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
		default:
			break;
	}

	if(stateTimer != NULL){
		xTimerReset(stateTimer, 0);
	}
	else{
		stateTimer = xTimerCreate("EPW state checking", (100), pdTRUE, (void *)7, checkState);
		xTimerStart(stateTimer, 0);
	}
}

void checkState(){
	EPW_State = getEPWState();

	if(CMD_State == EPW_STOP){
		if(EPW_State == EPW_STOP) EPW_State = EPW_IDLE;
	}

	if(EPW_State == EPW_IDLE) xTimerStop(stateTimer, 0);
}

uint32_t fl, fr;
void test_forward(){
	cmd_cnt = 50;

	if(fl && fr){
		SpeedValue_left = fl;
		SpeedValue_right = fr;
	}

	if((xTimerIsTimerActive(ctrlTimer) != pdTRUE) || (ctrlTimer == NULL)){
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

	if(cmd_cnt && (CMD_State != EPW_STOP)){
		/* start counting only if encoder get data(motor moving)
		 * moving period = cmd_cnt * Period */
		if(cnt[0] || cnt[1]) --cmd_cnt;
		SpeedValue_left += (cnt[0] < 90)? 1: (cnt[0] > 100)? -1: 0;
		SpeedValue_right += (cnt[1] < 90)? 1: (cnt[1] > 100)? -1: 0;

		mMove(SpeedValue_left, SpeedValue_right);
		/* record the value for next forward command */
		fl = SpeedValue_left;
		fr = SpeedValue_right;
	}
	else{
		mStop(mBoth);
		CMD_State = EPW_STOP;
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

uint32_t bl, br;
void test_backward(){
	cmd_cnt = 50;

	if(bl && br){
		SpeedValue_left = bl;
		SpeedValue_right = br;
	}

	if((xTimerIsTimerActive(ctrlTimer) != pdTRUE) || (ctrlTimer == NULL)){
		ctrlTimer = xTimerCreate("backward control", (Period), pdTRUE, (void *) 2, backward);
		xTimerStart(ctrlTimer, 0);
	}
	else{
		xTimerReset(ctrlTimer, 0);
	}
}

void backward(){
	int cnt[2];
	cnt[0] = getEncoderLeft();
	cnt[1] = getEncoderRight();

	if(cmd_cnt && (CMD_State != EPW_STOP)){
		/* start counting only if encoder get data(motor moving)
		 * moving period = cmd_cnt * Period */
		if(cnt[0] || cnt[1]) --cmd_cnt;
		SpeedValue_left -= (cnt[0] < 90)? 1: (cnt[0] > 100)? -1: 0;
		SpeedValue_right -= (cnt[1] < 90)? 1: (cnt[1] > 100)? -1: 0;

		mMove(SpeedValue_left, SpeedValue_right);
		/* record the value for next backward command */
		bl = SpeedValue_left;
		br = SpeedValue_right;
	}
	else{
		mStop(mBoth);
		CMD_State = EPW_STOP;
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
