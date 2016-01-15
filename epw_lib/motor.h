/* 20150708 wei han for the test of EPW2 */
#ifndef __MOTOR_H__
#define __MOTOR_H__

/* Define all pins of the motor of EPW2 */
#define MOTOR_PWM_PORT				GPIOD

/* The switch of the motor drivers */
#define MOTOR_RELAY_PIN				GPIO_Pin_9

/* The switch to toggle constant voltage and control signal */
#define MOTOR_LEFT_SW_PIN			GPIO_Pin_10
#define MOTOR_RIGHT_SW_PIN			GPIO_Pin_11 

/* Constant Voltage for motor driver initialization */
#define MOTOR_LEFT_CV_PIN			GPIO_Pin_12 //Green  TIM4_CH1
#define MOTOR_RIGHT_CV_PIN			GPIO_Pin_13 //Orange TIM4_CH2

/* Control Signal */
#define MOTOR_LEFT_PWM_PIN			GPIO_Pin_14 //Red  TIM4_CH3
#define MOTOR_RIGHT_PWM_PIN			GPIO_Pin_15 //Blue TIM4_CH4


extern void mPowerON(void);
extern void mPowerOFF(void);
extern void mSwitchON(void);
extern void mSwitchOFF(void);
extern void testMotor(uint32_t SpeedValue_left, uint32_t SpeedValue_right);
extern void init_motor(void);
void init_switch(void);
void init_motorPWM(void);

#endif 
/* __MOTOR_H__ */
