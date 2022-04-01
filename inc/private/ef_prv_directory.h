/**
 * ********************************************************************************************************************
 *  @file     ef_prv_directory.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private Header file for directory handling.
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
#ifndef EFAT_PRIVATE_DIRECTORY_H
#define EFAT_PRIVATE_DIRECTORY_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */

#include <efat.h>
#include "ef_port_diskio.h"
#include "ef_prv_def.h"

/* Public functions prototypes --------------------------------------------- */

/**
 *  @brief  Load start cluster number
 *
 *  @param  pxFS        Pointer to the Filesystem object
 *  @param  pu8Dir      Pointer to the directory entry
 *  @param  pu32Cluster Pointer to start cluster of the directory entry
 *
 *  @return Function completion
 *  @retval EF_RET_OK     Succeeded
 *  @retval EF_RET_ASSERT Assertion failed on pointer
 */
ef_return_et eEFPrvDirectoryClusterGet(
  ef_fs_st        * pxFS,
  const ef_u08_t  * pu8Dir,
  ef_u32_t        * pu32Cluster
);

/**
 *  @brief  Store start cluster number
 *
 *  @param  pxFS        Pointer to the Filesystem object
 *  @param  pu8Dir      Pointer to the directory entry
 *  @param  u32Cluster  Value to be set
 *
 *  @return Function completion
 *  @retval EF_RET_OK     Succeeded
 *  @retval EF_RET_ASSERT Assertion failed on pointer
 */
ef_return_et eEFPrvDirectoryClusterSet
(
  ef_fs_st  * pxFS,       /* Pointer to the pxFS object */
  ef_u08_t  * pu8Dir,     /* Pointer to the directory entry */
  ef_u32_t    u32Cluster  /* Value to be set */
);

/**
 *  @brief  Directory handling - Fill a cluster with zeros
 *
 *  @param  pxFS          Pointer to the filesystem object
 *  @param  u32Cluster    Directory table to clear
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
  ef_return_et eEFPrvDirectoryClusterClear (
  ef_fs_st  * pxFS,
  ef_u32_t    u32Cluster
);

/**
 *  @brief  Directory handling - Set directory index
 *
 *  @param  pxDir     Pointer to directory object
 *  @param  u32Offset Offset of directory table
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_DISK_ERR             A hard error occurred in the low level disk I/O layer
 *  @retval EF_RET_INT_ERR              Assertion failed
 *  @retval EF_RET_NOT_READY            The physical drive cannot work
 *  @retval EF_RET_NO_FILE              Could not find the file
 *  @retval EF_RET_NO_PATH              Could not find the path
 *  @retval EF_RET_INVALID_NAME         The path name format is invalid
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
  ef_return_et eEFPrvDirectoryIndexSet (  /* EF_RET_OK(0):succeeded, !=0:error */
  ef_directory_st * pxDir,            /* Pointer to directory object */
  ef_u32_t          u32Offset         /* Offset of directory table */
);

/**
 *  @brief  Directory handling - Move directory table index next, Do not stretch table
 *
 *  @param  pxDir     Pointer to the directory object
 *  @param  bStretch  Boolean to request index streching
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_NO_FILE              Could not find the file
 *  @retval EF_RET_DENIED               Access denied due to prohibited access or directory full
 */
ef_return_et eEFPrvDirectoryIndexNext (
    ef_directory_st * pxDir,
    ef_bool_t         bStretch
);

/**
 *  @brief  Directory handling - Reserve a block of directory entries
 *
 *  @param  pxDir       Pointer to the directory object
 *  @param  u32EntriesNb Number of contiguous entries to allocate
 *
 *  @return Operation result
 *  @retval EF_RET_OK       Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvDirectoryAllocate (
  ef_directory_st * pxDir,
  ef_u32_t          u32EntriesNb
);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_DIRECTORY_H */
/* END OF FILE ***************************************************************************************************** */

