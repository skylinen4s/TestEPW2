#include <stdio.h>
#include "ff.h"
#include "fftest.h"
#include "uart.h"

FATFS fatfs;
FIL file, file2;
BYTE buff[20] = "test write\n\r";
UINT bw, br;
FRESULT res;

int speed[] = { 100, 3000};

FRESULT ff_test(void)
{
    printf("test fatfs begin\n\r");
    SD_NVIC_Configuration();

    res = f_mount(&fatfs, "", 0);
    if(res != FR_OK) printf("mount failed: %d\n\r", res);
    
    res = f_open(&file, "test.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    res = f_open(&file2, "var.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if(res != FR_OK) printf("open failed: %d\n\r", res);
    
    res = f_write(&file, buff, sizeof(buff), &bw);

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
    printf("ff_read begin\n\r");
    res = f_mount(&fatfs, "", 0);
    if(res != FR_OK) printf("mount failed: %d\n\r", res);

    res = f_open(&file, "test.txt", FA_READ);
    res = f_open(&file2, "log.txt", FA_READ);
    if(res != FR_OK) printf("open failed: %d\n\r", res);

    speed[0] = 1;
    speed[1] = 0;
    while(1)
    {
	res = f_read(&file, buff, sizeof(buff), &br);
	res = f_read(&file2, speed, sizeof(speed), &br);
	if(res != FR_OK) printf("read failed: %d\n\r", res);
	if(res || br ==0) break;
	USART_puts(USART3, buff);
	USART_putd(USART3, br);

	int i;
	for(i = 0; i < 20; i++) buff[i] = 0;
    }
    printf("speed:%d, %d\n\r",speed[0], speed[1]);
    res = f_close(&file);
    res = f_close(&file2);
    if(res != FR_OK) printf("close failed: %d\n\r", res);

    res = f_mount(NULL, "", 0);
    if(res != FR_OK) printf("umnout failed: %d\n\r", res);
}
