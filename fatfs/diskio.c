/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "sdio_debug.h"	/* SDIO configuration */
#include "stm32f4_discovery_sdio_sd.h"

/* Definitions of physical drive number for each drive */
#define SD		0	/* Map SD card to physical drive 0 */

#define BLOCK_SIZE 512

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	if(pdrv) return STA_NOINIT;

	#if 0
	switch(SD_GetState())
	{
		case SD_CARD_READY:
			return RES_OK;
		case SD_CARD_IDENTIFICATION:
		case SD_CARD_STANDBY:
		case SD_CARD_TRANSFER:
		case SD_CARD_SENDING:
		case SD_CARD_RECEIVING:
		case SD_CARD_PROGRAMMING:
		case SD_CARD_DISCONNECTED:
			return RES_NOTRDY;
		case SD_CARD_ERROR:
		default:
			return RES_ERROR;
	}

	return STA_NODISK;
	#endif
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	SD_Error status;

	if(pdrv == SD) /* if pdrv == SD(0) */
	{
		status = SD_Init();
		if(status == SD_OK) return 0;
		else if(status == SD_DATA_CRC_FAIL) return STA_NODISK;
		else return STA_NOINIT;
	}

	return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	SD_Error status;

	if(pdrv == SD)
	{
		if(count != 1)
		{
			status = SD_ReadBlock(buff, sector*BLOCK_SIZE, BLOCK_SIZE) ;
		}
		else if(count<=47)
		{
			status = SD_ReadMultiBlocks(buff, sector*BLOCK_SIZE, BLOCK_SIZE, count);
			//Number of blocks (counts) exceeds '47' cant work correctly.
		}
		else
		{
			return RES_PARERR;
		}
		if(status != SD_OK) return RES_ERROR;

		#ifdef SD_DMA_MODE
		status = SD_WaitReadOperation();
		if(status != SD_OK) return RES_ERROR;

		#endif

		while(SD_GetStatus() != SD_TRANSFER_OK);
		if(status == SD_OK) return RES_OK;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	SD_Error status;

	if(pdrv == SD)
	{
		if(count == 1)
		{
			status = SD_WriteBlock(buff, sector*BLOCK_SIZE, BLOCK_SIZE);
		}
		else if(count<=47)
		{
			status = SD_WriteMultiBlocks(buff, sector*BLOCK_SIZE, BLOCK_SIZE, count);
			//Number of blocks (counts) exceeds '47' cant work correctly.
		}
		else
		{
			return RES_PARERR;
		}
		if(status != SD_OK) return RES_ERROR;

		#ifdef SD_DMA_MODE
		status = SD_WaitWriteOperation();
		if(status != SD_OK) return RES_ERROR;
		#endif

		while(SD_GetStatus() != SD_TRANSFER_OK);
		if(status == SD_OK) return RES_OK;
	}

	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;

	if(pdrv == SD)
	{
		switch(cmd)
		{
			case CTRL_SYNC:
				while(SDIO_GetResponse(SDIO_RESP1) == 0);
				res = RES_OK;
				break;
			case GET_SECTOR_COUNT:
				*(DWORD*)buff = SDCardInfo.CardCapacity >> 10;
				// Capacity / 1024 (bytes)
				res = RES_OK;
				break;
			case GET_SECTOR_SIZE:
				*(DWORD*)buff = SDCardInfo.SD_csd.DeviceSize;
				// DeviceSize = SectorSize , (this value+1) * 512 = Capacity (bytes)
				res = RES_OK;
				break;
			case GET_BLOCK_SIZE:
				*(DWORD*)buff = SDCardInfo.CardBlockSize;
				res = RES_OK;
				break;
			default:
				res = RES_PARERR;
				break;
		}
		return res;
	}

	return RES_PARERR;
}
#endif

/*-----------------------------------------------------------------------*/
/* User defined functions to give a current time to fatfs module         */
/* 31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31)     */
/* 15-11: Houre(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2)          */
/*-----------------------------------------------------------------------*/
DWORD get_fattime(void)
{
	return ((2015UL-1980) << 25)	//Year = 2006
			| (12UL << 21)			//Month = Feb
			| (02UL << 16)			//Day = 9
			| (16U << 11)			//Hour = 22
			| (00U << 5)			//Min = 30
			| (00U >> 1)			//Sec = 0
			;
}
