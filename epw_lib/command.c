#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "stm32f4xx_usart.h"
#include "uart.h"
#include "command.h"
#include "motor.h"
#include "linear_actuator.h"
#include "control.h"
#include "sensors.h"
/*parse*/
#include "string.h"
#include "clib.h"

extern uint32_t SpeedValue_left;
extern uint32_t SpeedValue_right;
uint32_t inc = 1;

struct RECEIVE_CMD *cmd_raw;


#if USER_MODE
void receive_task(){

	USART_puts(USART3, "user mode test\r\n");
	if(Receive_String_Ready){
		cmd_raw = received_string;
		USART_puts(USART3, cmd_raw);
		USART_puts(USART3, "\r\n");

		if(cmd_raw->_start == 'c' && cmd_raw->_end == 'e'){
			USART_puts(USART3, "id:");
			USART_putd(USART3, cmd_raw->cmd_id);

			USART_puts(USART3, " value:");
			USART_putd(USART3, cmd_raw->cmd_value);
			USART_puts(USART3, "\r\n");

			processCMD(cmd_raw->cmd_id, cmd_raw->cmd_value);
		}
	}
}

#endif

#if DEBUG_MODE
void receive_task(){
	if(Receive_String_Ready){
		if(received_string[0] == '+'){
			SpeedValue_left += inc;
			SpeedValue_right += inc;
			mMove(SpeedValue_left,SpeedValue_right);
			USART_puts(USART3, "left:");
			USART_putd(USART3, SpeedValue_left);
			USART_puts(USART3, " right:");
			USART_putd(USART3, SpeedValue_right);
			USART_puts(USART3, "\r\n");
		}

		else if(received_string[0] == '-'){
			SpeedValue_left -= inc;
			SpeedValue_right -= inc;
			mMove(SpeedValue_left,SpeedValue_right);
			USART_puts(USART3, "left:");
			USART_putd(USART3, SpeedValue_left);
			USART_puts(USART3, " right:");
			USART_putd(USART3, SpeedValue_right);
			USART_puts(USART3, "\r\n");
		}

		else if(received_string[0] == 'e'){
			getEncoder();
		}

		else if(received_string[0] == 'u'){
			if(received_string[1] == 'a'){
			    USART_puts(USART3, "Actu_A_up");
			    set_linearActuator_A_cmd(LINEAR_ACTU_CW);
			    USART_puts(USART3, "\r\n");
			}else if(received_string[1] == 'b'){
			    USART_puts(USART3, "Actu_B_up");
			    set_linearActuator_B_cmd(LINEAR_ACTU_CW);
			    USART_puts(USART3, "\r\n");
			}
		}
		else if(received_string[0] == 'd'){
			if(received_string[1] == 'a'){
			    USART_puts(USART3, "Actu_A_down");
			    //set_linearActuator_A_cmd(LINEAR_ACTU_CCW);
			    USART_puts(USART3, "\r\n");
			    processCMD('x', LINEAR_ACTU_CCW);
			}else if(received_string[1] == 'b'){
			    USART_puts(USART3, "Actu_B_down");
			    //set_linearActuator_B_cmd(LINEAR_ACTU_CCW);
			    USART_puts(USART3, "\r\n");
			    processCMD('y', LINEAR_ACTU_CCW);
			}
		}

		else if(received_string[0] == 'a'){
			mPowerON();
		}
		else if(received_string[0] == 'A'){
			mPowerOFF();
		}
		else if(received_string[0] == 'q'){
			mSwitchON();
		}
		else if(received_string[0] == 'Q'){
			mSwitchOFF();
		}
		else if(received_string[0] == 'c'){
			getCurData();
		}
		//else if(received_string[0] == 's'){
			/* stop */
		//	processCMD('s', '0');
		//}
		else if(received_string[0] == 'f'){
			/* forward */
			processCMD('f', '0');
		}
		else if(received_string[0] == 'b'){
			/* backward */
			processCMD('b', '0');
		}
		else if(received_string[0] == 't'){
			/* check motor status */
			char s;
			s = getIndicator();
			USART_puts(USART3, "status: ");
			USART_putd(USART3, s);
			USART_puts(USART3, "\r\n");
		}

		else if(received_string[0] == 'm'){
			/* motorTest */
			//motorTest();
			motorFNN();
		}
		else if(received_string[0] == 's'){
			motorStop();
		}
		else if(received_string[0] == 'r'){
			endofRecord();
		}

		else{
			USART_puts(USART3, received_string);
			USART_puts(USART3, "\r\n");
		}
	}
}
#endif