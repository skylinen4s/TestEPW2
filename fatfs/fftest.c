#include <stdio.h>
#include "ff.h"
#include "fftest.h"

FATFS fatfs;
FIL file;
BYTE buff[20] = "test write \n";
UINT br;
FRESULT res;

FRESULT ff_test(void)
{
    printf("test fatfs begin\n\r");
    SD_NVIC_Configuration();

    res = f_mount(&fatfs, "", 0);
    if(res != FR_OK) printf("mount failed: %d\n\r", res);
    
    res = f_open(&file, "test.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if(res != FR_OK) printf("open failed: %d\n\r", res);
    
    res = f_puts("test success\r", &file);
    res = f_write(&file, buff, sizeof(buff), &br);
    if(res != FR_OK) printf("fwrite failed: %d\n\r", res);
    
    res = f_close(&file);
    if(res != FR_OK) printf("fclose failled: %d\n\r", res);

    res = f_mount(NULL,"", 0);
    if(res != FR_OK) printf("umonut failed: %d\n\r", res);
    
    if(res == FR_OK)printf("test fatfs success\n\r");
}
