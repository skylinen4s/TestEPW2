#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "math.h"
#include "stdio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
//#include "stm32f4xx_conf.h"
//#include "stm32f4_discovery.h"
#include "uart.h"
#include "motor.h"
//#include "transfer.h"


void init_UserButton(void);
void init_LED(void);

void write()
{ 
    while(1){
        //USART_puts(USART3,"write1\n\r");
        GPIO_SetBits(GPIOD, GPIO_Pin_12);
        vTaskDelay(2000);
        GPIO_ResetBits(GPIOD, GPIO_Pin_12);
        vTaskDelay(2000);
    }
   
}
static void servo_test(void *pvParameters)
{ 
    int x = 60, y = 60;
    while(1)
    { 
        //USART_puts(USART3,"servo2\n\r");
        GPIO_SetBits(GPIOD, GPIO_Pin_13);
        vTaskDelay(1000);

        x+=5;
        y+=5;
        GPIO_ResetBits(GPIOD, GPIO_Pin_13);
        vTaskDelay(1000);
    }
}

void initialize()
{ 
    init_USART3(9600);
    USART_puts(USART3,"initial...\n\r");
    USART_puts(USART3,"USART is ready\n\r");
    //init_UserButton();
    init_LED();
    init_motor();
    USART_puts(USART3,"motor is ready \n\r");
    /* Enable USART interrupt */
    //USART3_Interrupt();
    init_encoder();
    init_encoder_exti();
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    initialize();

    xTaskCreate(servo_test, "servo_test", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
    xTaskCreate(write, "AutoWrite", configMINIMAL_STACK_SIZE, NULL, 2, NULL );

    vTaskStartScheduler();

    while(1);
}

void vApplicationTickHook(void) {
}

/* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created.  It is also called by various parts of the
   demo application.  If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
void vApplicationMallocFailedHook(void) {
    taskDISABLE_INTERRUPTS();
    for(;;);
}

/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
   task.  It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()).  If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
void vApplicationIdleHook(void) {
}

void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName) {
    (void) pcTaskName;
    (void) pxTask;
    /* Run time stack overflow checking is performed if
     configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
     function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for(;;);
}
/*
void init_UserButton(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init( GPIOA, &GPIO_InitStruct );

    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}*/

void init_LED(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    /* Enable GPIO D clock. */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    // Setup Blue & Green LED on STM32-Discovery Board to use PWM.
    GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15; //PD12->LED3 PD13->LED4 PD14->LED5 PDa5->LED6
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init( GPIOD, &GPIO_InitStruct ); 

    GPIO_WriteBit(GPIOD,GPIO_Pin_12,Bit_RESET);//Green
    GPIO_WriteBit(GPIOD,GPIO_Pin_13,Bit_RESET);//Orange
    GPIO_WriteBit(GPIOD,GPIO_Pin_14,Bit_RESET);//Red
    GPIO_WriteBit(GPIOD,GPIO_Pin_15,Bit_RESET);//Blue
}

#if 0
void EXTI0_IRQHandler(void)
{ 
    if (EXTI_GetFlagStatus(EXTI_Line0) == SET)
    {
    GPIO_SetBits(GPIOD,GPIO_Pin_15);
    riseUp(NO);
    }

    //GPIO_ResetBits(GPIOD,GPIO_Pin_15);
    //USART3_Printf("UserButton\n\r");
    // Clear the EXTI line pending bit //
    EXTI_ClearITPendingBit(EXTI_Line0);
}
#endif
