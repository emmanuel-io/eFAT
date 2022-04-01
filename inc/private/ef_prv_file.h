/**
 * ********************************************************************************************************************
 *  @file     ef_prv_file.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private file specific functions protoypes.
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
#ifndef EFAT_PRIVATE_WINDOW_FILE_H
#define EFAT_PRIVATE_WINDOW_FILE_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include <efat.h>
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
 *  @brief  Get file information from directory entry (LFN)
 *
 *  @param  pxDir       Pointer to the directory object
 *  @param  pxFileInfo  Pointer to the file information to be filled
 *
 *  @return Operation result
 *  @retval EF_RET_OK       Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvDirFileInfosGetVFAT (
  ef_directory_st * pxDir,
  ef_file_info_st * pxFileInfo
);

/**
 *  @brief  Get file information from directory entry (SFN)
 *
 *  @param  pxDir       Pointer to the directory object
 *  @param  pxFileInfo  Pointer to the file information to be filled
 *
 *  @return Operation result
 *  @retval EF_RET_OK       Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvDirFileInfosGetFAT (
  ef_directory_st * pxDir,
  ef_file_info_st * pxFileInfo
);


#if ( 0 != EF_CONF_VFAT )
  /* LFN configuration */
  #define eEFPrvDirFileInfosGet( pxDir, pxFileInfo ) eEFPrvDirFileInfosGetVFAT( pxDir, pxFileInfo )
#else
  /* SFN configuration */
  #define eEFPrvDirFileInfosGet( pxDir, pxFileInfo ) eEFPrvDirFileInfosGetFAT( pxDir, pxFileInfo )
#endif

/**
 *  @brief  Write back sector in window if dirty and clear flag
 *
 *  @param  pxFile    Pointer to the File object
 *  @param  pxFS      Pointer to the Filesystem object
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvFileWindowDirtyWriteBack (
  ef_file_st  * pxFile,
  ef_fs_st    * pxFS
);

/**
 *  @brief  Update file window with new sector
 *
 *  @param  pxFile    Pointer to the File object
 *  @param  pxFS      Pointer to the Filesystem object
 *  @param  xSector   New sector to load in the file window
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvFileWindowUpdate (
  ef_file_st  * pxFile,
  ef_fs_st    * pxFS,
  ef_lba_t      xSector
);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_WINDOW_FILE_H */
/* END OF FILE ***************************************************************************************************** */
