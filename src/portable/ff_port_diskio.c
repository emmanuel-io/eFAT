#if 0
#include "bsp_driver_sd.h"
/**
 * ********************************************************************************************************************
 *  @file     ef_port_diskio.c
 *  @ingroup  group_eFAT_Portable
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Code file for Low level disk interface.
 *
 *  @note     This is a fork of Fatfs module 0.14 by ChaN, all right reserved.
 *            If a working storage control module is available, it should be
 *            attached to the eFAT via a glue function rather than modifying it.
 *            This is an example of glue functions to attach various exsisting
 *            storage control modules to the eFAT module with a defined API.
 *
 * ********************************************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "efat.h"
#include "ef_port_diskio.h"
//#include "bsp_driver_sd.h"
#include "bsp_driver_sd.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/**
 *  @brief  Initialize a Drive
 *
 *  @return Status of Disk Functions
 */
ef_u08_t eEFPortDriveSDIOInitialize (
    void
);

/**
 *  @brief  Get Drive Status
 *
 *  @return Status of Disk Functions
 */
ef_u08_t eEFPortDriveSDIOStatus (
  void
  );

/**
 *  @brief  Read Sector(s)
 *
 *  @param  pu8Buffer   Pointer to the data buffer to store read data
 *  @param  xSector     Start sector in LBA
 *  @param  uiCount     Number of sectors to read
 *
 *  @return Results of Disk Functions
 *  @retval EF_RET_OK      Successful
 *  @retval EF_RET_DISK_ERROR   R/W Error
 *  @retval EF_RET_DISK_WRPRT   Write Protected
 *  @retval EF_RET_DISK_NOTRDY  Not Ready
 *  @retval EF_RET_DISK_PARERR  Invalid Parameter
 */
ef_u08_t eEFPortDriveSDIORead (
  ef_u08_t * pu8Buffer,
  ef_lba_t     xSector,
  ef_u32_t      uiCount
);

/**
 *  @brief  Write Sector(s)
 *
 *  @param  pu8Buffer   Pointer to the data to be written
 *  @param  xSector     Start sector in LBA
 *  @param  uiCount     Number of sectors to write
 *
 *  @return Results of Disk Functions
 *  @retval EF_RET_OK      Successful
 *  @retval EF_RET_DISK_ERROR   R/W Error
 *  @retval EF_RET_DISK_WRPRT   Write Protected
 *  @retval EF_RET_DISK_NOTRDY  Not Ready
 *  @retval EF_RET_DISK_PARERR  Invalid Parameter
 */
ef_u08_t eEFPortDriveSDIOWrite (
  const ef_u08_t * pu8Buffer,
  ef_lba_t           xSector,
  ef_u32_t            uiCount
);

/**
 *  @brief  Miscellaneous Functions
 *
 *  @param  u8Cmd       Control code
 *  @param  pvBuffer    Buffer to send/receive control data
 *
 *  @return Results of Disk Functions
 *  @retval EF_RET_OK      Successful
 *  @retval EF_RET_DISK_ERROR   R/W Error
 *  @retval EF_RET_DISK_WRPRT   Write Protected
 *  @retval EF_RET_DISK_NOTRDY  Not Ready
 *  @retval EF_RET_DISK_PARERR  Invalid Parameter
 */
ef_u08_t eEFPortDriveSDIOCtrl (
  ef_u08_t u8Cmd,
  void *  pvBuffer
);

/* Public functions ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 *  Drive status
 */
static volatile ef_u08_t u8SDIOStatus = 0x00;

/* Private functions ---------------------------------------------------------*/

/* Initialize a Drive */
ef_u08_t eEFPortDriveSDIOInitialize (
  void
)
{
  u8SDIOStatus |= EF_RET_DISK_NOINIT;

  /* Configure the uSD device */
  if ( MSD_OK == BSP_SD_Init( ) )
  {
    u8SDIOStatus &= ~EF_RET_DISK_NOINIT;
  }

  return u8SDIOStatus;
}

/* Get Drive Status */
ef_u08_t eEFPortDriveSDIOStatus (
  void
)
{
  u8SDIOStatus != EF_RET_DISK_NOINIT;

  if ( MSD_OK == BSP_SD_GetCardState( ) )
  {
    u8SDIOStatus &= ~EF_RET_DISK_NOINIT;
  }

  return u8SDIOStatus;
}

/* Read Sector(s)*/
ef_u08_t eEFPortDriveSDIORead (
  ef_u08_t * pu8Buffer,
  ef_lba_t     xSector,
  ef_u32_t      uiCount
)
{
  ef_u08_t u8RetVal = EF_RET_DISK_ERROR;
  ef_u32_t timeout = 100000;
          // 100000 000
  if ( MSD_OK == BSP_SD_ReadBlocks( (ef_u32_t*)pu8Buffer,
                                    (ef_u32_t) (xSector),
                                    uiCount,
                                    SD_DATATIMEOUT ) )
  {
    while ( MSD_OK != BSP_SD_GetCardState( ) )
    {
      u8RetVal = EF_RET_OK;
      if ( timeout-- == 0 )
      {
        u8RetVal = EF_RET_DISK_ERROR;
        break;
      }
    }
  }

  return u8RetVal;
}

/* Write Sector(s) */
ef_u08_t eEFPortDriveSDIOWrite (
  const ef_u08_t * pu8Buffer,
  ef_lba_t           xSector,
  ef_u32_t            uiCount
)
{
  ef_u08_t u8RetVal = EF_RET_DISK_ERROR;
  ef_u32_t timeout = 100000;

  if ( MSD_OK == BSP_SD_WriteBlocks(  (ef_u32_t*) pu8Buffer,
                                      (ef_u32_t) (xSector),
                                      uiCount,
                                      SD_DATATIMEOUT ) )
  {
    while ( MSD_OK != BSP_SD_GetCardState( ) )
    {
      u8RetVal = EF_RET_OK;
      if ( timeout-- == 0 )
      {
        u8RetVal = EF_RET_DISK_ERROR;
        break;
      }
    }
  }

  return u8RetVal;
}

/* Miscellaneous Functions */

ef_u08_t eEFPortDriveSDIOCtrl (
  ef_u08_t u8Cmd,
  void *  pvBuffer
)
{
  ef_u08_t u8RetVal = EF_RET_DISK_ERROR;
  BSP_SD_CardInfo CardInfo;

  if ( 0 != ( EF_RET_DISK_NOINIT & u8SDIOStatus ) )
  {
    u8RetVal = EF_RET_DISK_NOTRDY;
  }
  else
  {
    switch ( u8Cmd )
    {
      /* Make sure that no pending write process */
      case CTRL_SYNC :
        u8RetVal = EF_RET_OK;
        break;

      /* Get number of sectors on the disk (DWORD) */
      case GET_SECTOR_COUNT :
        BSP_SD_GetCardInfo( &CardInfo );
        *(ef_u32_t*)pvBuffer = CardInfo.LogBlockNbr;
        u8RetVal = EF_RET_OK;
        break;

      /* Get R/W sector size (WORD) */
      case GET_SECTOR_SIZE :
        BSP_SD_GetCardInfo( &CardInfo );
        *(ef_u16_t*)pvBuffer = CardInfo.LogBlockSize;
        u8RetVal = EF_RET_OK;
        break;

      /* Get erase block size in unit of sector (DWORD) */
      case GET_BLOCK_SIZE :
        BSP_SD_GetCardInfo( &CardInfo );
        *(ef_u32_t*)pvBuffer = CardInfo.LogBlockSize;
        u8RetVal = EF_RET_OK;
        break;

      default:
        u8RetVal = EF_RET_DISK_PARERR;
    }
  }

  return u8RetVal;
}

/* Public variables ----------------------------------------------------------*/

/**
 *  @brief  SD Card IO Drive Functions
 */
ef_drive_functions_st xffDriveFunctionsSDIO = {
    /* Pointer to function to Initialize Drive */
    .pxInitialize  = eEFPortDriveSDIOInitialize,
    /* Pointer to function to Get Disk Status */
    .pxStatus      = eEFPortDriveSDIOStatus,
    /* Pointer to function to Read Sector(s) */
    .pxRead        = eEFPortDriveSDIORead,
    /* Pointer to function to Write Sector(s) */
    .pxWrite       = eEFPortDriveSDIOWrite,
    /* Pointer to function to I/O control operation */
    .pxCtrl        = eEFPortDriveSDIOCtrl,
};

#endif
