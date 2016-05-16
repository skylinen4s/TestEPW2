#include <stdio.h>
#include "ff.h"
#include "record.h"
#include "uart.h"
#include "encoder.h"

FATFS fatfs;
FIL file, file2, f_value;
BYTE buff[20] = "test write\n\r";
UINT bw, br;
FRESULT res;

xTimerHandle RecordTimer;
#define RECORD_PERIOD 1000 //ms

extern uint32_t SpeedValue_left;
extern uint32_t SpeedValue_right;
extern Encoder_t ENCODER_L;
extern Encoder_t ENCODER_R;

uint32_t speed[2];
uint32_t encoder[2];

void start_record(){
    SD_NVIC_Configuration();

    /* mount file system */
    res = f_mount(&fatfs, "", 0);
    if(res != FR_OK) printf("mount failed: %d\n\r", res);

    /* open file to write data */
    res = f_open(&file, "data.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    res = f_open(&f_value, "pwm.val", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);

    if(res != FR_OK) printf("open file failed: %d\n\r", res);

    if(res == FR_OK) printf("file is ready to write data!\n\r");
}

void close_record(){
    res = f_close(&file);
    if(res != FR_OK) printf("fclose failled: %d\n\r", res);

    res = f_mount(NULL, "", 0);
    if(res != FR_OK) printf("umonut failed: %d\n\r", res);

    if(res == FR_OK) printf("file closed!");
}

void writeControlValue(uint32_t pwm_L, uint32_t pwm_R){
    if(&f_value){
        res = f_write(&f_value, &pwm_L, sizeof(uint32_t), &bw);
        res = f_write(&f_value, &pwm_R, sizeof(uint32_t), &bw);
    }
    res = f_sync(&f_value);
}

uint32_t readControlValue(){
    uint32_t pwm_value = 595;
    if(&f_value){
        res = f_read(&f_value, &pwm_value, sizeof(uint32_t), &br);
        res = f_sync(&f_value);

        return pwm_value;
    }
    return pwm_value;
}

void recControlData(uint32_t pwm_L, uint32_t pwm_R, int enc_L, int enc_R){
    if(&file){
        res = f_printf(&file, "%d %d %d %d\r\n", pwm_L, pwm_R, enc_L, enc_R);
    }
    //res = f_sync(&file);
    /* f_sync would spend some much time, making timer delayed and data error */
}

/* sync the data after all commands are over */
void endofRecord(){
    res = f_sync(&file);
}

void record(){
    RecordTimer = xTimerCreate("Data Record Polling", (RECORD_PERIOD), pdTRUE, (void *) 1, pwmrecord);
    xTimerStart(RecordTimer, 0);
}

void pwmrecord(){
    speed[0] = SpeedValue_left;
    speed[1] = SpeedValue_right;
    encoder[0] = ENCODER_L.count;
    encoder[1] = ENCODER_R.count;
    if(&file){
        res = f_write(&file, speed, sizeof(speed), &bw);
        res = f_write(&file, encoder, sizeof(encoder), &bw);
    }
    res = f_sync(&file);
}

FRESULT ff_test(void)
{
    printf("test fatfs begin\n\r");
    SD_NVIC_Configuration();

    res = f_mount(&fatfs, "", 0);
    if(res != FR_OK) printf("mount failed: %d\n\r", res);
    
    res = f_open(&file, "test.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    res = f_open(&file2, "var.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if(res != FR_OK) printf("open failed: %d\n\r", res);
    
    res = f_write(&file, buff, 12 * sizeof(char), &bw);
    res = f_printf(&file, "test-");
    res = f_printf(&file, "speed:%d,%d\n\r",speed[0],speed[1]--);
    res = f_puts("end of file\n\r", &file);

    speed[0] = 65;
    speed[1] = 1;
    res = f_write(&file2, speed, sizeof(speed), &bw);
    if(res != FR_OK) printf("fwrite failed: %d\n\r", res);
    
    res = f_close(&file);
    res = f_close(&file2);
    if(res != FR_OK) printf("fclose failled: %d\n\r", res);

    res = f_mount(NULL,"", 0);
    if(res != FR_OK) printf("umonut failed: %d\n\r", res);
    
    if(res == FR_OK) printf("test fatfs success\n\r");
}

FRESULT ff_read(void)
{
    char line[20];

    printf("ff_read begin\n\r");
    res = f_mount(&fatfs, "", 0);
    if(res != FR_OK) printf("mount failed: %d\n\r", res);

    res = f_open(&file, "test.txt", FA_READ);
    res = f_open(&file2, "var.txt", FA_READ);
    if(res != FR_OK) printf("open failed: %d\n\r", res);

    speed[0] = 1;
    speed[1] = 0;

    while(1)
    {
	f_gets(line, sizeof(line), &file);
	if(line[0] == 0) break;
	USART_puts(USART3, line);

	int i;
	for(i = 0; i < 20; i++) line[i] = 0;
    }

    res = f_read(&file2, speed, sizeof(speed), &br);
    printf("speed:%d, %d\n\r",speed[0], speed[1]);

    res = f_close(&file);
    res = f_close(&file2);
    if(res != FR_OK) printf("close failed: %d\n\r", res);

    res = f_mount(NULL, "", 0);
    if(res != FR_OK) printf("umnout failed: %d\n\r", res);
}
