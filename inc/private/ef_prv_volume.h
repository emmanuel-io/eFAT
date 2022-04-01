/**
 * ********************************************************************************************************************
 *  @file     ef_prv_volume.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private volume access management.
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
#ifndef EFAT_PRIVATE_VOLUME_H
#define EFAT_PRIVATE_VOLUME_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include <efat.h>             /* Declarations of eFAT API */
#include "ef_port_diskio.h" /* Declarations of device I/O functions */
#include "ef_prv_def.h"         /* Declarations of eFAT API */

/* Local constant macros ------------------------------------------------------------------------------------------- */
/**
 *  Partition number to enable find first mode on volume mounting
 */
#define PARTITION_FIND_FIRST  ( 255 )

/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */

/**
 *  @brief Enumerated types for handled boot sectors
 */
typedef enum {
  EF_SECTOR_TYPE_VBR_FAT16      = 0,  /**< Volume Boot Record FAT12/16 */
  EF_SECTOR_TYPE_VBR_FAT32      = 1,  /**< Volume Boot Record FAT32 */
//  EF_SECTOR_TYPE_VBR_EXFAT      = 2,  /**< Volume Boot Record ExFAT */
  EF_SECTOR_TYPE_BS_UNKNOWN     = 3,  /**< Boot Sector Valid but not FAT VBR */
  EF_SECTOR_TYPE_PROTECTIVE_MBR = 4,  /**< Boot Sector Protective MBR*/
  EF_SECTOR_TYPE_BS_INVALID     = 5,  /**< Boot Sector Invalid */
  EF_SECTOR_TYPE_DISK_ERROR     = 6   /**< Disk error */
} ef_sector_type_et;

/* Local function macros ------------------------------------------------------------------------------------------- */

/* Definitions of logical drive - physical location conversion */

#if EF_CONF_LBA64
#if EF_CONF_GPT_MIN > 0x100000000
#error Wrong EF_CONF_GPT_MIN setting
#endif
#endif
#if ( 0 != EF_CONF_USE_GPT )
static const ef_u08_t u8GUIDBasicDataPartition[ 16 ] = {0xA2,0xA0,0xD0,0xEB,0xE5,0xB9,0x33,0x44,0x87,0xC0,0x68,0xB6,0xB7,0x26,0x99,0xC7};
#else
static const ef_u08_t u8GUIDBasicDataPartition[ 1 ] = {0x00};
#endif
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions prototypes --------------------------------------------- */

/**
 *  @brief  Find an FAT volume
 *          (It supports only generic partitioning rules, MBR, GPT and SFD)
 *
 *  @param  pxFS        Pointer to the found filesystem object
 *  @param  u8Partition Partition to find
 *                      0:      Try VBR,
 *                      1..4:   Try in MBR or GPT,
 *                      5..128: Try in GPT partioning scheme
 *                      255:    Try to find the first one we can handle.
 *
 *  @return Returns BS status found in the hosting drive
 *  @retval EF_SECTOR_TYPE_VBR_FAT16    Volume Boot Record FAT12/16
 *  @retval EF_SECTOR_TYPE_VBR_FAT32    Volume Boot Record FAT32
 *  @retval EF_SECTOR_TYPE_BS_UNKNOWN   Boot Sector Valid but not FAT VBR
 *  @retval EF_SECTOR_TYPE_BS_INVALID   Boot Sector Invalid
 *  @retval EF_SECTOR_TYPE_DISK_ERROR   Disk error
 */
ef_sector_type_et eEFPrvVolumeFind (
  ef_fs_st  * pxFS,
  ef_u08_t   u8Partition
);

/**
 *  @brief  Create an FAT volume                                            */
/*-----------------------------------------------------------------------*/

#define N_SEC_TRACK 63        /* Sectors per track for determination of drive CHS */
#define GPT_ALIGN   0x100000  /* Alignment of partitions in GPT [byte] (>=128KB) */
#define GPT_ITEMS   128       /* Number of GPT table size (>=128, sector aligned) */

/**
 *  @brief  Create partitions on the physical drive
 *
 *  @param  u8PhyDrvNb  Physical drive number
 *  @param  plst[]      Partition list
 *  @param  sys         System ID (for only MBR, temp setting) and bit8:GPT
 *  @param  buf         Working buffer for a sector
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

ef_return_et eEFPrvPartitionCreate (
  ef_u08_t     u8PhyDrvNb, /* Physical drive number */
  const ef_lba_t   plst[ ],    /* Partition list */
  ef_u32_t     sys,        /* System ID (for only MBR, temp setting) and bit8:GPT */
  ef_u08_t   * buf         /* Working buffer for a sector */
);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_VOLUME_H */
/* END OF FILE ***************************************************************************************************** */

