/**
 * ********************************************************************************************************************
 *  @file     ef_prv_lock.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private Header file for access control to the volume
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
#ifndef EFAT_PRIVATE_LOCK_H
#define EFAT_PRIVATE_LOCK_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include "ef_port_diskio.h" /* Declarations of device I/O functions */
#include <efat.h>             /* Declarations of eFAT API */
#include "ef_prv_def.h"     /* Declarations of eFAT API */
/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */
/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

/*-----------------------------------------------------------------------*/
/* Request/Release grant to access the volume                            */
/*-----------------------------------------------------------------------*/
/**
 *  @brief  Request grant to access the volume
 *
 *  @param  pxFS  Pointer to the Filesystem object
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvFSLock (
  ef_fs_st  * pxFS
);

/**
 *  @brief  Conditionnal Release grant to access the volume
 *
 *  @param  pxFS    Pointer to the Filesystem object
 *  @param  eResult File function return code
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvFSUnlock (
  ef_fs_st    * pxFS,
  ef_return_et  eResult
);

/**
 *  @brief  Force Release grant to access the volume
 *
 *  @param  pxFS    Pointer to the Filesystem object
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvFSUnlockForce (
  ef_fs_st  * pxFS
);
/*-----------------------------------------------------------------------*/
/* File lock control functions                                           */
/*-----------------------------------------------------------------------*/
/**
 *  @brief  Check if the file can be accessed
 *
 *  @param  pxDir   Directory object pointing the file to be checked
 *  @param  iAccess Desired access type (0:Read mode open, 1:Write mode open, 2:Delete or rename)
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_LOCKED               The operation is rejected according to the file sharing policy
 *  @retval EF_RET_TOO_MANY_OPEN_FILES  Number of open files > EF_CONF_FILE_LOCK
 */
ef_return_et eEFPrvLockCheck ( /* v */
  ef_directory_st * pxDir,
  int               iAccess
);

/**
 *  @brief  Check if an entry is available for a new object
 *
 *  @return Function completion
 *  @retval 1 Succeeded
 *  @retval 0 Fail
 */
ef_return_et eEFPrvLockEnq (
  void
);

/**
 *  @brief  Increment object open counter and returns its index (0:Internal error)
 *
 *  @param  pxDir       Directory object pointing the file to register or increment
 *  @param  iAccess     Desired access (0:Read, 1:Write, 2:Delete/Rename)
 *  @param  pu32LockId  Pointer to the Lock ID
 *
 *  @return Function completion
 */
ef_return_et eEFPrvLockInc (
  ef_directory_st * pxDir,
  int               iAccess,
  ef_u32_t        * pu32LockId
);

/**
 *  @brief  Decrement object open counter
 *
 *  @param  uiIndex Semaphore index (1..)
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_INT_ERR              Assertion failed
 */
ef_return_et eEFPrvLockDec (
  ef_u32_t  u32Index
);

/**
 *  @brief  Clear lock entries of the volume
 *
 *  @param  pxFS  Pointer to the Filesystem object
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_INT_ERR              Assertion failed
 */
ef_return_et eEFPrvLockClear (
  ef_fs_st  * pxFS
);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_LOCK_H */
/* END OF FILE ***************************************************************************************************** */

