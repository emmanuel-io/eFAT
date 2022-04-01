/**
 * ********************************************************************************************************************
 *  @file     ef_prv_fs_window.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private disk access window in the filesystem object.
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
#ifndef EFAT_PRIVATE_WINDOW_FS_H
#define EFAT_PRIVATE_WINDOW_FS_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include <efat.h>
#include "ef_port_diskio.h"
#include "ef_prv_def.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/**
 *  @brief  Store disk access window in the filesystem object
 *
 *  @param  pxFS  Pointer to the Filesystem object
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvFSWindowStore (
  ef_fs_st  * pxFS
);

/**
 *  @brief  Load disk access window in the filesystem object
 *
 *  @param  pxFS    Pointer to the Filesystem object
 *  @param  xSector Sector LBA to make appearance in the pxFS->pu8Window[]
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvFSWindowLoad (
  ef_fs_st  * pxFS,
  ef_lba_t    xSector
);

/**
 *  @brief  Synchronize filesystem and data on the storage
 *
 *  @param  pxFS  Pointer to the Filesystem object
 *
 *  @return File function return code
 *  @retval EF_RET_OK         Succeeded
 *  @retval EF_RET_DISK_ERR   A hard error occurred in the low level disk I/O layer
 */
ef_return_et eEFPrvFSSync (
  ef_fs_st  * pxFS
);


/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_WINDOW_FS_H */
/* END OF FILE ***************************************************************************************************** */
