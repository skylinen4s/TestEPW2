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

void test_forward(){
	ctrlTimer = xTimerCreate("encoder Polling", (Period), pdTRUE, (void *) 1, forward);
	xTimerStart(ctrlTimer, 0);
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
		mStop();
		xTimerStop(ctrlTimer, 0);
		cmd_cnt = 0;
	}
}
