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

/* Private functions ---------------------------------------------------------*/

/*
 * the following Timeout is useful to give the control back to the applications
 * in case of errors in either BSP_SD_ReadCpltCallback() or BSP_SD_WriteCpltCallback()
 * the value by default is as defined in the BSP platform driver otherwise 30 secs
 */
#define EF_PORT_SD_TIMEOUT ( 30 * 10000 )

#define EF_PORT_SD_DEFAULT_BLOCK_SIZE 512

/* Private variables ---------------------------------------------------------*/
/* Disk status */

static ef_return_et eEFPortDriveSDIOCheckStatus (
  ef_u32_t u32Timeout
);


static ef_return_et eEFPortDriveSDIOCheckStatus (
  ef_u32_t u32Timeout
)
{
  ef_return_et  eRetVal = EF_RET_OK;


  /* Get card status */
  if ( SD_TRANSFER_OK != BSP_SD_GetCardState( )  )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_NOINIT );
  }
  else
  {
    /* Keep transfering */;
    EF_CODE_COVERAGE( );
  }

  return eRetVal;
}

/* Initialize a Drive */
ef_return_et eEFPortDriveSDIOInitialize (
  void
)
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
      else
      {
        eSDIOStatus = EF_RETURN_CODE_HANDLER( EF_RET_DISK_NOINIT );
      }
    }
    else
    {
      /* Keep transfering */;
      EF_CODE_COVERAGE( );
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

  if ( MSD_OK != BSP_SD_ReadBlocks(  (uint32_t*) pu8Buffer,
                                      (uint32_t) (xSector),
                                      u32Count,
                                      EF_PORT_SD_TIMEOUT) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERROR );
  }
  else
  {
    ef_u32_t timeout = EF_PORT_SD_TIMEOUT / 10;

    /* wait until the Read operation is finished */
    while ( MSD_OK != BSP_SD_GetCardState( ) )
    {
      eRetVal = EF_RET_OK;
      if ( timeout-- == 0 )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_TIMEOUT_RD );
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

/* Write Sector(s) */
ef_return_et eEFPortDriveSDIOWrite (
  const ef_u08_t  * pu8Buffer,
  ef_lba_t          xSector,
  ef_u32_t          u32Count
)
{
  ef_return_et eRetVal = EF_RET_OK;

  if ( MSD_OK != BSP_SD_WriteBlocks(  (uint32_t*) pu8Buffer,
                                      (uint32_t) (xSector),
                                      u32Count,
                                      EF_PORT_SD_TIMEOUT) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERROR );
  }
  else
  {
    ef_u32_t timeout = EF_PORT_SD_TIMEOUT / 10;

    /* wait until the Write operation is finished */
    while ( MSD_OK != BSP_SD_GetCardState( ) )
    {
      eRetVal = EF_RET_OK;
      if ( timeout-- == 0 )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_TIMEOUT_WR );
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
  ef_return_et    eRetVal = EF_RET_OK;
  BSP_SD_CardInfo CardInfo;

  if ( EF_RET_DISK_NOINIT == eSDIOStatus )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_NOTRDY );
  }
  else if ( CTRL_SYNC == u8Cmd )
  {
    /* Else, if Make sure that no pending write process */
    EF_CODE_COVERAGE( );
  }
  else if ( GET_SECTOR_COUNT == u8Cmd )
  {
    EF_ASSERT_PRIVATE( 0 != pvBuffer );
    /* Get number of sectors on the disk (DWORD) */
    BSP_SD_GetCardInfo( &CardInfo );
    *(ef_u32_t*)pvBuffer = CardInfo.LogBlockNbr;
  }
  else if ( GET_SECTOR_SIZE == u8Cmd )
  {
    EF_ASSERT_PRIVATE( 0 != pvBuffer );
    /* Get R/W xSector size (WORD) */
    BSP_SD_GetCardInfo( &CardInfo );
    *(ef_u16_t*)pvBuffer = (ef_u16_t) CardInfo.LogBlockSize;
  }
  else if ( GET_BLOCK_SIZE == u8Cmd )
  {
    EF_ASSERT_PRIVATE( 0 != pvBuffer );
    /* Get erase block size in unit of xSector (DWORD) */
    BSP_SD_GetCardInfo( &CardInfo );
    *(ef_u32_t*)pvBuffer = CardInfo.LogBlockSize;
  }
  else
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_PARERR );
  }

  return eRetVal;
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

