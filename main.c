#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"

#include "FreeRTOS.h"
#include "task.h"

#include "math.h"
#include "stdio.h"

/* EPW_libs */
#include "uart.h"
#include "motor.h"
#include "linear_actuator.h"

#include "record.h"
#include "control.h"

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
    /* ---------communication-------- */
    init_USART3(9600);
    USART_puts(USART3,"initial...\n\r");
    USART_puts(USART3,"USART is ready\n\r");

    /* ------------motors------------ */
    init_motor();
    init_linear_actuator();
    USART_puts(USART3,"motor is ready \n\r");

    /* ------------sensor------------ */
    init_encoder();
    init_CurTransducer();
    init_Indicator();
    
    /* -----------SD(data)----------- */
    USART_puts(USART3,"test sdio/fat \n\r");
    start_record();
    USART_puts(USART3,"test end \n\r");
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
