/**
 * ********************************************************************************************************************
 *  @file     ef_prv_drive.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Header file for Low level disk interface.
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

/* START OF FILE *************************************************************************************************** */
#ifndef EFAT_PRIVATE_DRIVE_DEFINED
#define EFAT_PRIVATE_DRIVE_DEFINED
#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include "ef_port_types.h"
/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Public functions prototypes---------------------------------------------- */

/**
 *  @brief  Initialize a Drive
 *
 *  @param  u8PhyDrvNb 8 bits unsigned integer identifying the physical drive number
 *
 *  @return Status of Disk Functions
 */
ef_return_et  eEFPrvDriveInitialize (
  ef_u08_t u8PhyDrvNb
);

/**
 *  @brief  Get Drive Status
 *
 *  @param  u8PhyDrvNb 8 bits unsigned integer identifying the physical drive number
 *
 *  @return Status of Disk Functions
 */
ef_return_et  eEFPrvDriveStatus (
  ef_u08_t u8PhyDrvNb
  );

/**
 *  @brief  Read Sector(s)
 *
 *  @param  u8PhyDrvNb  8 bits unsigned integer identifying the physical drive number
 *  @param  pu8Buffer   Pointer to the data buffer to store read data
 *  @param  xSector     Start sector in LBA
 *  @param  u32Count    Number of sectors to read
 *
 *  @return Results of Disk Functions
 *  @retval EF_RET_OK      Successful
 *  @retval EF_RET_DISK_ERROR   R/W Error
 *  @retval EF_RET_DISK_WRPRT   Write Protected
 *  @retval EF_RET_DISK_NOTRDY  Not Ready
 *  @retval EF_RET_DISK_PARERR  Invalid Parameter
 */
ef_return_et  eEFPrvDriveRead (
  ef_u08_t    u8PhyDrvNb,
  ef_u08_t  * pu8Buffer,
  ef_lba_t    xSector,
  ef_u32_t    u32Count
);

/**
 *  @brief  Write Sector(s)
 *
 *  @param  u8PhyDrvNb  8 bits unsigned integer identifying the physical drive number
 *  @param  pu8Buffer   Pointer to the data to be written
 *  @param  xSector     Start sector in LBA
 *  @param  u32Count    Number of sectors to write
 *
 *  @return Results of Disk Functions
 *  @retval EF_RET_OK      Successful
 *  @retval EF_RET_DISK_ERROR   R/W Error
 *  @retval EF_RET_DISK_WRPRT   Write Protected
 *  @retval EF_RET_DISK_NOTRDY  Not Ready
 *  @retval EF_RET_DISK_PARERR  Invalid Parameter
 */
ef_return_et  eEFPrvDriveWrite (
  ef_u08_t          u8PhyDrvNb,
  const ef_u08_t  * pu8Buffer,
  ef_lba_t          xSector,
  ef_u32_t          u32Count
);

/**
 *  @brief  Miscellaneous Functions
 *
 *  @param  u8PhyDrvNb  8 bits unsigned integer identifying the physical drive number
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
ef_return_et  eEFPrvDriveIOCtrl (
  ef_u08_t  u8PhyDrvNb,
  ef_u08_t  u8Cmd,
  void    * pvBuffer
);

/**
 *  @brief  Register the functions needed to access a Drive
 *
 *  @param  pxDriveFunctions  Pointers to structure of Functions Pointer for a drive
 *
 *  @return Function completion
 *  @retval EF_RET_OK                 Succeeded
 *  @retval EF_RET_DENIED             Access denied due to prohibited access or directory full
 *  @retval EF_RET_INVALID_PARAMETER  Given parameter is invalid
 */
ef_return_et eEFPrvDriveRegister (
  ef_drive_functions_st * pxDriveFunctions
);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_DRIVE_DEFINED */
/* END OF FILE ***************************************************************************************************** */
