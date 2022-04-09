/**
 * ********************************************************************************************************************
 *  @file     ef_def.h
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Public definitions for eFAT
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
#ifndef EFAT_PUBLIC_DEFINITIONS_H
#define EFAT_PUBLIC_DEFINITIONS_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
/**
 *  Current Revision ID
 */
#define EF_REVISION  0001

#include <ef_conf.h>        /* eFAT configuration options */
#include <ef_port_types.h>  /* eFAT portable definitions */

#if ( EF_REVISION != EF_CONF_REVISION )
#error Wrong configuration file (ffconf.h).
#endif
/* Local function macros ------------------------------------------------------------------------------------------- */

/**
 *  Partition number to enable find first mode on volume mounting
 */
#define PARTITION_FIND_FIRST  ( 255 )

/* File access mode and open method flags (3rd argument of f_open) */
#define EF_FILE_OPEN_WRITE    0x01 /**< File opening in write mode */
#define EF_FILE_OPEN_EXISTING 0x02 /**< File opening only if it exist */
#define EF_FILE_OPEN_ANYWAY   0x04 /**< File opening if it exist or not */
#define EF_FILE_OPEN_NEW      0x08 /**< File opening only if it doesn't exist */
#define EF_FILE_OPEN_TRUNCATE 0x10 /**< File opening in truncate mode */
#define EF_FILE_OPEN_APPEND   0x20 /**< File opening in append mode */
#define EF_FILE_OPEN_MASK     0x3F /**< File opening parameters mask */
/*
 * Opening mode :
 * File : exist absent  create  result
 *        NO    NO      NO
 *        NO    NO      NO
 *        NO    NO      NO
 *        NO    NO      NO
 *        NO    NO      NO
 *
 *
 */

/* Format options (2nd argument of f_mkfs) */
#define FM_FAT    0x01
#define FM_FAT32  0x02
#define FM_ANY    0x07
#define FM_SFD    0x08

/* Command code for disk_ioctrl function */

/* Generic command (Used by eFAT) */
#define CTRL_SYNC         (  0 )  /**< Complete pending write process */
#define GET_SECTOR_COUNT  (  1 )  /**< Get media size (needed at EF_USE_MKFS == 1) */
#define GET_SECTOR_SIZE   (  2 )  /**< Get sector size (needed at EF_CONF_SS_MAX != EF_CONF_SS_MIN) */
#define GET_BLOCK_SIZE    (  3 )  /**< Get erase block size (needed at EF_USE_MKFS == 1) */
#define CTRL_TRIM         (  4 )  /**< Inform device that the data on the block of sectors is no longer used (needed at EF_CONF_USE_TRIM == 1) */

/* Generic command (Not used by eFAT) */
#define CTRL_POWER        (  5 )  /**< Get/Set power status */
#define CTRL_LOCK         (  6 )  /**< Lock/Unlock media removal */
#define CTRL_EJECT        (  7 )  /**< Eject media */
#define CTRL_FORMAT       (  8 )  /**< Create physical format on the media */

/* MMC/SDC specific ioctl command */
#define MMC_GET_TYPE      ( 10 )  /**< Get card type */
#define MMC_GET_CSD       ( 11 )  /**< Get CSD */
#define MMC_GET_CID       ( 12 )  /**< Get CID */
#define MMC_GET_OCR       ( 13 )  /**< Get OCR */
#define MMC_GET_SDSTAT    ( 14 )  /**< Get SD status */
#define ISDIO_READ        ( 55 )  /**< Read data form SD iSDIO register */
#define ISDIO_WRITE       ( 56 )  /**< Write data to SD iSDIO register */
#define ISDIO_MRITE       ( 57 )  /**< Masked write data to SD iSDIO register */

/* ATA/CF specific ioctl command */
#define ATA_GET_REV       ( 20 )  /**< Get F/W revision */
#define ATA_GET_MODEL     ( 21 )  /**< Get model name */
#define ATA_GET_SN        ( 22 )  /**< Get serial number */

/**
 *  This option switches support for FAT12 filesystem. (0:Disable or 1:Enable)
 */
#if ( EF_CONF_FS_FAT12 )
  #define EF_FS_FAT12   ( 0x01 )  /**< File System type FAT12 */
#else
  #define EF_FS_FAT12   ( 0 )
#endif

/**
 *  This option switches support for FAT12/16 filesystem. (0:Disable or 1:Enable)
 */
#if ( EF_CONF_FS_FAT16 )
  #define EF_FS_FAT16   ( 0x02 )  /**< File System type FAT16 */
#else
  #define EF_FS_FAT16   ( 0 )
#endif

/**
 *  This option switches support for FAT32 filesystem. (0:Disable or 1:Enable)
 */
#if ( EF_CONF_FS_FAT32 )
  #define EF_FS_FAT32   ( 0x04 )  /**< File System type FAT32 */
#else
  #define EF_FS_FAT32   ( 0 )
#endif

/**
 *  This define support for any (not all!) FAT12/16/32 filesystem.
 */
#define EF_FS_FATS   ( EF_FS_FAT12 | EF_FS_FAT16 | EF_FS_FAT32 )

/**
 *  This set of options defines size of file name members in the ef_file_info_st structure
 *  which is used to read out directory items. These values should be sufficient for
 *  the file names to read. The maximum possible length of the read file name depends
 *  on character encoding. When LFN is not enabled, these options have no effect.
 */
#define EF_LFN_BUF    ( 255 )
#define EF_SFN_BUF    ( 12 )

/**
 *  Minimum Number of volumes (logical drives) to be used.
 */
#define EF_CONF_VOLUMES_NB_MIN  ( 1 )

/**
 *  Maximum Number of volumes (logical drives) to be used.
 */
#define EF_CONF_VOLUMES_NB_MAX  ( 26 )

/**
 *  Type of path name strings on eFAT API
 */
#if ( EF_DEF_API_UTF8 == EF_CONF_API_ENCODING )  /* API in Unicode UTF-8 encoding */
  #define _T(x) u8 ## x
  #define _TEXT(x) u8 ## x
#elif ( EF_DEF_API_UTF16 == EF_CONF_API_ENCODING )   /* API in Unicode UTF-16 encoding */
  #define _T(x) L ## x
  #define _TEXT(x) L ## x
#elif ( EF_DEF_API_UTF32 == EF_CONF_API_ENCODING )  /* API in Unicode UTF-32 encoding */
  #define _T(x) U ## x
  #define _TEXT(x) U ## x
#elif ( EF_DEF_API_OEM == EF_CONF_API_ENCODING )    /* API in ANSI/OEM in current CP encoding */
  #define _T(x) x
  #define _TEXT(x) x
#else
  #error Wrong EF_CONF_API_ENCODING setting
#endif

/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */

/** UTF-16 character type */
typedef ef_u16_t  ucs2_t;

/**
 *  Type of path name strings on eFAT API
 */
#if ( EF_DEF_API_UTF8 == EF_CONF_API_ENCODING )     /* API in Unicode UTF-8 encoding */
  typedef char TCHAR;
#elif ( EF_DEF_API_UTF16 == EF_CONF_API_ENCODING )  /* API in Unicode UTF-16 encoding */
  typedef ucs2_t TCHAR;
#elif ( EF_DEF_API_UTF32 == EF_CONF_API_ENCODING )  /* API in Unicode UTF-32 encoding */
  typedef ef_u32_t TCHAR;
#elif ( EF_DEF_API_OEM == EF_CONF_API_ENCODING )    /* API in ANSI/OEM in current CP encoding */
  typedef char TCHAR;
#else
  #error Wrong EF_CONF_API_ENCODING setting
#endif

/**
 *  Type of file size and LBA variables
 */
#if EF_CONF_LBA64
  typedef ef_u64_t ef_lba_t;
#else
  typedef ef_u32_t ef_lba_t;
#endif

/**
 *  @brief  File function return code (ef_return_et)
 */
typedef enum {
  EF_RET_OK = 0,              /**< (0) Succeeded */
  EF_RET_DISK_ERR,            /**< (1) A hard error occurred in the low level disk I/O layer */
  EF_RET_INT_ERR,             /**< (2) Assertion failed */
  EF_RET_NOT_READY,           /**< (3) The physical drive cannot work */
  EF_RET_NO_FILE,             /**< (4) Could not find the file */
  EF_RET_NO_PATH,             /**< (5) Could not find the pxPath */
  EF_RET_INVALID_NAME,        /**< (6) The pxPath name format is invalid */
  EF_RET_DENIED,              /**< (7) Access denied due to prohibited access or directory full */
  EF_RET_EXIST,               /**< (8) Access denied due to prohibited access */
  EF_RET_INVALID_OBJECT,      /**< (9) The file/directory object is invalid */
  EF_RET_WRITE_PROTECTED,     /**< (10) The physical drive is write protected */
  EF_RET_INVALID_DRIVE,       /**< (11) The logical drive number is invalid */
  EF_RET_NOT_ENABLED,         /**< (12) The volume has no work area */
  EF_RET_NO_FILESYSTEM,       /**< (13) There is no valid FAT volume */
  EF_RET_MKFS_ABORTED,        /**< (14) The f_mkfs() aborted due to any problem */
  EF_RET_TIMEOUT,             /**< (15) Could not get a grant to access the volume within defined period */
  EF_RET_LOCKED,              /**< (16) The operation is rejected according to the file sharing policy */
  EF_RET_NOT_ENOUGH_CORE,     /**< (17) LFN working buffer could not be allocated */
  EF_RET_TOO_MANY_OPEN_FILES, /**< (18) Number of open files > EF_CONF_FILE_LOCK */
  EF_RET_INVALID_PARAMETER,   /**< (19) Given parameter is invalid */
  EF_RET_ASSERT,              /**< (20) Assertion failed */
  EF_RET_ERROR,               /**< (21) An error occurred (internally or in a subfunction) */
  EF_RET_SYS_ERROR,           /**< (22) An error occurred from system port */
  EF_RET_FAT_ENTRY_FREE,      /**< (23) Cluster is empty */
  EF_RET_FAT_ENTRY_END_1,     /**< (24) Cluster is end of chain (special case of allocation failure) */
  EF_RET_FAT_ENTRY_NEXT,      /**< (25) Cluster is followed by another one */
  EF_RET_FAT_ENTRY_BAD,       /**< (26) Cluster is empty */
  EF_RET_FAT_ENTRY_LAST,      /**< (27) Cluster is last of a chain */
  EF_RET_FAT_FULL,            /**< (28) File allocation table is full */
  EF_RET_PUBLIC_ASSERT,       /**< (29) Public given parameter is invalid */
  EF_RET_DISK_ERROR,          /**< (30) R/W Error */
  EF_RET_DISK_TIMEOUT,        /**< (30) R/W Error */
  EF_RET_DISK_TIMEOUT_WR,     /**< (30) R/W Error */
  EF_RET_DISK_TIMEOUT_RD,     /**< (30) R/W Error */
  EF_RET_DISK_NOTRDY,         /**< (31) Not Ready */
  EF_RET_DISK_PARERR,         /**< (32) Invalid Parameter */
  EF_RET_DISK_NOINIT,         /**< (33) Drive not initialized */
  EF_RET_DISK_NODISK,         /**< (34) No medium in the drive */
  EF_RET_DISK_PROTECT,         /**< (35) Write protected */
  EF_RET_FAT_CLUSTER_UNDER,      /**< (23) Cluster is empty */
  EF_RET_FAT_CLUSTER_OVER,      /**< (23) Cluster is empty */
  EF_RET_FAT_CLUSTER_OUT,      /**< (23) Cluster is empty */
  EF_RET_FAT_ERROR,            /**< (28) File allocation table is full */
  EF_RET_LFN_BUFFER_ERROR,      /**< (23) Cluster is empty */
  EF_RET_BUFFER_ERROR      /**< (23) Cluster is empty */
} ef_return_et;

/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PUBLIC_DEFINITIONS_H */
/* END OF FILE ***************************************************************************************************** */
