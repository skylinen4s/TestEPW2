#include "control.h"

#define Vset		100
#define Period		100 //ms
#define cmd_times	50 //times

xTimerHandle ctrlTimer;
xTimerHandle testTimer;
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
				USART_puts(USART3, "forward");
			}
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
		stateTimer = xTimerCreate("EPW state checking", (20), pdTRUE, (void *)7, checkState);
		xTimerStart(stateTimer, 0);
	}
}

void checkState(){
	EPW_State = getEPWState();
	USART_puts(USART3, "epw");
	USART_putd(USART3, EPW_State);
	USART_puts(USART3, " ");


	if(CMD_State == EPW_STOP){
		if(EPW_State == EPW_STOP) EPW_State = EPW_IDLE;
	}

	if(EPW_State == EPW_IDLE) xTimerStop(stateTimer, 0);
}

void motorStop(){
	mStop(mBoth);
	CMD_State = EPW_STOP;
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
/*
	USART_puts(USART3, "fl:");
	USART_putd(USART3, SpeedValue_left);
	USART_puts(USART3, " fr:");
	USART_putd(USART3, SpeedValue_right);
	USART_puts(USART3, "\r\nel:");
	USART_putd(USART3, cnt[0]);
	USART_puts(USART3, " rl:");
	USART_putd(USART3, cnt[1]);
	USART_puts(USART3, "\r\n");
*/
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

/**********************************************************
                motor behavior test function
 **********************************************************/
static void motortest()
{
	int cnt[2];
	cnt[0] = getEncoderLeft();
	cnt[1] = getEncoderRight();

	int cur[3];
	cur[0] = getCurAll();
	cur[1] = getCurLeft();
	cur[2] = getCurRight();

	cmd_cnt++;
	if(cmd_cnt == 400 || CMD_State == EPW_STOP){
		SpeedValue_left += 12;
		SpeedValue_right += 12;
		// increase duty cycle 1% every period
		mMove(SpeedValue_left, SpeedValue_right);

		cmd_cnt = 0;
		if(SpeedValue_right > 900 || CMD_State == EPW_STOP){
			mStop(mBoth);
			xTimerDelete(testTimer, 0);
			endofRecord();
		}
	}
	recControlData2(SpeedValue_left, SpeedValue_right, cnt[0], cnt[1], cur[1], cur[2]);
}

static void motorResp()
{
	int cnt[2];
	cnt[0] = getEncoderLeft();
	cnt[1] = getEncoderRight();

	int cur[2];
	cur[0] = getCurLeft();
	cur[1] = getCurRight();

	cmd_cnt++;
	if(cmd_cnt >= 500 || CMD_State == EPW_STOP){
		mStop(mBoth);
		if(!(cnt[0] || cnt[1])){
			xTimerDelete(testTimer, 0);
			endofRecord();
		}
	}
	recControlData2(SpeedValue_left, SpeedValue_right, cnt[0], cnt[1], cur[0], cur[1]);
}

PID_t pid_l;
PID_t pid_r;

void motorPID(){

	int cnt[2];
	cnt[0] = getEncoderLeft();
	cnt[1] = getEncoderRight();

	int cur[2];
	cur[0] = getCurLeft();
	cur[1] = getCurRight();

	if(cmd_cnt && (CMD_State != EPW_STOP)){
		--cmd_cnt;

		SpeedValue_left = 600 + computePID(&pid_l, 20, cnt[0]);
		SpeedValue_right = 600 + computePID(&pid_r, 20, cnt[1]);

		mMove(SpeedValue_left,SpeedValue_right);
	}
	else{
		mStop(mBoth);
		//CMD_State = EPW_STOP;
		if(!(cnt[0] || cnt[1])){
			xTimerDelete(testTimer, 0);
			USART_puts(USART3, "delete\r\n");
			endofRecord();
		}
	}

	//recControlData(SpeedValue_left, SpeedValue_right, cnt[0], cnt[1]);
	recControlData2(SpeedValue_left, SpeedValue_right, cnt[0], cnt[1], cur[0], cur[1]);
}

void motorTest()
{
	SpeedValue_left = 96;
	SpeedValue_right = 96;
	mMove(SpeedValue_left,SpeedValue_right);

	CMD_State = EPW_IDLE;

	//cmd_cnt = 270; //for motorPID
	cmd_cnt = 0; // fot motortest

	if((xTimerIsTimerActive(testTimer) != pdTRUE) || (testTimer == NULL)){
		testTimer = xTimerCreate("test motor", (20), pdTRUE, (void *) 6, motortest);

		//init_PID(&pid_l, 12.0f, 0.2f, 0.5f);
		//init_PID(&pid_r, 12.0f, 0.2f, 0.5f);

		USART_puts(USART3, "ctrl:");
		USART_putd(USART3, testTimer);
		USART_puts(USART3, "\r\n");
		xTimerStart(testTimer, 0);
	}
	else{
		xTimerReset(testTimer, 0);
	}
}

extern int fnn_l, fnn_r;

/* for model testing */
void testFNN(){
	int cnt[2];
	cnt[0] = getEncoderLeftSign();
	cnt[1] = getEncoderRightSign();

	int cur[2];
	cur[0] = getCurLeft();
	cur[1] = getCurRight();

	// current related to encoder
	/*static l_cur[2];

	if(cnt[0] < 0) cur[0] = 0 -cur[0];
	else if(cnt[0] == 0){
			if(l_cur[0] < 0) cur[0] = 0 - cur[0];
			else if(l_cur[0] > 0) cur[0] = cur[0];
	}
	if(cnt[1] <= 0) cur[1] = 0 -cur[1];
	else if(cnt[1] == 0){ 
			if(l_cur[1] < 0) cur[1] = 0 - cur[1];
			else if(l_cur[1] > 0) cur[1] = cur[1];
	}
	l_cur[0] = cur[0];
	l_cur[1] = cur[1];*/

	fnn_l = 0;
	fnn_r = 0;

	if(cmd_cnt && (CMD_State != EPW_STOP)){
		--cmd_cnt;
		if(cmd_cnt > 3800)fzyNeuCtrl(cnt[0], cnt[1], 5.0f, 5.0f);
		else if(cmd_cnt > 3650)fzyNeuCtrl(cnt[0], cnt[1], 10.0f, 5.0f);
		else if(cmd_cnt > 3500)fzyNeuCtrl(cnt[0], cnt[1], 10.0f, 10.0f);
		else if(cmd_cnt > 3350)fzyNeuCtrl(cnt[0], cnt[1], 10.0f, 15.0f);
		else if(cmd_cnt > 3200)fzyNeuCtrl(cnt[0], cnt[1], 15.0f, 15.0f);
		else if(cmd_cnt > 3000)fzyNeuCtrl(cnt[0], cnt[1], 10.0f, 10.0f);
		else if(cmd_cnt > 2750)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 5.0f);
		else if(cmd_cnt > 2600)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 0.0f);
		else if(cmd_cnt > 2400)fzyNeuCtrl(cnt[0], cnt[1], -5.0f, 0.0f);
		else if(cmd_cnt > 2200)fzyNeuCtrl(cnt[0], cnt[1], -5.0f, -7.5f);
		else if(cmd_cnt > 2050)fzyNeuCtrl(cnt[0], cnt[1], -7.5f, -7.5f);
		else if(cmd_cnt > 1800)fzyNeuCtrl(cnt[0], cnt[1], -15.0f, -10.0f);
		else if(cmd_cnt > 1650)fzyNeuCtrl(cnt[0], cnt[1], -15.0f, -15.0f);
		else if(cmd_cnt > 1500)fzyNeuCtrl(cnt[0], cnt[1], -10.0f, -15.0f);
		else if(cmd_cnt > 1250)fzyNeuCtrl(cnt[0], cnt[1], -10.0f, 0.0f);
		else if(cmd_cnt > 1000)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 5.0f);
		else if(cmd_cnt > 750)fzyNeuCtrl(cnt[0], cnt[1], 5.0f, 0.0f);
		else if(cmd_cnt > 500)fzyNeuCtrl(cnt[0], cnt[1], 5.0f, -5.0f);
		else if(cmd_cnt > 250)fzyNeuCtrl(cnt[0], cnt[1], 5.0f, -10.0f);
		else if(cmd_cnt > 100)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, -5.0f);
		else if(cmd_cnt > 0)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 0.0f);

		if(fnn_l < 0) fnn_l = fnn_l*3;
		if(fnn_r < 0) fnn_r = fnn_r*3;

		SpeedValue_left = 600 + fnn_l;
		SpeedValue_right = 600 + fnn_r;
		mMove(SpeedValue_left,SpeedValue_right);

	}
	else{
		mStop(mBoth);
		//CMD_State = EPW_STOP;
		if(!(cnt[0] || cnt[1])){
			xTimerDelete(testTimer, 0);
			USART_puts(USART3, "delete\r\n");
			endofRecord();
		}
	}

	// current related to control signals
	if(SpeedValue_left < 500) cur[0] = 0 - cur[0];
	if(SpeedValue_right < 500) cur[1] = 0 - cur[1];

	recControlData3(SpeedValue_left, SpeedValue_right, cnt[0], cnt[1], cur[0], cur[1],fnn_l, fnn_r);
}

void measFNN(){
	int cnt[2];
	cnt[0] = getEncoderLeftSign();
	cnt[1] = getEncoderRightSign();

	int cur[2];
	cur[0] = getCurLeft();
	cur[1] = getCurRight();

	static float sp = 32.0f;
	if(cmd_cnt && (CMD_State != EPW_STOP)){
		--cmd_cnt;

		if(cmd_cnt == 0){
			sp -= 1;
			cmd_cnt = 250;
			USART_puts(USART3, "value: ");
			USART_putd(USART3, math_round(sp));
			USART_puts(USART3, "\n\r");
		}
		else if(cmd_cnt < 300) fzyNeuCtrl(cnt[0], cnt[1], sp, sp);


		if(sp == -21.0f) cmd_cnt = 0;

		if(fnn_l < 0) fnn_l = fnn_l*2;
		if(fnn_r < 0) fnn_r = fnn_r*2;

		SpeedValue_left = 600 + fnn_l;
		SpeedValue_right = 600 + fnn_r;
		mMove(SpeedValue_left,SpeedValue_right);
	}
	else{
		mStop(mBoth);
		//CMD_State = EPW_STOP;
		if(!(cnt[0] || cnt[1])){
			xTimerDelete(testTimer, 0);
			USART_puts(USART3, "delete\r\n");
			endofRecord();
		}
	}

	recControlData3(SpeedValue_left, SpeedValue_right, cnt[0], cnt[1], cur[0], cur[1],fnn_l, fnn_r);
}

void moveFNN(){
	int cnt[2];
	cnt[0] = getEncoderLeftSign();
	cnt[1] = getEncoderRightSign();

	int cur[2];
	cur[0] = getCurLeft();
	cur[1] = getCurRight();

	fnn_l = 0;
	fnn_r = 0;

	if(cmd_cnt && (CMD_State != EPW_STOP)){
		--cmd_cnt;

		/*if(cmd_cnt > 3150)fzyNeuCtrl(cnt[0], cnt[1], 20.0f, 20.0f);
		else if(cmd_cnt > 3050)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 0.0f);
		else if(cmd_cnt > 2800)fzyNeuCtrl(cnt[0], cnt[1], 20.0f, 20.0f);
		else if(cmd_cnt > 2700)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 0.0f);
		else if(cmd_cnt > 2450)fzyNeuCtrl(cnt[0], cnt[1], 20.0f, 20.0f);
		else if(cmd_cnt > 2350)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 0.0f);
		else if(cmd_cnt > 2100)fzyNeuCtrl(cnt[0], cnt[1], 20.0f, 20.0f);
		else if(cmd_cnt > 2000)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 0.0f);
		else if(cmd_cnt > 1750)fzyNeuCtrl(cnt[0], cnt[1], 20.0f, 20.0f);
		else if(cmd_cnt > 1650)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 0.0f);
		else if(cmd_cnt > 1400)fzyNeuCtrl(cnt[0], cnt[1], 20.0f, 20.0f);
		else if(cmd_cnt > 1300)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 0.0f);
		else if(cmd_cnt > 1050)fzyNeuCtrl(cnt[0], cnt[1], 20.0f, 20.0f);
		else if(cmd_cnt > 950)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 0.0f);
		else if(cmd_cnt > 700)fzyNeuCtrl(cnt[0], cnt[1], 20.0f, 20.0f);
		else if(cmd_cnt > 600)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 0.0f);
		else if(cmd_cnt > 350)fzyNeuCtrl(cnt[0], cnt[1], 20.0f, 20.0f);
		else if(cmd_cnt > 250)fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 0.0f);
		else */
		if(cmd_cnt > 0)fzyNeuCtrl(cnt[0], cnt[1], 20.0f, 20.0f);

		SpeedValue_left = 600 + fnn_l;
		SpeedValue_right = 600 + fnn_r;
		mMove(SpeedValue_left,SpeedValue_right);

	}
	else{
		//mStop(mBoth);
		fzyNeuCtrl(cnt[0], cnt[1], 0.0f, 0.0f);
		//CMD_State = EPW_STOP;
		if(!(cnt[0] || cnt[1])){
			xTimerDelete(testTimer, 0);
			USART_puts(USART3, "delete\r\n");
			endofRecord();
		}
	}

	// current related to control signals
	if(SpeedValue_left < 590) cur[0] = 0 - cur[0];
	if(SpeedValue_right < 590) cur[1] = 0 - cur[1];

	recControlData3(SpeedValue_left, SpeedValue_right, cnt[0], cnt[1], cur[0], cur[1],fnn_l, fnn_r);
}

void motorFNN(){
	initFNN();

	/* reset encoder counter */
	getEncoderLeftSign();
	getEncoderRightSign();

	SpeedValue_left = 600;
	SpeedValue_right = 600;

	cmd_cnt = 250;
	/* command time(ms) = cmd_cnt * period(20ms) */

	if((xTimerIsTimerActive(testTimer) != pdTRUE) || (testTimer == NULL)){
		testTimer = xTimerCreate("test motor", (20), pdTRUE, (void *) 6, moveFNN);

		USART_puts(USART3, "ctrl:");
		USART_putd(USART3, testTimer);
		USART_puts(USART3, "\r\n");
		xTimerStart(testTimer, 0);
	}
	else{
		xTimerReset(testTimer, 0);
	}
}