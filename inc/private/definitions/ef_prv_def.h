/**
 * ********************************************************************************************************************
 *  @file     ef_prv_def.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private  definitions.
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
#ifndef EFAT_PRIVATE_DEFINITIONS_H
#define EFAT_PRIVATE_DEFINITIONS_H

#ifdef __cplusplus
  extern "C" {
#endif
/* START OF FILE *************************************************************************************************** */
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */

  /**
   *  Get Sector offset in the cluster
   */
  #define EF_CLUSTER_OFFSET_GET( pxFS )  (( (ef_u32_t) pxFile->u32FileOffset / EF_SECTOR_SIZE( pxFS ) ) \
                                         & ( (ef_u32_t) pxFS->u8ClstSize - 1 ))

/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */
/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
#include <efat.h>
/* Public constant macros -------------------------------------------------------------------------------------------------------------- */

#define EF_FILE_SIZE_MAX  ( 0xFFFFFFFE )  /**< Maximum file size */

/* Additional file access control and file status flags for internal use */
//#define EF_FA_SEEKEND  ( 0x20 )  /**< Seek to end of the file on file open */
#define EF_FILE_MODIFIED  ( 0x40 )  /**< File has been modified */
#define EF_FILE_WIN_DIRTY ( 0x80 )  /**< ef_file_st.buf[] needs to be written-back */

#define EF_FS_WIN_DIRTY   ( 0x01 )  /**< disk access window needs to be written-back */

/* File attribute bits for directory entry (ef_file_info_st.u8Attrib) */
#define EF_DIR_ATTRIB_BIT_READONLY  ( 0x01 )  /**< Read only */
#define EF_DIR_ATTRIB_BIT_HIDDEN    ( 0x02 )  /**< Hidden */
#define EF_DIR_ATTRIB_BIT_SYSTEM    ( 0x04 )  /**< System */
#define EF_DIR_ATTRIB_BIT_VOLUME_ID ( 0x08 )  /**< Volume Label */
#define EF_DIR_ATTRIB_BIT_DIRECTORY ( 0x10 )  /**< Directory */
#define EF_DIR_ATTRIB_BIT_ARCHIVE   ( 0x20 )  /**< Archive */
/* Additional file attribute bits for internal use */
#define EF_DIR_ATTRIB_BITS_LFN      ( 0x0F )  /**< LFN entry */
#define EF_DIR_ATTRIB_BITS_DEFINED  ( 0x3F )  /**< Mask of defined bits */

/* Name status flags in u8Name[11] */
#define EF_NSFLAG     ( 11 )    /**< Index of the name status byte */
#define EF_NS_LOSS    ( 0x01 )  /**< Out of 8.3 format */
#define EF_NS_LFN     ( 0x02 )  /**< Force to create LFN entry */
#define EF_NS_LAST    ( 0x04 )  /**< Last segment */
#define EF_NS_BODY    ( 0x08 )  /**< Lower case statusFlags (body) */
#define EF_NS_EXT     ( 0x10 )  /**< Lower case statusFlags (ext) */
#define EF_NS_DOT     ( 0x20 )  /**< Dot entry */
#define EF_NS_NOLFN   ( 0x40 )  /**< Do not find LFN */
#define EF_NS_NONAME  ( 0x80 )  /**< Not followed */

/* eFAT refers the FAT structure as simple byte array instead of structure member
/ because the C structure is not binary compatible between different platforms */
#define EF_DIR_NAME_START         (  0 )    /**< Short file name (11-byte) */
#define EF_DIR_ATTRIBUTES         ( 11 )    /**< Attribute (ef_u08_t) */
#define EF_DIR_LFN_NTres          ( 12 )    /**< Lower case statusFlags (ef_u08_t) */
//#define DIR_CrtTime10             ( 13 )    /**< Created time sub-second (ef_u08_t) */
#define EF_DIR_TIME_CREATED       ( 14 )    /**< Created time (ef_u32_t) */
#define EF_DIR_DATE_LAST_ACCESS   ( 18 )    /**< Last accessed date (ef_u16_t) */
#define EF_DIR_FIRST_CLUSTER_HI   ( 20 )    /**< Higher 16-bit of first cluster (ef_u16_t) */
#define EF_DIR_TIME_MODIFIED      ( 22 )    /**< Modified time (ef_u32_t) */
#define EF_DIR_FIRST_CLUSTER_LOW  ( 26 )    /**< Lower 16-bit of first cluster (ef_u16_t) */
#define EF_DIR_FILE_SIZE          ( 28 )    /**< File size (ef_u32_t) */

#define EF_DIR_LFN_ORDER          ( 0  )   /**< LFN: LFN order and LLE statusFlags (ef_u08_t) */
#define EF_DIR_LFN_TYPE           ( 12 )    /**< LFN: Entry type (ef_u08_t) */
#define EF_DIR_LFN_CHECKSUM       ( 13 )    /**< LFN: Checksum of the SFN (ef_u08_t) */
//#define EF_DIR_FIRST_CLUSTER_LOW  ( 26 )    /**< LFN: MBZ field (ef_u16_t) */

#define EF_DIR_LFN_LAST           ( 0x40 )  /**< Last long entry statusFlags in EF_DIR_LFN_ORDER */

#define EF_DIR_ENTRY_SIZE        (   32 )  /**< Size of a directory entry */
#define EF_DIR_DELETED_MASK      ( 0xE5 )  /**< Deleted directory entry mark set to EF_DIR_NAME_START[0] */
#define EF_DIR_REPLACEMENT_CHAR  ( 0x05 )  /**< Replacement of the character collides with EF_DIR_DELETED_MASK */

/* Re-entrancy related */
#if ( ( 0 != EF_CONF_FS_LOCK ) && ( 0 != EF_CONF_VFAT ) )
  #error Static LFN work area cannot be used at thread-safe configuration
#endif


/* Definitions of sector size */
#if ( EF_CONF_SECTOR_SIZE != 512 ) && ( EF_CONF_SECTOR_SIZE != 1024 ) && ( EF_CONF_SECTOR_SIZE != 2048 ) && ( EF_CONF_SECTOR_SIZE != 4096 )
  #error Wrong sector size configuration
#endif
#if ( 0 == EF_CONF_SECTOR_SIZE_FIXED )
  #define EF_SECTOR_SIZE(fs)  ((fs)->u16SecSize)  /**< Variable sector size */
#else
  #define EF_SECTOR_SIZE(fs)  ((ef_u32_t)EF_CONF_SECTOR_SIZE)  /**< Fixed sector size */
#endif

/* Timestamp */
#if ( 0 == EF_CONF_TIMESTAMP )
  #if ( EF_CONF_TIMESTAMP_YEAR < 1980 ) || ( EF_CONF_TIMESTAMP_YEAR > 2107 )
    #error Invalid setting of EF_CONF_TIMESTAMP_YEAR
  #endif
  #if ( EF_CONF_TIMESTAMP_MONTH < 1 ) || ( EF_CONF_TIMESTAMP_MONTH > 12 )
    #error Invalid setting of EF_CONF_TIMESTAMP_MONTH
  #endif
  #if ( EF_CONF_TIMESTAMP_MDAY < 1 ) || ( EF_CONF_TIMESTAMP_MDAY > 31 )
    #error Invalid setting of EF_CONF_TIMESTAMP_MDAY
  #endif
  /**
   *  This define what we use to get current RTC time
   */
  #define EF_FATTIME_GET()  ((ef_u32_t)(EF_CONF_TIMESTAMP_YEAR - 1980) << 25 | (ef_u32_t)EF_CONF_TIMESTAMP_MONTH << 21 | (ef_u32_t)EF_CONF_TIMESTAMP_MDAY << 16)
#else
  #define EF_FATTIME_GET()  u32EFPortFatTimeGet()
#endif

/**
 *  This define return code handler function switched by EF_CONF_CODE_COVERAGE
 *  eEFPrvPortReturnCodeHandler is declared in ef_port_system.c
 */
//#if ( 0 != EF_CONF_RETURN_CODE_HANDLER )
  #define EF_RETURN_CODE_HANDLER( error_code) eEFPrvPortReturnCodeHandler(error_code, #error_code, __FILE_NAME__, __LINE__)
//#else
//  #define EF_RETURN_CODE_HANDLER(error_code) (error_code)
//#endif

  /**
 *  This define assertion function switched by EF_CONF_ASSERT_PUBLIC
 */
#if ( 0 != EF_CONF_ASSERT_PUBLIC )
  #define EF_ASSERT_PUBLIC(expression) \
    if (!(expression)) { return EF_RETURN_CODE_HANDLER( EF_RET_PUBLIC_ASSERT ); }
#else
  #define EF_ASSERT_PUBLIC(expression)
#endif

/**
 *  This define assertion function switched by EF_CONF_ASSERT_PRIVATE
 *  eEFPrvPortAssertFailed is declared in ef_port_system.c
 */
#if ( 0 != EF_CONF_ASSERT_PRIVATE )
  #define EF_ASSERT_PRIVATE(expression) \
    if (!(expression)) { return eEFPrvPortAssertFailed(__FILE__, __LINE__); }
#else
  #define EF_ASSERT_PRIVATE(expression)
#endif

/**
 *  This define code coverage function switched by EF_CONF_CODE_COVERAGE
 *  eEFPrvPortAssertFailed is declared in ef_port_system.c
 */
#if ( 0 != EF_CONF_CODE_COVERAGE )
  #define EF_CODE_COVERAGE( ) \
    if (!(expression)) { return eEFPrvPortAssertFailed(__FILE__, __LINE__); }
#else
  #define EF_CODE_COVERAGE()
#endif


/* Public function macros -------------------------------------------------------------------------------------------------------------- */
/* Public typedefs, structures, unions and enums --------------------------------------------------------------------------------------- */

/**
 *  @brief  Filesystem object structure (ef_fs_st)
 */
typedef struct ef_fs_strut {
  ef_u08_t    u8LogicNumber;          /**< Logic drive number (-1:not mounted) */
  ef_u08_t    u8FsType;               /**< Filesystem type (0:not mounted) */
  ef_u08_t    u8PhysDrv;              /**< Associated physical drive */
  ef_u08_t    u8Partition;            /**< Associated partition on drive ( 0 - 128 if properly mounted ) */
  ef_u08_t    u8MountInfos;           /**< Bit 7: ?, Bit 6: ?, Bit 5: ?,  Bit 4: ?, Bits 3-0: File system number */
  ef_u08_t    u8FatsNb;               /**< Number of FATs (1 or 2) */
  ef_u16_t    u16MountId;             /**< Volume mount ID */
  ef_u16_t    u16RootDirNb;           /**< Number of root directory entries (FAT12/16) */
  ef_u08_t    u8ClstSize;             /**< Cluster size in sectors */
  ef_u16_t    u16SecSize;             /**< Sector size in bytes (512, 1024, 2048 or 4096) */
//#if ( 0 != EF_CONF_VFAT )
//  ucs2_t *    pxLFNBuffer;            /**< LFN working buffer */
//#else
//  ucs2_t *    pxLFNBuffer;            /**< LFN working buffer */
//#endif
#if ( 0 != EF_CONF_FS_LOCK )
  EF_SYNC_t   xSyncObject;            /**< Identifier of sync object */
#else
  EF_SYNC_t   xSyncObject;            /**< Identifier of sync object */
#endif
  ef_u32_t    u32ClstLast;            /**< Last allocated cluster */
  ef_u32_t    u32ClstFreeNb;          /**< Number of free clusters */
#if ( 0 != EF_CONF_RELATIVE_PATH )
  ef_u32_t    u32DirClstCurrent;      /**< Current directory start cluster (0:root) */
#else
  ef_u32_t    u32DirClstCurrent;      /**< Current directory start cluster (0:root) */
#endif
  ef_u08_t    u8FsInfoFlags;          /**< FSINFO flags (b7:disabled, b0:dirty) only for FAT32 */
  ef_u32_t    u32FatEntriesNb;        /**< Number of FAT entries (number of clusters + 2) */
  ef_u32_t    u32FatSize;             /**< Size of an FAT [sectors] */
  ef_lba_t    xVolBase;               /**< Volume base sector */
  ef_lba_t    xFatBase;               /**< FAT base sector */
  ef_lba_t    xDirBase;               /**< Root directory base sector/cluster */
  ef_lba_t    xDataBase;              /**< Data base sector */
  ef_lba_t    xWindowSector;          /**< Current sector appearing in the u8Window[] */
  ef_u08_t  * pu8Window;              /**< Pointer to Disk access window for Directory & FAT */
  ef_u32_t    u32WinSize;             /**< Size of the Disk access window [bytes] */
  ef_u08_t    u8WinFlags;             /**< u8Window[] u8StatusFlags (b0:dirty) */
  ef_u08_t  * pu8FATWindow;           /**< Pointer to Disk access window for FAT */
  ef_u32_t    u32FATWinSize;          /**< Size of the Disk access window [bytes] */
  ef_u08_t    u8FATWinFlags;          /**< u8Window[] u8StatusFlags (b0:dirty) */
} ef_fs_st;

/**
 *  @brief  Object ID and allocation information (ef_object_st)
 */
typedef struct ef_object_struct {
  ef_fs_st  * pxFS;         /**< Pointer to the hosting volume of this object */
  ef_u16_t    u16MountId;   /**< Hosting volume mount ID */
  ef_u08_t    u8Attrib;     /**< Object attribute */
  ef_u32_t    u32ClstStart; /**< Object data start cluster (0:no cluster or root directory) */
  ef_u32_t    u32LockId;    /**< File lock ID origin from 1 (index of file semaphore table Files[]) */
} ef_object_st;

/**
 *  @brief  File object structure (ef_file_st)
 */
typedef struct ef_file_struct {
  ef_object_st  xObject;                          /**< Object identifier */
  ef_u08_t      u8StatusFlags;                    /**< File status flags */
  ef_u08_t      u8ErrorCode;                      /**< Abort u8StatusFlags (error code) */
  ef_u32_t      u32FileOffset;                    /**< File read/write pointer (Zeroed on file open) */
  ef_u32_t      u32Clst;                          /**< Current cluster of u32FileOffset (invalid when u32FileOffset is 0) */
//  ef_u08_t     u8ClusterOffset;                  /**< Current sector offset in cluster of u32FileOffset (invalid when u32FileOffset is 0) */
  ef_u32_t      u32Size;                          /**< File size (valid when 0 != xObject.u32ClstStart ) */
  ef_lba_t      xSector;                          /**< Sector number appearing in u8Window[ ] (0:invalid) */
  ef_u08_t      u8Window[ EF_CONF_SECTOR_SIZE ];  /**< File private data read/write window */
  ef_lba_t      xDirSector;                       /**< Sector number containing the directory entry */
  ef_u08_t    * pu8DirPtr;                        /**< Pointer to the directory entry in the window[] */
} ef_file_st;

/**
 *  @brief  Directory object structure (ef_directory_st)
 */
typedef struct ef_directory_struct {
  ef_object_st  xObject;      /**< Object identifier */
  ef_u32_t      u32Offset;    /**< Current read/write offset */
  ef_u32_t      u32Clst;      /**< Current cluster */
  ef_lba_t      xSector;      /**< Current sector (0:Read operation has terminated) */
  ef_u08_t    * pu8Dir;       /**< Pointer to the directory item in the u8Window[] */
  ef_u08_t      u8Name[ 12 ]; /**< SFN (in/out) {body[8],ext[3],status[1]} */
//#if ( 0 != EF_CONF_VFAT )
  ef_u32_t      u32BlkOffset; /**< Offset of current entry block being processed (0xFFFFFFFF:Invalid) */
//#endif
#if ( 0 != EF_CONF_USE_FIND )
  const TCHAR * pxPattern;    /**< Pointer to the name matching pattern */
#endif
} ef_directory_st;

/* Public variables ------------------------------------------------------------------------------------------------ */
/* Public function prototypes--------------------------------------------------------------------------------------- */

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_DEFINITIONS_H */
/* END OF FILE ***************************************************************************************************** */

