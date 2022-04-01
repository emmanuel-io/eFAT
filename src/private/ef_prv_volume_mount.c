/**
 * ********************************************************************************************************************
 *  @file     ef_prv_volume_mount.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private volume mounting functions.
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
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include <ef_port_load_store.h>
#include <efat.h>
#include <ef_prv_def.h>
#include <ef_prv_def_gpt.h>
#include <ef_prv_fat.h>
#include "ef_prv_drive.h"
#include "ef_prv_def.h"
#include "ef_prv_directory.h"
#include "ef_prv_dirfunc.h"
#include "ef_prv_fs_window.h"
#include "ef_prv_lock.h"
#include "ef_prv_string.h"
#include "ef_prv_volume.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_unicode.h"
#include "ef_prv_validate.h"
#include "ef_prv_volume_nb.h"
#include "ef_prv_volume.h"
#include "ef_prv_def_mbr.h"
#include "ef_prv_def_bpb_fat.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */

/**
 * Max FAT12 clusters (differs from specs, but right for real DOS/Windows behavior)
 */
#define EF_CLUSTER_NB_MAX_FAT12  ( 0x0FF5 )

/**
 * Max FAT16 clusters (differs from specs, but right for real DOS/Windows behavior)
 */
#define EF_CLUSTER_NB_MAX_FAT16  ( 0xFFF5 )

/**
 * Max FAT32 clusters (not specified, practical limit)
 */
#define EF_CLUSTER_NB_MAX_FAT32  ( 0x0FFFFFF5 )


/* Directory entry block scratchpad buffer */
#if ( 0 != EF_CONF_VFAT )
static ef_u08_t  DirBuf[ MAXDIRB( EF_LFN_UNITS_MAX ) ];
#endif
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */

/**
 * BIOS Parameter Block informations of interest
 */
typedef struct bpb_fat_struct
{
  ef_u16_t u16SectorSize;              /**< Size of a sector in bytes */
  ef_u08_t u8ClusterSize;              /**< Size of a cluster in sectors */
  ef_u16_t u16ReservedSectorsCount;    /**< Number of reserved sectors */
  ef_u08_t u8FATCount;                 /**< Number of FATs (1 or 2) */
  ef_u16_t u16RootEntriesCount;        /**< Number of directory entries */
  ef_u16_t u16TotalSectorsCount;       /**< Number of Sectors on the volume (if < 65536) */
  ef_u08_t u8MediaType;                /**< Media descriptor type */
  ef_u16_t u16FATSize;                 /**< FAT Size in Sectors */
  ef_u16_t u16TrackSize;               /**< Track Size in Sectors */
  ef_u16_t u16HeadsSidesCount;         /**< Number of heads or sides */
  ef_u32_t u32HiddenSectorsCount;      /**< Number of hidden sectors */
  ef_u32_t u32TotalSectorsLargeCount;  /**< Number of Sectors on the volume (if >= 65536) */
}__attribute__((packed)) bpb_fat_st;

/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/**
 *  Pointers to the filesystem objects (logical drives)
 */
static  ef_fs_st * pxeFAT[ EF_CONF_VOLUMES_NB ];

/**
 *  Filesystem mount ID
 */
static ef_u16_t Fsid = 0;

/* Public variables ------------------------------------------------------------------------------------------------ */

/* Local function prototypes---------------------------------------------------------------------------------------- */

ef_return_et eEFPrvVolumeMountFinishFAT12 (
  ef_fs_st *  pxFS
);

ef_return_et eEFPrvVolumeMountFinishFAT16 (
  ef_fs_st *  pxFS
);

ef_return_et eEFPrvVolumeMountFinishFAT32 (
  ef_fs_st *  pxFS
);


/**
 *  @brief  Get File system type (FAT12 FAT16 FAT32) from the number of Clusters
 *
 *  @param  pu8Format       The calculated Filesystem type from the number of clusters
 *  @param  pu32ClustersNb  Pointer to the number of clusters to return
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
static  ef_return_et  eEFPrvClustersNbToFSType (
  ef_u32_t  u32ClustersNb,
  ef_u08_t * pu8Format
);

/**
 *  @brief  Analyze BPB + EBPB Data from boot sector & update filesystem object
 *
 *  @param  pxFS  Pointer to the filesystem object
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
static  ef_return_et eEFPrvBPBFATAnalyze (
  ef_fs_st  * pxFS
);


/* Local functions ------------------------------------------------------------------------------------------------- */

/* Get FS Type from Clusters Number */
static  ef_return_et  eEFPrvClustersNbToFSType (
  ef_u32_t  u32ClustersNb,
  ef_u08_t * pu8Format
)
{
  EF_ASSERT_PRIVATE( 0 != pu8Format );

  ef_return_et  eRetVal = EF_RET_OK;
  if ( 0 == u32ClustersNb )
  {
    *pu8Format = 0;
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else if ( EF_CLUSTER_NB_MAX_FAT12 >= u32ClustersNb )
  {
    *pu8Format = EF_FS_FAT12;
  }
  else if ( EF_CLUSTER_NB_MAX_FAT16 >= u32ClustersNb )
  {
    *pu8Format = EF_FS_FAT16;
  }
  else if ( EF_CLUSTER_NB_MAX_FAT32 >= u32ClustersNb )
  {
    *pu8Format = EF_FS_FAT32;
  }
  else
  {
    *pu8Format = 0;
    eRetVal = EF_RET_ERROR;
  }
  return eRetVal;
}


ef_return_et eEFPrvVolumeMountFinishFAT12 (
  ef_fs_st *  pxFS
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_OK;

  if ( 0 == pxFS->u16RootDirNb )
  {
    /* (EF_BS_BPB_FAT_OFFSET_ROOT_ENTRIES must not be 0) */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  /* Root directory start sector */
  pxFS->xDirBase = pxFS->xFatBase + pxFS->u32FatSize;
  /* (Needed FAT size) */
  ef_u32_t  u32FATSizeBytes = pxFS->u32FatEntriesNb;
  u32FATSizeBytes *= 3;
  u32FATSizeBytes /= 2;
  u32FATSizeBytes += ( pxFS->u32FatEntriesNb & 1 );
  u32FATSizeBytes += (ef_u32_t) EF_SECTOR_SIZE( pxFS ) - 1;
  u32FATSizeBytes /= (ef_u32_t) EF_SECTOR_SIZE( pxFS );
  if ( pxFS->u32FatSize < u32FATSizeBytes )
  {
    /* (BPB_FATSz must not be less than the size needed) */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  /* Get FSInfo if available */
  /* Initialize cluster allocation information */
  pxFS->u32ClstLast   = 0xFFFFFFFF;
  pxFS->u32ClstFreeNb = 0xFFFFFFFF;
  pxFS->u8FsInfoFlags = 0x80;

  return eRetVal;
}


ef_return_et eEFPrvVolumeMountFinishFAT16 (
  ef_fs_st *  pxFS
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_OK;

  if ( 0 == pxFS->u16RootDirNb )
  {
    /* (EF_BS_BPB_FAT_OFFSET_ROOT_ENTRIES must not be 0) */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  /* Root directory start sector */
  pxFS->xDirBase = pxFS->xFatBase + pxFS->u32FatSize;
  /* (Needed FAT size) */
  ef_u32_t  u32FATSizeBytes = pxFS->u32FatEntriesNb;
  u32FATSizeBytes *= 2;
  u32FATSizeBytes += (ef_u32_t) EF_SECTOR_SIZE( pxFS ) - 1;
  u32FATSizeBytes /= (ef_u32_t) EF_SECTOR_SIZE( pxFS );
  if ( pxFS->u32FatSize < u32FATSizeBytes )
  {
    /* (BPB_FATSz must not be less than the size needed) */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }

  /* Get FSInfo if available */
  /* Initialize cluster allocation information */
  pxFS->u32ClstLast   = 0xFFFFFFFF;
  pxFS->u32ClstFreeNb = 0xFFFFFFFF;
  pxFS->u8FsInfoFlags = 0x80;

  return eRetVal;
}


ef_return_et eEFPrvVolumeMountFinishFAT32 (
  ef_fs_st *  pxFS
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_OK;
//  ef_u32_t            SectorsNb;
//  ef_u32_t            u32FATSize;
//  ef_u32_t            szbfat;
//  ef_u16_t            u16ReservedSectors;

  ef_u32_t  u32FATSizeBytes = pxFS->u32FatEntriesNb;

  if ( 0 != pxFS->u16RootDirNb )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILESYSTEM );
  }
  pxFS->xDirBase = u32EFPortLoad( pxFS->pu8Window + EF_BS_EBPB_FAT32_OFFSET_ROOT_DIRECTORY_NB );  /* Root directory start sector */
  u32FATSizeBytes *= 4;
  u32FATSizeBytes += (ef_u32_t) EF_SECTOR_SIZE( pxFS ) - 1;
  u32FATSizeBytes /= (ef_u32_t) EF_SECTOR_SIZE( pxFS );
  if ( pxFS->u32FatSize < u32FATSizeBytes )
  {
    /* (BPB_FATSz must not be less than the size needed) */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }

  if ( EF_SECTOR_SIZE( pxFS ) != u16EFPortLoad( pxFS->pu8Window + EF_BS_BPB_FAT_OFFSET_SECTOR_SIZE ) )
  {
    /* (EF_BS_BPB_FAT_OFFSET_SECTOR_SIZE must be equal to the physical sector size) */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }

  //  ef_u16_t  u16Value = u16EFPortLoad( pxFS->pu8Window + EF_BS_EBPB_FAT32_OFFSET_FAT_VERSION );
  //  if ( 0 != u16Value )
  if ( 0 != u16EFPortLoad( pxFS->pu8Window + EF_BS_EBPB_FAT32_OFFSET_FAT_VERSION ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILESYSTEM );
//    return EF_RET_NO_FILESYSTEM;  /* (Must be FAT32 revision 0.0) */
  }

  /* Get FSInfo if available */
  /* Initialize cluster allocation information */
  pxFS->u32ClstLast   = 0xFFFFFFFF;
  pxFS->u32ClstFreeNb = 0xFFFFFFFF;
  pxFS->u8FsInfoFlags = 0x80;

  if (    ( 0 != EF_CONF_USE_FAT32_FSINFO_CLUSTER_FREE )
       || ( 0 != EF_CONF_USE_FAT32_FSINFO_CLUSTER_ALLOCATED ) )
  {
    /* Allow to update FSInfo only if EF_BS_EBPB_FAT32_OFFSET_FS_INFO_SECTOR == 1 */
    if (    ( 1 == u16EFPortLoad( pxFS->pu8Window + EF_BS_EBPB_FAT32_OFFSET_FS_INFO_SECTOR ) )
         && ( EF_RET_OK == eEFPrvFSWindowLoad( pxFS, pxFS->xVolBase + 1 ) ) )
    {
      pxFS->u8FsInfoFlags = 0;
      if (    ( 0xAA55 == ( u16EFPortLoad( pxFS->pu8Window + EF_BS_OFFSET_SIGNATURE ) ) )  /* Load FSInfo data if available */
           && ( 0x41615252 == u32EFPortLoad( pxFS->pu8Window + EF_BS_FAT32_FSI_OFFSET_SIGNATURE_LEAD ) )
           && ( 0x61417272 == u32EFPortLoad( pxFS->pu8Window + EF_BS_FAT32_FSI_OFFSET_SIGNATURE_NEXT ) ) )
      {
        if ( 0 != EF_CONF_USE_FAT32_FSINFO_CLUSTER_FREE )
        {
          pxFS->u32ClstFreeNb = u32EFPortLoad( pxFS->pu8Window + EF_BS_FAT32_FSI_OFFSET_FREE_CLUSTERS );
        }
        if ( 0 != EF_CONF_USE_FAT32_FSINFO_CLUSTER_ALLOCATED )
        {
          pxFS->u32ClstLast = u32EFPortLoad( pxFS->pu8Window + EF_BS_FAT32_FSI_OFFSET_CLUSTER_LAST_ALLOC );
        }
      }
    }
  }

  return eRetVal;
}


static  ef_return_et eEFPrvBPBFATAnalyze (
  ef_fs_st *  pxFS
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  bpb_fat_st    xBPBFAT;
  ef_u08_t   * pu8BootSector = pxFS->pu8Window;
  ef_return_et  eRetVal = EF_RET_ERROR;

  /* Size of a sector in bytes */
  xBPBFAT.u16SectorSize = u16EFPortLoad( pu8BootSector + EF_BS_BPB_FAT_OFFSET_SECTOR_SIZE );
  /* Size of a cluster in sectors */
  xBPBFAT.u8ClusterSize = pu8BootSector[ EF_BS_BPB_FAT_OFFSET_CLUSTER_SIZE ];
  /* Number of reserved sectors */
  xBPBFAT.u16ReservedSectorsCount = u16EFPortLoad( pu8BootSector + EF_BS_BPB_FAT_OFFSET_SECTORS_RESERVED_NB );
  /* Number of FATs (1 or 2) */
  xBPBFAT.u8FATCount = pu8BootSector[ EF_BS_BPB_FAT_OFFSET_FATS_NB ];
  /* Number of directory entries */
  xBPBFAT.u16RootEntriesCount = u16EFPortLoad( pu8BootSector + EF_BS_BPB_FAT_OFFSET_ROOT_ENTRIES );
  /* Number of Sectors on the volume (if < 65536) */
  xBPBFAT.u16TotalSectorsCount = u16EFPortLoad( pu8BootSector + EF_BS_BPB_FAT_OFFSET_SECTORS_COUNT );
  /* Media descriptor type */
  xBPBFAT.u8MediaType = pu8BootSector[ EF_BS_BPB_FAT_OFFSET_MEDIA_DESCRIPTOR ];
  /* FAT Size in Sectors */
  xBPBFAT.u16FATSize = u16EFPortLoad( pu8BootSector + EF_BS_BPB_FAT_OFFSET_FAT16_SIZE );
  /* Track Size in Sectors */
  xBPBFAT.u16TrackSize = u16EFPortLoad( pu8BootSector + EF_BS_BPB_FAT_OFFSET_TRACK_SIZE );
  /* Number of heads or sides */
  xBPBFAT.u16HeadsSidesCount = u16EFPortLoad( pu8BootSector + EF_BS_BPB_FAT_OFFSET_HEADS_NB );
  /* Number of hidden sectors */
  xBPBFAT.u32HiddenSectorsCount = u32EFPortLoad( pu8BootSector + EF_BS_BPB_FAT_OFFSET_SECTORS_HIDDEN_NB );
  /* Number of Sectors on the volume (if >= 65536) */
  xBPBFAT.u32TotalSectorsLargeCount = u32EFPortLoad( pu8BootSector + EF_BS_BPB_FAT_OFFSET_SECTORS_LARGE_COUNT );

  /* Size of a sector in bytes */
  if ( xBPBFAT.u16SectorSize != EF_SECTOR_SIZE( pxFS ) )
  {
    ; /* (EF_BS_BPB_FAT_OFFSET_SECTOR_SIZE must be equal to the physical sector size) */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  /* Number of FATs */
  else if ( ( 1 != xBPBFAT.u8FATCount ) && ( 2 != xBPBFAT.u8FATCount ) )
  {
    ; /* We have an error (Must be 1 or 2) */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  /* Cluster size */
  else if ( 0 == xBPBFAT.u8ClusterSize )
  {
    ; /* We have an error (Must not be zero ) */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else if ( 0 != ( xBPBFAT.u8ClusterSize & ( xBPBFAT.u8ClusterSize - 1 ) ) )
  {
    ; /* We have an error (Must be power of 2) */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else if ( 0 != ( xBPBFAT.u16RootEntriesCount % ( EF_SECTOR_SIZE( pxFS ) / EF_DIR_ENTRY_SIZE ) ) )
  {
    ; /* We have an error (Must be sector aligned) */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else if ( 0 == xBPBFAT.u16ReservedSectorsCount )
  {
    ; /* We have an error (Must not be 0) */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {

    /* Update Filesystem object */
    pxFS->u8FatsNb = pu8BootSector[ EF_BS_BPB_FAT_OFFSET_FATS_NB ];
    /* Update Filesystem object */
    pxFS->u8ClstSize = pu8BootSector[ EF_BS_BPB_FAT_OFFSET_CLUSTER_SIZE ];
    /* Update Filesystem object */
    pxFS->u16RootDirNb = u16EFPortLoad( pu8BootSector + EF_BS_BPB_FAT_OFFSET_ROOT_ENTRIES );

    /* Number of sectors for FAT area */
    ef_u32_t  u32FATSize = (ef_u32_t) xBPBFAT.u16FATSize;
    /* Number of sectors per FAT */
    if ( 0 == u32FATSize )
    {
      u32FATSize = u32EFPortLoad( pu8BootSector + EF_BS_EBPB_FAT32_OFFSET_FAT_SIZE );
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
    /* Update Filesystem object */
    pxFS->u32FatSize = u32FATSize;
    u32FATSize *= pxFS->u8FatsNb;

    /* Total number of sectors for FAT area */
    ef_u32_t  u32SectorsCount = (ef_u32_t) xBPBFAT.u16TotalSectorsCount;
    if ( 0 == u32SectorsCount )
    {
      u32SectorsCount = xBPBFAT.u32TotalSectorsLargeCount;
    }
    else
    {
      EF_CODE_COVERAGE( );
    }

    eRetVal = EF_RET_OK;

    /*
     *  Determine the clusters number to later extract FAT sub type
     */
    /* RSV + FAT + ef_directory_st */
    ef_u32_t  u32sysect =  (ef_u32_t) xBPBFAT.u16ReservedSectorsCount
                          + u32FATSize
                          + (ef_u32_t) pxFS->u16RootDirNb / (  (ef_u32_t) EF_SECTOR_SIZE( pxFS )
                                                              / (ef_u32_t) EF_DIR_ENTRY_SIZE );
    if ( u32SectorsCount < u32sysect )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    else
    {
      EF_CODE_COVERAGE( );
    }

    /* Number of clusters */
    ef_u32_t  u32ClustersNb = ( u32SectorsCount - u32sysect ) / xBPBFAT.u8ClusterSize;

    /* Determine the FAT sub type */
    if ( EF_RET_OK != eEFPrvClustersNbToFSType( u32ClustersNb, &(pxFS->u8FsType) ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    else
    {
      EF_CODE_COVERAGE( );
    }

    /* Boundaries and Limits */

    /* Number of FAT entries */
    pxFS->u32FatEntriesNb = u32ClustersNb + 2;
    /* Volume start sector */
    pxFS->xVolBase        = pxFS->xWindowSector;
    /* FAT start sector */
    pxFS->xFatBase        = pxFS->xVolBase + xBPBFAT.u16ReservedSectorsCount;
    /* Data start sector */
    pxFS->xDataBase       = pxFS->xVolBase + u32sysect;

    ef_u32_t  u32szbfat = pxFS->u32FatEntriesNb;

    if ( 0 != ( EF_FS_FAT32 & pxFS->u8FsType ) )
    {
      if ( 0 != pxFS->u16RootDirNb )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILESYSTEM );
      }
      else
      {
        /* Root directory start sector */
        pxFS->xDirBase = u32EFPortLoad( pu8BootSector + EF_BS_EBPB_FAT32_OFFSET_ROOT_DIRECTORY_NB );
        u32szbfat *= 4;
      }
    }
    else if ( 0 != ( EF_FS_FAT16 & pxFS->u8FsType ) )
    {
      if ( 0 == pxFS->u16RootDirNb )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILESYSTEM );
      }
      else
      {
        pxFS->xDirBase = pxFS->xFatBase + u32FATSize;   /* Root directory start sector */
        u32szbfat *= 2;
      }
    }
    else if ( 0 != ( EF_FS_FAT12 & pxFS->u8FsType ) )
    {
      if ( 0 == pxFS->u16RootDirNb )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILESYSTEM );
      }
      else
      {
        u32szbfat =   ( ( u32szbfat * 3 ) / 2 )
                    + ( u32szbfat & 1 );
      }
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
    u32szbfat =   ( u32szbfat + ( (ef_u32_t) EF_SECTOR_SIZE( pxFS ) - 1 ) ) / (ef_u32_t) EF_SECTOR_SIZE( pxFS );

    if ( pxFS->u32FatSize < u32szbfat )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      /* (BPB_FATSz must not be less than the size needed) */
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
  }

  return eRetVal;
}

/* Public functions prototypes --------------------------------------------- */

ef_return_et eEFPrvVolumeMount (
  ef_fs_st *  pxFS,
  ef_u08_t   u8ReadOnly
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_INVALID_DRIVE;

  /* Following code attempts to mount the volume.
   * (find a FAT volume, analyze the BPB and initialize the filesystem object)
   */
  ef_u08_t xStatus;

  /* Initialize the physical drive */
  xStatus =  eEFPrvDriveInitialize( pxFS->u8PhysDrv );
  /* Check if the initialization succeeded */
  if ( 0 != ( xStatus & EF_RET_DISK_NOINIT ) )
  {
    /* Failed to initialize due to no medium or hard error */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NOT_READY );
  }
  /* Check disk write protection if needed */
  else if (    ( 0 == u8ReadOnly )
            && ( 0 != ( xStatus & EF_RET_DISK_PROTECT ) ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_WRITE_PROTECTED );
  }
  /* Get sector size (multiple sector size cfg only) */
  else if (    ( 0 == EF_CONF_SECTOR_SIZE_FIXED )
            && ( EF_RET_OK !=  eEFPrvDriveIOCtrl( pxFS->u8PhysDrv,
                                                    GET_SECTOR_SIZE,
                                                    &(pxFS->u16SecSize) ) ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
  }
  else if (    ( EF_CONF_SECTOR_SIZE < EF_SECTOR_SIZE( pxFS ) )
            || ( 0 != ( EF_SECTOR_SIZE( pxFS ) & ( EF_SECTOR_SIZE( pxFS ) - 1 ) ) ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
  }
  else
  {
    /* Find an FAT volume on the drive */
    ef_sector_type_et eSectorType = eEFPrvVolumeFind( pxFS, pxFS->u8Partition );
    if ( EF_SECTOR_TYPE_DISK_ERROR == eSectorType )
    {
      /* An error occured in the disk I/O layer */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
    }
    else if (    ( EF_SECTOR_TYPE_BS_UNKNOWN == eSectorType )
              || ( EF_SECTOR_TYPE_BS_INVALID == eSectorType )
              || ( EF_SECTOR_TYPE_PROTECTIVE_MBR == eSectorType ) )
    {
      /* No Filesystem volume is found */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILESYSTEM );
    }
    else if (    (    ( 0 != EF_FS_FAT12 )
                   && ( EF_SECTOR_TYPE_VBR_FAT16 == eSectorType ) )
              || (    ( 0 != EF_FS_FAT16 )
                   && ( EF_SECTOR_TYPE_VBR_FAT16 == eSectorType ) )
              || (    ( 0 != EF_FS_FAT32 )
                   && ( EF_SECTOR_TYPE_VBR_FAT32 == eSectorType ) ) )
    {
      if ( EF_RET_OK != eEFPrvBPBFATAnalyze( pxFS ) )
      {
        /* Something went wrong when parsing the Boot sector */
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILESYSTEM );
      }
      else if (    ( 0 != ( EF_FS_FAT12 & pxFS->u8FsType ) )
                && ( EF_RET_OK != eEFPrvVolumeMountFinishFAT12( pxFS ) ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILESYSTEM );
      }
      else if (    ( 0 != ( EF_FS_FAT16 & pxFS->u8FsType ) )
                && ( EF_RET_OK != eEFPrvVolumeMountFinishFAT16( pxFS ) ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILESYSTEM );
      }
      else if (    ( 0 != ( EF_FS_FAT32 & pxFS->u8FsType ) )
                && ( EF_RET_OK != eEFPrvVolumeMountFinishFAT32( pxFS ) ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILESYSTEM );
      }
      else
      {
        /* Everything went fine */
        eRetVal = EF_RET_OK;

        if ( 0 != EF_CONF_VFAT )
        {
//      #if ( 1 == EF_CONF_VFAT )
//          pxFS->pxLFNBuffer = LfnBuf;  /* Static LFN working buffer */
//      #endif
        }
        if ( 0 != EF_CONF_RELATIVE_PATH )
        {
          /* Initialize current directory */
          pxFS->u32DirClstCurrent = 0;
        }
        /* Clear file lock semaphores */
        (void) eEFPrvLockClear( pxFS );
      }

    }
    else
    {
      /* No FAT volume is found */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILESYSTEM );
    }
  }
  if ( EF_RET_OK == eRetVal )
  {
    /* Volume mount ID */
    pxFS->u16MountId = ++Fsid;
    if ( 0 != EF_CONF_VFAT )
    {
//  #if ( EF_DEF_VFAT_BUFFER_STATIC == EF_CONF_VFAT_BUFFER ) && ( 0 != EF_CONF_VFAT )
//      /* Static LFN working buffer */
//      pxFS->pxLFNBuffer = LfnBuf;
//  #endif
    }
    if ( 0 != EF_CONF_RELATIVE_PATH )
    {
      /* Initialize current directory */
      pxFS->u32DirClstCurrent = 0;
    }
    /* Clear file lock semaphores */
    (void) eEFPrvLockClear( pxFS );
  }

  return eRetVal;
}

ef_return_et eEFPrvVolumeFSPtrGet (
  int8_t      s8VolumeNb,
  ef_fs_st ** ppxFS
)
{
  EF_ASSERT_PRIVATE( 0 <=  s8VolumeNb );
  EF_ASSERT_PRIVATE( EF_CONF_VOLUMES_NB > s8VolumeNb );
  EF_ASSERT_PRIVATE( 0 != ppxFS );

  ef_return_et  eRetVal = EF_RET_INVALID_DRIVE;

  *ppxFS  = pxeFAT[ s8VolumeNb ];

  /* The filesystem object is already valid */
  eRetVal = EF_RET_OK;

  return eRetVal;
}

ef_return_et eEFPrvVolumeFSPtrSet (
  int8_t      s8VolumeNb,
  ef_fs_st  * pxFS
)
{
  EF_ASSERT_PRIVATE( 0 <=  s8VolumeNb );
  EF_ASSERT_PRIVATE( EF_CONF_VOLUMES_NB > s8VolumeNb );
//  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_INVALID_DRIVE;

  pxeFAT[ s8VolumeNb ] = pxFS;

  /* The filesystem object is already valid */
  eRetVal = EF_RET_OK;

  return eRetVal;
}

ef_return_et eEFPrvVolumeMountCheck (
  const TCHAR **  ppxPath,
  ef_fs_st    **  ppxFS
)
{
  EF_ASSERT_PRIVATE( 0 != ppxPath );
  EF_ASSERT_PRIVATE( 0 != *ppxPath );
  EF_ASSERT_PRIVATE( 0 != ppxFS );

  *ppxFS        = 0;

  ef_return_et  eRetVal = EF_RET_INVALID_DRIVE;
  int8_t        s8VolumeNb = 0;
  ef_fs_st *    pxFS = 0;

  /* Get logical drive number */
  if ( EF_RET_OK != eEFPrvVolumeNbGet( ppxPath, &s8VolumeNb ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_DRIVE );
  }
  /* Else, if getting the pointer to the filesystem object */
  else if ( EF_RET_OK != eEFPrvVolumeFSPtrGet( s8VolumeNb, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_PARAMETER );
  }
  /* Else, if the filesystem object is not valid anymore */
  else if ( 0 == pxFS )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NOT_ENABLED );
  }
  /* Else, if we cannot lock the volume */
  else if ( EF_RET_OK != eEFPrvFSLock( pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_TIMEOUT );
  }
  else
  {
    /* Return pointer to the filesystem object */
    *ppxFS = pxFS;

    /* Desired access u8Mode, write access or not */
//        u8Mode &= (ef_u08_t)~EF_FA_READ;
    /* If the volume has been mounted */
    if ( 0 != ( EF_FS_FATS & pxFS->u8FsType ) )
    {
      ef_u08_t xStatus =  eEFPrvDriveStatus( pxFS->u8PhysDrv );
      /* and the physical drive is kept initialized */
      if ( 0 == ( xStatus & EF_RET_DISK_NOINIT ) )
      {
        /* The filesystem object is already valid */
        eRetVal = EF_RET_OK;
      }
      else
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_DRIVE );
      }
    }
  }

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
