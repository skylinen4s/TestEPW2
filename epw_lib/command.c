#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "stm32f4xx_usart.h"
#include "uart.h"
#include "command.h"
#include "motor.h"
#include "linear_actuator.h"
#include "control.h"
/*parse*/
#include "string.h"

extern uint32_t SpeedValue_left;
extern uint32_t SpeedValue_right;
uint32_t inc = 1;

struct receive_cmd_list * receive_cmd_type;

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
			    set_linearActuator_A_cmd(LINEAR_ACTU_CCW);
			    USART_puts(USART3, "\r\n");
			}else if(received_string[1] == 'b'){
			    USART_puts(USART3, "Actu_B_down");
			    set_linearActuator_B_cmd(LINEAR_ACTU_CCW);
			    USART_puts(USART3, "\r\n");
			}
		}

		else if(received_string[0] == 'a'){
			mPowerON();
		}
		else if(received_string[0] == 'A'){
			mPowerOFF();
		}
		else if(received_string[0] == 'b'){
			mSwitchON();
		}
		else if(received_string[0] == 'B'){
			mSwitchOFF();
		}
		else if(received_string[0] == 'c'){
			getCurData();

		}
		else if(received_string[0] == 'f'){
			test_forward();
		}
		else if(received_string[0] == 't'){
			check();
		}

		else{
			USART_puts(USART3, received_string);
			USART_puts(USART3, "\r\n");
		}
	}
}
#endif