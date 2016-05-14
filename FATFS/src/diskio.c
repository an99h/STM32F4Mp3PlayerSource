
#include "usb_conf.h"
#include "diskio.h"
#include "usbh_msc_core.h"
#include "sd.h"
/*--------------------------------------------------------------------------

Module Private Functions and Variables

---------------------------------------------------------------------------*/

static volatile DSTATUS Stat = STA_NOINIT;	/* Disk status */

extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern USBH_HOST                     USB_Host;

#define USB_DISK 0	//USB,卷标为0
#define SD_CARD	 1  //SD卡,卷标为1


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE drv		/* Physical drive number (0) */
)
{
	u8 res=0;
	switch(drv)
	{
	case SD_CARD: //SD卡
		res = SD_Initialize();//SD_Initialize()
		if(res)//STM32 SPI的bug,在sd卡操作失败的时候如果不执行下面的语句,可能导致SPI读写异常
		{
			SD_SPI_SpeedLow();
			SD_SPI_ReadWriteByte(0xff);//提供额外的8个时钟
			SD_SPI_SpeedHigh();
		}
		break;
	case USB_DISK:
		if(HCD_IsDeviceConnected(&USB_OTG_Core))
		{
			Stat &= ~STA_NOINIT;
		}
		return Stat;
	}
	if(res)
		return  STA_NOINIT;
	else
		return 0; //初始化成功
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE drv		/* Physical drive number (0) */
)
{
	switch(drv)
	{
	case USB_DISK:
		if (drv) return STA_NOINIT;		/* Supports only single drive */
		return Stat;
	case SD_CARD:
		break;
	default:
		break;
	}
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE drv,			/* Physical drive number (0) */
    BYTE *buff,			/* Pointer to the data buffer to store read data */
    DWORD sector,		/* Start sector number (LBA) */
    BYTE count			/* Sector count (1..255) */
)
{
	BYTE status = USBH_MSC_OK;
	u8 res=0;
	if (!count)return RES_PARERR;//count不能等于0，否则返回参数错误
	switch(drv)
	{
	case USB_DISK:
		if (drv || !count) return RES_PARERR;
		if (Stat & STA_NOINIT) return RES_NOTRDY;
		if(HCD_IsDeviceConnected(&USB_OTG_Core))
		{
			do
			{
				status = USBH_MSC_Read10(&USB_OTG_Core, buff, sector, 512*count);
				USBH_MSC_HandleBOTXfer(&USB_OTG_Core ,&USB_Host);

				if(!HCD_IsDeviceConnected(&USB_OTG_Core))
				{
					return RES_ERROR;
				}
			}
			while(status == USBH_MSC_BUSY );
		}
		if(status == USBH_MSC_OK)
			return RES_OK;
		return RES_ERROR;
		
	case SD_CARD: //SD卡
		res=SD_ReadDisk(buff,sector,count);
		if(res)//STM32 SPI的bug,在sd卡操作失败的时候如果不执行下面的语句,可能导致SPI读写异常
		{
			SD_SPI_SpeedLow();
			SD_SPI_ReadWriteByte(0xff);//提供额外的8个时钟
			SD_SPI_SpeedHigh();
		}
		break;
	default:
		res = 1;
		break;
	}
	if(res==0x00)
		return RES_OK;
	else
		return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT disk_write (
    BYTE drv,			/* Physical drive number (0) */
    const BYTE *buff,	/* Pointer to the data to be written */
    DWORD sector,		/* Start sector number (LBA) */
    BYTE count			/* Sector count (1..255) */
)
{
	BYTE status = USBH_MSC_OK;
	u8 res=0;
	if (!count)return RES_PARERR;//count不能等于0，否则返回参数错误
	switch(drv)
	{
	case USB_DISK:
		if (drv || !count) return RES_PARERR;
		if (Stat & STA_NOINIT) return RES_NOTRDY;
		if (Stat & STA_PROTECT) return RES_WRPRT;
		if(HCD_IsDeviceConnected(&USB_OTG_Core))
		{
			do
			{
				status = USBH_MSC_Write10(&USB_OTG_Core,(BYTE*)buff, sector, 512*count);
				USBH_MSC_HandleBOTXfer(&USB_OTG_Core, &USB_Host);

				if(!HCD_IsDeviceConnected(&USB_OTG_Core))
				{
					return RES_ERROR;
				}
			}

			while(status == USBH_MSC_BUSY );

		}
		if(status == USBH_MSC_OK)
			return RES_OK;
		return RES_ERROR;
	case SD_CARD: //SD卡
		res=SD_WriteDisk((u8*)buff,sector,count);
		break;
	default:
		res=1;
		break;

	}
	if(res == 0x00)return RES_OK;
	else return RES_ERROR;
}
#endif /* _READONLY == 0 */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL != 0
DRESULT disk_ioctl (
    BYTE drv,		/* Physical drive number (0) */
    BYTE ctrl,		/* Control code */
    void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_OK;


	if(drv == USB_DISK)
	{
		if (drv) return RES_PARERR;

		res = RES_ERROR;

		if (Stat & STA_NOINIT) return RES_NOTRDY;

		switch (ctrl)
		{
		case CTRL_SYNC :		/* Make sure that no pending write process */

			res = RES_OK;
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */

			*(DWORD*)buff = (DWORD) USBH_MSC_Param.MSCapacity;
			res = RES_OK;
			break;

		case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
			*(WORD*)buff = 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */

			*(DWORD*)buff = 512;

			break;


		default:
			res = RES_PARERR;
		}
	}
	else if(drv == SD_CARD)
	{
		switch(ctrl)
		{
		case CTRL_SYNC:
			SD_CS=0;
			if(SD_WaitReady()==0)
				res = RES_OK;
			else res = RES_ERROR;
			SD_CS=1;
			break;
		case GET_SECTOR_SIZE:
			*(WORD*)buff = 512;
			res = RES_OK;
			break;
		case GET_BLOCK_SIZE:
			*(WORD*)buff = 8;
			res = RES_OK;
			break;
		case GET_SECTOR_COUNT:
			*(DWORD*)buff = SD_GetSectorCount();
			res = RES_OK;
			break;
		default:
			res = RES_PARERR;
			break;
		}
	}
	else res=RES_ERROR;//其他的不支持


	return res;
}
#endif /* _USE_IOCTL != 0 */
