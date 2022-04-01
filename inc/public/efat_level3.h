/**
 * ********************************************************************************************************************
 *  @defgroup group_eFAT_Public    eFAT Public
 *  @defgroup group_eFAT_Portable  eFAT Portable
 *  @defgroup group_eFAT_Private   eFAT Private
 * ********************************************************************************************************************
 */
/**
 * ********************************************************************************************************************
 *  @file     efat_level3.h
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief   Header file for eFAT module
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
#ifndef EFAT_PUBLIC_LEVEL_3_H
#define EFAT_PUBLIC_LEVEL_3_H
#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

  /* Includes -------------------------------------------------------------------------------------------------------- */
#include "ef_def.h"
#include "ef_port_types.h"
#include "efat.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */

/**
 *  @brief  Pointer to a streaming Function
 */
typedef ef_u32_t (StreamFn)(const ef_u08_t*,ef_u32_t);


/**
 *  @brief  Format parameter structure (MKFS_PARM)
 */
typedef struct {
  ef_u08_t u8Format;       /**< Format option (FM_FAT, FM_FAT32 and FM_SFD) */
  ef_u08_t u8FatsNb;       /**< Number of FATs */
  ef_u32_t u32DataAlign;    /**< Data area alignment (sector) */
  ef_u32_t u32RootDirNb;    /**< Number of root directory entries */
  ef_u32_t u32ClusterSize; /**< Cluster size (byte) */
} ef_mkfs_param_st;

/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/**
 *  @brief  Forward Data to the Stream Directly
 *
 *  @param  pxFile  Pointer to the file object
 *  @param  pxFunc  Pointer to the streaming function
 *  @param  u32BFw  Number of bytes to forward
 *  @param  pu32BFw Pointer to number of bytes forwarded
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
ef_return_et eEF_forward (
  EF_FILE   * pxFile,
  StreamFn  * pxFunc,
  ef_u32_t    u32BFw,
  ef_u32_t  * pu32BFw
);

/**
 *  @brief  Create an FAT volume
 *
 *  @param  pxPath        Logical drive number
 *  @param  pxParameters  Format options
 *  @param  pvBuffer      Pointer to working buffer (null: use heap memory)
 *  @param  u32Size       Size of working buffer [byte]
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
ef_return_et eEF_mkfs (
  const TCHAR             * pxPath,
  const ef_mkfs_param_st  * pxParameters,
  void                    * pvBuffer,
  ef_u32_t                 u32Size
);

/**
 *  @brief  Create Partition Table on the Physical Drive
 *
 *  @param  u8PhysDrv   Physical drive number
 *  @param  pxSizes[ ]  Pointer to the size table for each partitions
 *  @param  pvBuffer    Pointer to the working buffer (null: use heap memory)
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
ef_return_et eEF_fdisk (
  ef_u08_t         u8PhysDrv,
  const ef_lba_t    pxSizes[ ],
  void            * pvBuffer
);

/*--------------------------------------------------------------*/

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PUBLIC_LEVEL_3_H */
/* END OF FILE ***************************************************************************************************** */
