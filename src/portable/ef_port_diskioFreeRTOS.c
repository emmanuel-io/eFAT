/**
 * ********************************************************************************************************************
 *  @file     ef_port_diskioFreeRTOS.c
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
 *  eFAT - embedded FAT Filesystem module
 * ********************************************************************************************************************
 *
 *  Copyright (C) 2021, Amadio Emmanuel, all right reserved.
 *  Copyright (C) 2019, ChaN, all right reserved.
 *
 *  eFAT module is an open source software. Redistribution and use of eFAT in source and binary forms, with or without
 *  modification, are permitted provided that the following condition is met:
 *
 *  Redistributions of source code must retain the above copyright notice, this condition and the following disclaimer.
 *
 *  This software is provided by the copyright holders and contributors "AS IS" and any warranties related to this
 *  software are DISCLAIMED.
 *  The copyright owners or contributors be NOT LIABLE for any damages caused by use of this software.
 * ********************************************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "efat.h"
#include <ef_port_memory.h>
#include "ef_port_diskio.h"
#include "bsp_driver_sd.h"

#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "semphr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/**
 *  @brief  Initialize a Drive
 *
 *  @return Status of Disk Functions
 */
ef_return_et eEFPortDriveSDIOInitialize (
    void
);

/**
 *  @brief  Get Drive Status
 *
 *  @return Status of Disk Functions
 */
ef_return_et eEFPortDriveSDIOStatus (
  void
  );

/**
 *  @brief  Read Sector(s)
 *
 *  @param  pu8Buffer   Pointer to the data buffer to store read data
 *  @param  xSector     Start xSector in LBA
 *  @param  u32Count     Number of sectors to read
 *
 *  @return Results of Disk Functions
 *  @retval EF_RET_OK      Successful
 *  @retval EF_RET_DISK_ERROR   R/W Error
 *  @retval EF_RET_DISK_WRPRT   Write Protected
 *  @retval EF_RET_DISK_NOTRDY  Not Ready
 *  @retval EF_RET_DISK_PARERR  Invalid Parameter
 */
ef_return_et eEFPortDriveSDIORead (
  ef_u08_t  * pu8Buffer,
  ef_lba_t    xSector,
  ef_u32_t    u32Count
);

/**
 *  @brief  Write Sector(s)
 *
 *  @param  pu8Buffer   Pointer to the data to be written
 *  @param  xSector     Start xSector in LBA
 *  @param  u32Count     Number of sectors to write
 *
 *  @return Results of Disk Functions
 *  @retval EF_RET_OK      Successful
 *  @retval EF_RET_DISK_ERROR   R/W Error
 *  @retval EF_RET_DISK_WRPRT   Write Protected
 *  @retval EF_RET_DISK_NOTRDY  Not Ready
 *  @retval EF_RET_DISK_PARERR  Invalid Parameter
 */
ef_return_et eEFPortDriveSDIOWrite (
  const ef_u08_t  * pu8Buffer,
  ef_lba_t          xSector,
  ef_u32_t          u32Count
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
ef_return_et eEFPortDriveSDIOCtrl (
  ef_u08_t u8Cmd,
  void *  pvBuffer
);

/* Public functions ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static ef_return_et eEFPortDriveSDIOCheckStatus (
  ef_u32_t timeout
);
/* Private functions ---------------------------------------------------------*/



/**
 *  Drive status
 */
static volatile ef_return_et eSDIOStatus = EF_RET_DISK_NOINIT;
SemaphoreHandle_t xSemaphoreSDRead = NULL;
SemaphoreHandle_t xSemaphoreSDWrite = NULL;
StaticSemaphore_t xSemaphoreBufferSDRead;
StaticSemaphore_t xSemaphoreBufferSDWrite;

/* Private functions ---------------------------------------------------------*/

/*
 * the following Timeout is useful to give the control back to the applications
 * in case of errors in either BSP_SD_ReadCpltCallback() or BSP_SD_WriteCpltCallback()
 * the value by default is as defined in the BSP platform driver otherwise 30 secs
 */
#define EF_PORT_SD_TIMEOUT 30 * 10000

#define EF_PORT_SD_DEFAULT_BLOCK_SIZE 512

/*
 * when using cachable memory region, it may be needed to maintain the cache
 * validity. Enable the define below to activate a cache maintenance at each
 * read and write operation.
 * Notice: This is applicable only for cortex M7 based platform.
 */
/* USER CODE BEGIN enableSDDmaCacheMaintenance */
#define ENABLE_SD_DMA_CACHE_MAINTENANCE  ( 0 )
/* USER CODE END enableSDDmaCacheMaintenance */

/*
* Some DMA requires 4-Byte aligned address buffer to correctly read/wite data,
* in FatFs some accesses aren't thus we need a 4-byte aligned scratch buffer to correctly
* transfer data
*/
/* USER CODE BEGIN enableScratchBuffer */
#define ENABLE_SCRATCH_BUFFER
/* USER CODE END enableScratchBuffer */

/* Private variables ---------------------------------------------------------*/
ALIGN_32BYTES(static ef_u08_t u8ScratchBuffer[ BLOCKSIZE ]); // 32-Byte aligned for cache maintenance
/* Disk status */

static ef_return_et eEFPortDriveSDIOCheckStatus (
  ef_u32_t u32Timeout
);


static ef_return_et eEFPortDriveSDIOCheckStatus (
  ef_u32_t u32Timeout
)
{
  ef_return_et  eRetVal = EF_RET_DISK_NOINIT;

  ef_u32_t      u32Timer = osKernelSysTick( );

  /* block until SDIO peripherial is ready again or a timeout occur */
  while ( osKernelSysTick() - u32Timer < u32Timeout )
  {
    if ( SD_TRANSFER_OK == BSP_SD_GetCardState( )  )
    {
      eRetVal = EF_RET_OK;
      break;
    }
  }
  if ( EF_RET_OK != eRetVal ) { EF_RETURN_CODE_HANDLER( EF_RET_DISK_NOINIT ); }

  return eRetVal;
}

/* Initialize a Drive */
ef_return_et eEFPortDriveSDIOInitialize (
  void
)
{
  /*
   * check that the kernel has been started before continuing
   * as the osMessage API will fail otherwise
   */
  int32_t s32KernelRun = osKernelRunning( );
  if ( 0 != s32KernelRun )
  {
    /*
     * if the SD is correctly initialized, create the operation queue
     * if not already created
     */
    if ( EF_RET_DISK_NOINIT == eSDIOStatus )
    {
      if ( MSD_OK == BSP_SD_Init( ) )
      {
        eSDIOStatus = EF_RET_OK;
      }
    }
    if ( NULL == xSemaphoreSDRead )
    {
      xSemaphoreSDRead = xSemaphoreCreateBinaryStatic( &xSemaphoreBufferSDRead );
//        xSemaphoreGive( xSemaphoreSDRead );
    }
    if ( NULL == xSemaphoreSDWrite )
    {
      xSemaphoreSDWrite = xSemaphoreCreateBinaryStatic( &xSemaphoreBufferSDWrite );
//        xSemaphoreGive( xSemaphoreSDWrite );
    }
    if (    ( NULL == xSemaphoreSDRead )
         || ( NULL == xSemaphoreSDWrite ) )
    {
      eSDIOStatus = EF_RETURN_CODE_HANDLER( EF_RET_DISK_NOINIT );
    }
  }

  return eSDIOStatus;
}

/* Get Drive Status */
ef_return_et eEFPortDriveSDIOStatus (
  void
)
{

  if ( SD_TRANSFER_OK == BSP_SD_GetCardState( ) )
  {
    eSDIOStatus = EF_RET_OK;
  }
  else
  {
    eSDIOStatus = EF_RETURN_CODE_HANDLER( EF_RET_DISK_NOINIT );
  }


  return eSDIOStatus;
}

/* Read Sector(s)*/
ef_return_et eEFPortDriveSDIORead (
  ef_u08_t * pu8Buffer,
  ef_lba_t    xSector,
  ef_u32_t   u32Count
)
{
  ef_return_et eRetVal = EF_RET_OK;
  /*
   * ensure the SDCard is ready for a new operation
   */
  if ( EF_RET_OK != eEFPortDriveSDIOCheckStatus(EF_PORT_SD_TIMEOUT) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERROR );
  }
  else
  {
    /* Slow path, fetch each xSector a part and memcpy to destination buffer */
    ef_u32_t  i;
    ef_u08_t ret = MSD_ERROR;

    for ( i = u32Count ; i != 0 ; i-- )
    {
      xSemaphoreTake( xSemaphoreSDRead, ( TickType_t ) 0 );

//      if  (    ( MSD_OK == BSP_SD_ReadBlocks_DMA( (ef_u32_t*)scratch, (ef_u32_t)xSector++, 1 ) )
//          && ( pdTRUE == xSemaphoreTake( xSemaphoreSDRead, ( TickType_t ) EF_PORT_SD_TIMEOUT ) )
//          && ( MSD_OK == BSP_SD_GetCardState( ) )
//        )
      ret = MSD_ERROR;
      if  ( MSD_OK != BSP_SD_ReadBlocks_DMA( (ef_u32_t*)u8ScratchBuffer, (ef_u32_t)xSector++, 1 ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERROR );
        break;
      }
      else if  ( pdTRUE != xSemaphoreTake( xSemaphoreSDRead, ( TickType_t ) EF_PORT_SD_TIMEOUT ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_TIMEOUT_RD );
        break;
      }
      else if  ( 0 != eEFPortDriveSDIOCheckStatus(EF_PORT_SD_TIMEOUT) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERROR );
        break;
      }
      else
      {
        if ( 0 != ENABLE_SD_DMA_CACHE_MAINTENANCE )
        {
          /*
           *
           * invalidate the scratch buffer before the next read to get
           * the actual data instead of the cached one
           */
          SCB_InvalidateDCache_by_Addr((ef_u32_t*)u8ScratchBuffer, BLOCKSIZE);
        }
        eEFPortMemCopy( u8ScratchBuffer, pu8Buffer, BLOCKSIZE );
        pu8Buffer += BLOCKSIZE;

        ret = MSD_OK;
      }
    }

    if (    ( 0 == i )
         && ( MSD_OK == ret ) )
    {
      eRetVal = EF_RET_OK;
    }
    else
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERROR );
    }
  }

  return eRetVal;
}

/* Write Sector(s) */
ef_return_et eEFPortDriveSDIOWrite (
  const ef_u08_t  * pu8Buffer,
  ef_lba_t          xSector,
  ef_u32_t          u32Count
)
{
  ef_return_et eRetVal = EF_RET_OK;

  /*
   * ensure the SDCard is ready for a new operation
   */
  if ( EF_RET_OK != eEFPortDriveSDIOCheckStatus(EF_PORT_SD_TIMEOUT) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERROR );
  }
  else
  {
    xSemaphoreTake( xSemaphoreSDWrite, ( TickType_t ) 0 );

    /* Slow path, fetch each xSector a part and memcpy to destination buffer */
    for ( ef_u32_t  i = u32Count ; i != 0 ; i-- )
    {
      if ( 0 != ENABLE_SD_DMA_CACHE_MAINTENANCE )
      {
        /*
         * invalidate the scratch buffer before the next write
         * to get the actual data instead of the cached one
         */
        SCB_InvalidateDCache_by_Addr( (ef_u32_t*)u8ScratchBuffer, BLOCKSIZE );
      }
      eEFPortMemCopy( pu8Buffer, u8ScratchBuffer, BLOCKSIZE);
      pu8Buffer += BLOCKSIZE;
      if ( MSD_OK != BSP_SD_WriteBlocks_DMA( (ef_u32_t*)u8ScratchBuffer, (ef_u32_t)xSector++, 1 ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERROR );
        break;
      }
      else if ( pdTRUE != xSemaphoreTake( xSemaphoreSDWrite, ( TickType_t ) EF_PORT_SD_TIMEOUT ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_TIMEOUT_WR );
        break;
      }
      else if ( 0 != eEFPortDriveSDIOCheckStatus(EF_PORT_SD_TIMEOUT) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERROR );
        break;
      }
      else if ( 1 == i )
      {
        eRetVal = EF_RET_OK;
        break;
      }
      else
      {
        /* Keep transfering */;
        EF_CODE_COVERAGE( );
      }
    }
  }

  return eRetVal;
}

/* Miscellaneous Functions */

ef_return_et eEFPortDriveSDIOCtrl (
  ef_u08_t   u8Cmd,
  void      * pvBuffer
)
{
  ef_return_et eRetVal = EF_RET_DISK_ERROR;
  BSP_SD_CardInfo CardInfo;

  if ( EF_RET_DISK_NOINIT == eSDIOStatus )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_NOTRDY );
  }
  else
  {
    switch ( u8Cmd )
    {
      /* Make sure that no pending write process */
      case CTRL_SYNC :
        eRetVal = EF_RET_OK;
        break;

      /* Get number of sectors on the disk (DWORD) */
      case GET_SECTOR_COUNT :
        BSP_SD_GetCardInfo( &CardInfo );
        *(ef_u32_t*)pvBuffer = CardInfo.LogBlockNbr;
        eRetVal = EF_RET_OK;
        break;

      /* Get R/W xSector size (WORD) */
      case GET_SECTOR_SIZE :
        BSP_SD_GetCardInfo( &CardInfo );
        *(ef_u16_t*)pvBuffer = (ef_u16_t) CardInfo.LogBlockSize;
        eRetVal = EF_RET_OK;
        break;

      /* Get erase block size in unit of xSector (DWORD) */
      case GET_BLOCK_SIZE :
        BSP_SD_GetCardInfo( &CardInfo );
        *(ef_u32_t*)pvBuffer = CardInfo.LogBlockSize;
        eRetVal = EF_RET_OK;
        break;

      default:
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_PARERR );
    }
  }

  return eRetVal;
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void BSP_SD_ReadCpltCallback(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if ( pdTRUE != xSemaphoreGiveFromISR( xSemaphoreSDRead, &xHigherPriorityTaskWoken ) )
  {
    for (;;);
  }
  /* If xHigherPriorityTaskWoken was set to true we should yield.
   * The actual macro used here is port specific.
   */
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void BSP_SD_WriteCpltCallback(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if ( pdTRUE != xSemaphoreGiveFromISR( xSemaphoreSDWrite, &xHigherPriorityTaskWoken ) )
  {
    for (;;);
  }
  /* If xHigherPriorityTaskWoken was set to true we should yield.
   * The actual macro used here is port specific.
   */
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
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

