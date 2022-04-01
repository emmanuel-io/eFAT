/**
 * ********************************************************************************************************************
 *  @file     ef_prv_fat.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief   Private Header file.
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
#ifndef EFAT_PRIVATE_FAT_H
#define EFAT_PRIVATE_FAT_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */


/* Includes -------------------------------------------------------------------------------------------------------- */
#include <efat.h>
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
/* Remark: Variables defined here without initial value shall be guaranteed
/  zero/null at start-up. If not, the linker option or start-up routine is
/  not compliance with C standard. */

/*--------------------------------*/
/* LFN/Directory working buffer   */
/*--------------------------------*/

/* Check if cluster number is valid */
/**
 *  @brief  Check if cluster number is valid
 *
 *  @param  u32FatEntriesNb Maximum number of fat entries from file system
 *  @param  u32Cluster      Cluster number to converted
 *
 *  @return Validity of the cluster number
 *  @retval EF_RET_OK       Success, number is valid
 *  @retval EF_RET_ERROR    Error, number is not valid
 */
ef_return_et eEFPrvFATClusterNbCheck (
  ef_u32_t  u32FatEntriesNb,
  ef_u32_t  u32Cluster
//  ef_fs_st  * pxFS,
//  ef_u32_t   u32Cluster
);

/**
 *  @brief  Convert cluster number to physical sector number
 *          If cluster number is outside FAT size, sector number is set to zero
 *
 *  @param  pxFS        Pointer to the Filesystem object
 *  @param  u32Cluster  Cluster number to converted
 *  @param  pxSector    Pointer to the sector number to update
 *
 *  @return Operation result
 *  @retval EF_RET_OK       Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvFATClusterToSector (
  ef_fs_st  * pxFS,
  ef_u32_t   u32Cluster,
  ef_lba_t  * pxSector
);

/**
 *  @brief  FAT access - Get value of a FAT entry
 *
 *  @param  pxObject    Pointer to the corresponding object
 *  @param  u32Cluster  Cluster number to get the value
 *  @param  pu32Value   Pointer to the cluster value number to update
 *
 *  @return Cluster
 *  @retval 0xFFFFFFFF    Disk error
 *  @retval 1             Internal error
 *  @retval 2..0x7FFFFFFF Cluster status
 *                      0:No free cluster,
 *                      1:Internal error,
 *                      0xFFFFFFFF:Disk error,
 *                      >=2:New cluster#
 *  @return Function completion
 *  @retval EF_RET_OK         Succeeded
 *  @retval EF_RET_DISK_ERR   A hard error occurred in the low level disk I/O layer
 *  @retval EF_RET_INT_ERR    Internal error
 *  @retval EF_RET_ASSERT     Assertion failed
 */
ef_return_et eEFPrvFATGet (
  ef_fs_st  * pxFS,
  ef_u32_t    u32Cluster,
  ef_u32_t  * pu32Value
);

/**
 *  @brief  FAT access - Set value of a FAT entry
 *
 *  @param  pxFS        Corresponding filesystem object
 *  @param  u32Cluster  FAT index number (cluster number) to be changed
 *  @param  u32NewValue New value to be set to the entry
 *
 *  @return Function completion
 *  @retval EF_RET_OK         Succeeded
 *  @retval EF_RET_DISK_ERR   A hard error occurred in the low level disk I/O layer
 *  @retval EF_RET_INT_ERR    Internal error
 *  @retval EF_RET_ASSERT     Assertion failed
 */
ef_return_et eEFPrvFATSet (
  ef_fs_st  * pxFS,
  ef_u32_t    u32Cluster,
  ef_u32_t    u32NewValue
);

/**
 *  @brief  FAT handling - Remove a cluster chain
 *
 *  @param  xObject         Pointer to Corresponding object
 *  @param  u32Cluster      Cluster to remove a chain from
 *  @param  u32ClusterPrev  Previous cluster of clst (0 if entire chain)
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
ef_return_et eEFPrvFATChainRemove (
  ef_object_st  * xObject,
  ef_u32_t       u32Cluster,
  ef_u32_t       u32ClusterPrev
);

/**
 *  @brief  FAT handling - Stretch a chain or Create a new chain
 *
 *  @param  pxObject    Pointer to Corresponding object
 *  @param  u32Cluster  Cluster number to stretch, 0:Create a new chain
 *  @param  pu32Cluster Pointer to the new cluster number to update
 *                      0:No free cluster,
 *                      1:Internal error,
 *                      0xFFFFFFFF:Disk error,
 *                      >=2:New cluster#
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
ef_return_et eEFPrvFATChainCreateOld (
  ef_object_st  * pxObject,
  ef_u32_t       u32Cluster,
  ef_u32_t     * pu32Cluster
);

/**
 *  @brief  FAT handling - Create a new chain
 *
 *  @param  pxObject    Pointer to Corresponding object
 *  @param  pu32Cluster Pointer to the new cluster number
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
ef_return_et eEFPrvFATChainCreate (
  ef_object_st  * pxObject,
  ef_u32_t     * pu32Cluster
);

/**
 *  @brief  FAT handling - Crawl or Stretch a chain
 *
 *  @param  pxObject    Pointer to Corresponding object
 *  @param  u32Cluster  Cluster number to crawl or stretch
 *  @param  pu32Cluster Pointer to the new cluster number to update
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
ef_return_et eEFPrvFATChainStretch (
  ef_object_st  * pxObject,
  ef_u32_t       u32Cluster,
  ef_u32_t     * pu32Cluster
);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_FAT_H */
/* END OF FILE ***************************************************************************************************** */

