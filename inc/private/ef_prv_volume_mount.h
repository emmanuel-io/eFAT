/**
 * ********************************************************************************************************************
 *  @file     ef_prv_volume_mount.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private volume mounting.
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
#ifndef EFAT_PRIVATE_VOLUME_MOUNT_H
#define EFAT_PRIVATE_VOLUME_MOUNT_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include <efat.h>
#include "ef_port_diskio.h"
#include "ef_prv_def.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */

#if ( ( 0 != EF_CONF_LBA64 ) && ( 0x100000000 < EF_CONF_GPT_MIN ) )
  #error Wrong EF_CONF_GPT_MIN setting
#endif

/* Public functions prototypes --------------------------------------------- */

/**
 *  @brief  Determine logical drive number and mount the volume if needed
 *
 *  @param  pxFS        Pointer to the filesystem object
 *  @param  u8ReadOnly  Non zero: Read Only
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_DISK_ERR             A hard error occurred in the low level disk I/O layer
 *  @retval EF_RET_INT_ERR              Assertion failed
 *  @retval EF_RET_NOT_READY            The physical drive cannot work
 *  @retval EF_RET_NO_FILE              Could not find the file
 *  @retval EF_RET_NO_PATH              Could not find the pxPath
 *  @retval EF_RET_INVALID_NAME         The pxPath name format is invalid
 *  @retval EF_RET_DENIED               Access denied due to prohibited access or directory full
 *  @retval EF_RET_EXIST                Access denied due to prohibited access
 *  @retval EF_RET_INVALID_OBJECT       The file/directory object is invalid
 *  @retval EF_RET_WRITE_PROTECTED      The physical drive is write protected
 *  @retval EF_RET_INVALID_DRIVE        The logical drive number is invalid
 *  @retval EF_RET_NOT_ENABLED          The volume has no work area
 *  @retval EF_RET_NO_FILESYSTEM        There is no valid FAT volume
 *  @retval EF_RET_MKFS_ABORTED         The f_mkfs() aborted due to any problem
 *  @retval EF_RET_TIMEOUT              Could not get a grant to access the volume within defined period
 *  @retval EF_RET_LOCKED               The operation is rejected according to the file sharing policy
 *  @retval EF_RET_NOT_ENOUGH_CORE      LFN working buffer could not be allocated
 *  @retval EF_RET_TOO_MANY_OPEN_FILES  Number of open files > EF_CONF_FILE_LOCK
 *  @retval EF_RET_INVALID_PARAMETER    Given parameter is invalid
 */
ef_return_et eEFPrvVolumeMount (
  ef_fs_st  * pxFS,
  ef_u08_t   u8ReadOnly
);

/**
 *  @brief  Determine logical drive number and mount the volume if needed
 *
 *  @param  ppxPath Pointer to pointer to the pxPath name (drive number)
 *  @param  ppxFS   Pointer to pointer to the found filesystem object
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_DISK_ERR             A hard error occurred in the low level disk I/O layer
 *  @retval EF_RET_INT_ERR              Assertion failed
 *  @retval EF_RET_NOT_READY            The physical drive cannot work
 *  @retval EF_RET_NO_FILE              Could not find the file
 *  @retval EF_RET_NO_PATH              Could not find the pxPath
 *  @retval EF_RET_INVALID_NAME         The pxPath name format is invalid
 *  @retval EF_RET_DENIED               Access denied due to prohibited access or directory full
 *  @retval EF_RET_EXIST                Access denied due to prohibited access
 *  @retval EF_RET_INVALID_OBJECT       The file/directory object is invalid
 *  @retval EF_RET_WRITE_PROTECTED      The physical drive is write protected
 *  @retval EF_RET_INVALID_DRIVE        The logical drive number is invalid
 *  @retval EF_RET_NOT_ENABLED          The volume has no work area
 *  @retval EF_RET_NO_FILESYSTEM        There is no valid FAT volume
 *  @retval EF_RET_MKFS_ABORTED         The f_mkfs() aborted due to any problem
 *  @retval EF_RET_TIMEOUT              Could not get a grant to access the volume within defined period
 *  @retval EF_RET_LOCKED               The operation is rejected according to the file sharing policy
 *  @retval EF_RET_NOT_ENOUGH_CORE      LFN working buffer could not be allocated
 *  @retval EF_RET_TOO_MANY_OPEN_FILES  Number of open files > EF_CONF_FILE_LOCK
 *  @retval EF_RET_INVALID_PARAMETER    Given parameter is invalid
 */
ef_return_et eEFPrvVolumeMountCheck (
  const TCHAR **  ppxPath,
  ef_fs_st    **  ppxFS
);

/**
 *  @brief  Determine filesystem object from volume number
 *
 *  @param  s8VolumeNb Pointer to pointer to the pxPath name (drive number)
 *  @param  ppxFS       Pointer to pointer to the filesystem object to get
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_DISK_ERR             A hard error occurred in the low level disk I/O layer
 *  @retval EF_RET_INT_ERR              Assertion failed
 *  @retval EF_RET_NOT_READY            The physical drive cannot work
 *  @retval EF_RET_NO_FILE              Could not find the file
 *  @retval EF_RET_NO_PATH              Could not find the pxPath
 *  @retval EF_RET_INVALID_NAME         The pxPath name format is invalid
 *  @retval EF_RET_DENIED               Access denied due to prohibited access or directory full
 *  @retval EF_RET_EXIST                Access denied due to prohibited access
 *  @retval EF_RET_INVALID_OBJECT       The file/directory object is invalid
 *  @retval EF_RET_WRITE_PROTECTED      The physical drive is write protected
 *  @retval EF_RET_INVALID_DRIVE        The logical drive number is invalid
 *  @retval EF_RET_NOT_ENABLED          The volume has no work area
 *  @retval EF_RET_NO_FILESYSTEM        There is no valid FAT volume
 *  @retval EF_RET_MKFS_ABORTED         The f_mkfs() aborted due to any problem
 *  @retval EF_RET_TIMEOUT              Could not get a grant to access the volume within defined period
 *  @retval EF_RET_LOCKED               The operation is rejected according to the file sharing policy
 *  @retval EF_RET_NOT_ENOUGH_CORE      LFN working buffer could not be allocated
 *  @retval EF_RET_TOO_MANY_OPEN_FILES  Number of open files > EF_CONF_FILE_LOCK
 *  @retval EF_RET_INVALID_PARAMETER    Given parameter is invalid
 */
ef_return_et eEFPrvVolumeFSPtrGet (
  int8_t      s8VolumeNb,
  ef_fs_st ** ppxFS
);

/**
 *  @brief  Set filesystem object pointer associated with volume number
 *
 *  @param  s8VolumeNb  Pointer to pointer to the pxPath name (drive number)
 *  @param  pxFS        Pointer to the filesystem object to get
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_INVALID_PARAMETER    Given parameter is invalid
 */
ef_return_et eEFPrvVolumeFSPtrSet (
  int8_t      s8VolumeNb,
  ef_fs_st  * pxFS
);
/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_VOLUME_MOUNT_H */
/* END OF FILE ***************************************************************************************************** */

