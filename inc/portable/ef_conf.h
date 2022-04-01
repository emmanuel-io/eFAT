/**
 * ********************************************************************************************************************
 *  @file     ef_conf.h
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Header file for eFAT module configuration
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
#ifndef EFAT_CONFIGURATIONS_H
#define EFAT_CONFIGURATIONS_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */
/**
 *  This option switches support for return code handler. (0:Disable or 1:Enable)
 */
#define EF_CONF_RETURN_CODE_HANDLER ( 1 )

/**
 *  This option switches support for assertions. (0:Disable or 1:Enable)
 */
#define EF_CONF_ASSERT_PUBLIC ( 1 )

/**
 *  This option switches support for assertions. (0:Disable or 1:Enable)
 */
#define EF_CONF_ASSERT_PRIVATE ( 1 )

/**
 *  This defines the character encoding on the API as ANSI/OEM in current CP
 */
#define EF_DEF_API_OEM ( 0 )

/**
 *  This defines the character encoding on the API as UTF-8.
 */
#define EF_DEF_API_UTF8   ( 1 )

/**
 *  This defines the character encoding on the API as UTF-16.
 */
#define EF_DEF_API_UTF16  ( 2 )

/**
 *  This defines the character encoding on the API as UTF-32.
 */
#define EF_DEF_API_UTF32  ( 3 )

/**
 *  This defines the character encoding on the API as ANSI/OEM in current CP
 */
#define EF_DEF_FILE_IO_OEM ( 0 )

/**
 *  This defines the character encoding on the API as UTF-8.
 */
#define EF_DEF_FILE_IO_UTF8   ( 1 )

/**
 *  This defines the character encoding on the API as UTF-16 LE.
 */
#define EF_DEF_FILE_IO_UTF16LE  ( 2 )

/**
 *  This defines the character encoding on the API as UTF-16 BE.
 */
#define EF_DEF_FILE_IO_UTF16BE  ( 3 )

/**
 *  Revision ID
 */
#define EF_CONF_REVISION  0001

/**
 *  This defines working on static buffer on the BSS for LFN support.
 */
#define EF_DEF_VFAT_BUFFER_STATIC ( 0 )

/**
 *  This defines working on static buffer on the stack for LFN support.
 */
#define EF_DEF_VFAT_BUFFER_STACK  ( 1 )

/**
 *  This defines working on dynamic buffer for LFN support.
 */
#define EF_DEF_VFAT_BUFFER_DYNAMIC  ( 2 )

/* ************************************************************************* **
 *  Function Configurations
 * ************************************************************************* */

/**
 *  This option switches LF-CRLF conversion for string functions :
 *  f_gets(), f_putc(), f_puts() and f_printf().
 *
 *  0: Disable LF-CRLF conversion.
 *  1: Enable LF-CRLF conversion.
 */
#define EF_CONF_CONVERT_LF_CRLF ( 1 )

/**
 *  This option switches filtered directory read functions, f_findfirst() and
 *  f_findnext(). (0:Disable, 1:Enable 2:Enable with matching xNameAlt[] too)
 */
#define EF_CONF_USE_FIND 2

/* ************************************************************************* **
 *  Locale and Namespace Configurations
 * ************************************************************************* */

/**
 *  Support OEM code page 437 - U.S. (0:Disable or 1:Enable)
 */
#define EF_CONF_CP437  ( 1 )

/**
 *  Support OEM code page 720 - Arabic (0:Disable or 1:Enable)
 */
#define EF_CONF_CP720  ( 0 )

/**
 *  Support OEM code page 737 - Greek (0:Disable or 1:Enable)
 */
#define EF_CONF_CP737  ( 0 )

/**
 *  Support OEM code page 771 - KBL (0:Disable or 1:Enable)
 */
#define EF_CONF_CP771  ( 0 )

/**
 *  Support OEM code page 775 - Baltic (0:Disable or 1:Enable)
 */
#define EF_CONF_CP775  ( 0 )

/**
 *  Support OEM code page 850 - Latin 1 (0:Disable or 1:Enable)
 */
#define EF_CONF_CP850  ( 0 )

/**
 *  Support OEM code page 852 - Latin 2 (0:Disable or 1:Enable)
 */
#define EF_CONF_CP852  ( 0 )

/**
 *  Support OEM code page 855 - Cyrillic (0:Disable or 1:Enable)
 */
#define EF_CONF_CP855  ( 0 )

/**
 *  Support OEM code page 857 - Turkish (0:Disable or 1:Enable)
 */
#define EF_CONF_CP857  ( 0 )

/**
 *  Support OEM code page 860 - Portuguese (0:Disable or 1:Enable)
 */
#define EF_CONF_CP860  ( 0 )

/**
 *  Support OEM code page 861 - Icelandic (0:Disable or 1:Enable)
 */
#define EF_CONF_CP861  ( 0 )

/**
 *  Support OEM code page 862 - Hebrew (0:Disable or 1:Enable)
 */
#define EF_CONF_CP862  ( 0 )

/**
 *  Support OEM code page 863 - Canadian French (0:Disable or 1:Enable)
 */
#define EF_CONF_CP863  ( 0 )

/**
 *  Support OEM code page 864 - Arabic (0:Disable or 1:Enable)
 */
#define EF_CONF_CP864  ( 0 )

/**
 *  Support OEM code page 865 - Nordic (0:Disable or 1:Enable)
 */
#define EF_CONF_CP865  ( 0 )

/**
 *  Support OEM code page 866 - Russian (0:Disable or 1:Enable)
 */
#define EF_CONF_CP866  ( 0 )

/**
 *  Support OEM code page 869 - Greek 2 (0:Disable or 1:Enable)
 */
#define EF_CONF_CP869  ( 0 )

/**
 *  Support OEM code page 932 - Japanese (0:Disable or 1:Enable)
 */
#define EF_CONF_CP932  ( 0 )

/**
 *  Support OEM code page 936 - Simplified Chinese (0:Disable or 1:Enable)
 */
#define EF_CONF_CP936  ( 0 )

/**
 *  Support OEM code page 949 - Korean (0:Disable or 1:Enable)
 */
#define EF_CONF_CP949  ( 0 )

/**
 *  Support OEM code page 950 - Traditional Chinese (0:Disable or 1:Enable)
 */
#define EF_CONF_CP950  ( 0 )

/**
 *  The EF_CONF_VFAT switches the support for LFN (long file name) (0:Disable or 1:Enable).
 */
#define EF_CONF_VFAT  ( 0 )

/**
 *  This option switches the working buffer for LFN support when LFN is enabled.
 *
 * EF_DEF_VFAT_BUFFER_STATIC  : static working buffer on the BSS.
 * EF_DEF_VFAT_BUFFER_STACK   : working buffer on the stack.
 *
 *  When use stack for the working buffer, take care on stack overflow.
 */
#define EF_CONF_VFAT_BUFFER  ( EF_DEF_VFAT_BUFFER_STATIC )

/**
 *  This option switches the character encoding on the API when LFN is enabled.
 *
 *   EF_DEF_API_OEM   : ANSI/OEM in current CP (TCHAR = char)
 *   EF_DEF_API_UTF8  : Unicode in UTF-8  (TCHAR = char)
 *   EF_DEF_API_UTF16 : Unicode in UTF-16 (TCHAR = ucs2_t)
 *   EF_DEF_API_UTF32 : Unicode in UTF-32 (TCHAR = ef_u32_t)
 *
 *  Also behavior of string I/O functions will be affected by this option.
 *  When LFN is not enabled, this option has no effect.
 */
#define EF_CONF_API_ENCODING ( EF_DEF_API_OEM )


/**
 *  When EF_CONF_API_ENCODING >= 1 with LFN enabled, string I/O functions, f_gets(),
 *  f_putc(), f_puts and f_printf() convert the character encoding in it.
 *  This option selects assumption of character encoding ON THE FILE to be
 *  read/written via those functions.
 *
 *   EF_DEF_FILE_IO_OEM     : ANSI/OEM in current CP
 *   EF_DEF_FILE_IO_UTF16LE : Unicode in UTF-16LE
 *   EF_DEF_FILE_IO_UTF16BE : Unicode in UTF-16BE
 *   EF_DEF_FILE_IO_UTF8    : Unicode in UTF-8
*/
#define EF_CONF_STRF_ENCODING ( EF_DEF_FILE_IO_OEM )

/**
 *  This option configures support for relative path.
 *
 *  0: Disable relative path and remove related functions.
 *  1: Enable relative path. f_chdir() and eEF_chdrive() are available.
 *  2: f_getcwd() function is available in addition to 1.
 */
#define EF_CONF_RELATIVE_PATH   ( 2 )

/* ************************************************************************* **
 *  Drive/Volume Configurations
 * ************************************************************************* */

/**
 *  Number of volumes (logical drives) to be used. (1-26)
 */
#define EF_CONF_VOLUMES_NB  ( 1 )

/**
 *  Number of drivers (disk drives) to be used. (1-26)
 */
#define EF_CONF_DRIVERS_NB  ( 2 )

/**
 *  This option switches support for fixed sector size. (0:Disable or 1:Enable)
 */
#define EF_CONF_SECTOR_SIZE_FIXED ( 1 )

/**
 *  This set of options configures the range of sector size to be supported. (512,
 *  1024, 2048 or 4096) Always set both 512 for most systems, generic memory card and
 *  harddisk. But a larger value may be required for on-board flash memory and some
 *  type of optical media. When EF_CONF_SS_MAX is larger than EF_CONF_SS_MIN, eFAT is configured
 *  for variable sector size mode and  eEFPrvDriveIOCtrl() function needs to implement
 *  GET_SECTOR_SIZE command.
 */
#define EF_CONF_SECTOR_SIZE ( 512 )

/**
 *  This option switches support for 64-bit LBA. (0:Disable or 1:Enable)
 */
#define EF_CONF_LBA64       ( 0 )

/**
 *  Minimum number of sectors to switch GPT format to create partition in f_mkfs and
 *  f_fdisk function. 0x100000000 max. This option has no effect when EF_CONF_LBA64 == 0.
 */
 #define EF_CONF_GPT_MIN    ( 0x100000000 )

/**
 *  This option switches support for GPT Partionning. (0:Disable or 1:Enable).
 *  This option needs 64-bit LBA support (EF_CONF_LBA64 == 1).
 */
#define EF_CONF_USE_GPT ( 0 )

/**
 *  This option switches support for MBR Partionning. (0:Disable or 1:Enable).
 */
#define EF_CONF_USE_MBR ( 1 )

/**
 *  This option switches support for ATA-TRIM. (0:Disable or 1:Enable)
 *  To enable Trim function, also CTRL_TRIM command should be implemented to the
 *   eEFPrvDriveIOCtrl() function.
 */
 #define EF_CONF_USE_TRIM ( 1 )

/* ************************************************************************* **
 *  System Configurations
 * ************************************************************************* */

/**
 *  This option switches support for FAT12 filesystem. (0:Disable or 1:Enable)
 */
#define EF_CONF_FS_FAT12  ( 0 )

/**
 *  This option switches support for FAT12/16 filesystem. (0:Disable or 1:Enable)
 */
#define EF_CONF_FS_FAT16  ( 0 )

/**
 *  This option switches support for FAT32 filesystem. (0:Disable or 1:Enable)
 */
#define EF_CONF_FS_FAT32  ( 1 )

/**
 *  This option switches support for checking FAT16 filesystem. (0:Disable or 1:Enable)
 *  Note that this does not enable FAT16 filesystem, only brings information
 *  back to user application. */
#define EF_CONF_FS_CHECK_FAT16  ( 1 )

/**
 *  This option switches support for checking FAT32 filesystem. (0:Disable or 1:Enable)
 *  Note that this does not enable FAT32 filesystem, only brings information
 *  back to user application. */
#define EF_CONF_FS_CHECK_FAT32  ( 1 )

/**
 *  This option switches support for timestamp function. (0:Disable or 1:Enable)
 *  If an RTC is not available nor needed disable it.
 *  Every directory entry modified by eFAT will have a fixed timestamp
 *  defined by EF_CONF_TIMESTAMP_MONTH, EF_CONF_TIMESTAMP_MDAY and EF_CONF_TIMESTAMP_YEAR in local time.
 *  When enabled, u32EFPortFatTimeGet() function need to be added to the project
 *  to read current time form real-time clock.
 *  EF_CONF_TIMESTAMP_MONTH, EF_CONF_TIMESTAMP_MDAY and EF_CONF_TIMESTAMP_YEAR have no effect.
 */
#define EF_CONF_TIMESTAMP       ( 0 )

/**
 *  This sets the Month when support for timestamp function is disabled.
 *  When used it must be in the range : 1 to 12.
 */
#define EF_CONF_TIMESTAMP_MONTH ( 1 )

/**
 *  This sets the Day of the Month when support for timestamp function is disabled.
 *  When used it must be in the range : 1 to 31.
 */
#define EF_CONF_TIMESTAMP_MDAY  ( 1 )

/**
 *  This sets the Year when support for timestamp function is disabled.
 *  When used it must be in the range : 1980 to 2107.
 */
#define EF_CONF_TIMESTAMP_YEAR  ( 2020 )

/**
 *  If you need to know correct free space on the FAT32 volume, set bit 0 of this
 *  option, and f_getfree() function at first time after volume mount will force
 *  a full FAT scan.
 *  ( 1 ) Use free cluster count in the FSINFO if available.
 *  ( 0 ) Do not trust free cluster count in the FSINFO.
 */
#define EF_CONF_USE_FAT32_FSINFO_CLUSTER_FREE      ( 0 )

/**
 *  On FAT32, controls the use of last allocated cluster number.
 *  ( 1 ) Use last allocated cluster number in the FSINFO if available.
 *  ( 0 ) Do not trust last allocated cluster number in the FSINFO.
 */
#define EF_CONF_USE_FAT32_FSINFO_CLUSTER_ALLOCATED ( 0 )

/**
 *  The option EF_CONF_FILE_LOCK switches file lock function to control duplicated file open
 *  and illegal operation to open objects.
 *
 *  0:  Disable file lock function. To avoid volume corruption, application program
 *      should avoid illegal open, remove and rename to the open objects.
 *  >0: Enable file lock function. The value defines how many files/sub-directories
 *      can be opened simultaneously under file lock control. Note that the file
 *      lock control is independent of re-entrancy.
 */
#define EF_CONF_FILE_LOCK   ( 0 )

/* #include <somertos.h>  // O/S definitions */
/**
 *  The option EF_CONF_FS_LOCK switches the re-entrancy (thread safe) of the eFAT
 *  module itself. Note that regardless of this option, file access to different
 *  volume is always re-entrant and volume control functions, f_mount(), f_mkfs()
 *  and f_fdisk() function, are always not re-entrant. Only file/directory access
 *  to the same volume is under control of this function. (0:Disable or 1:Enable)
 *
 *   0: Disable re-entrancy. EF_CONF_TIMEOUT and EF_SYNC_t have no effect.
 *   1: Enable re-entrancy. Also user provided synchronization handlers,
 *      iffSyncObjectTake(), iffSyncObjectGive(), iffSyncObjectDelete() and iffSyncObjectCreate()
 *      function, must be added to the project. Samples are available in
 *      option/syscall.c.
 */
#define EF_CONF_FS_LOCK ( 0 )

/** The EF_CONF_TIMEOUT defines timeout period in unit of time tick.
 *  The EF_SYNC_t defines O/S dependent sync object type. e.g. HANDLE, ID, OS_EVENT*,
 *  SemaphoreHandle_t and etc. A header file for O/S definitions needs to be
 *  included somewhere in the scope of ff.h.
 */
#define EF_CONF_TIMEOUT ( 1000 )

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_CONFIGURATIONS_H */
/* END OF FILE ***************************************************************************************************** */

