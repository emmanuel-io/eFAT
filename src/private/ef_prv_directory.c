/**
 * ********************************************************************************************************************
 *  @file     ef_prv_directory.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Code file for directory handling.
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
#include <efat.h>
#include <ef_prv_def.h>
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
#include <ef_port_load_store.h>
#include <ef_port_memory.h>

/* Local constant macros ------------------------------------------------------------------------------------------- */
#define MAX_DIR 0x00200000  /**< Max size of FAT directory */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/* Load start cluster number */
ef_return_et eEFPrvDirectoryClusterGet(
  ef_fs_st        * pxFS,
  const ef_u08_t  * pu8Dir,
  ef_u32_t        * pu32Cluster
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );
  EF_ASSERT_PRIVATE( 0 != pu8Dir );
  EF_ASSERT_PRIVATE( 0 != pu32Cluster );

  ef_u32_t u32Cluster = u16EFPortLoad( pu8Dir + EF_DIR_FIRST_CLUSTER_LOW );

  if ( 0 != ( EF_FS_FAT32 & pxFS->u8FsType ) )
  {
    u32Cluster |= (ef_u32_t) u16EFPortLoad( pu8Dir + EF_DIR_FIRST_CLUSTER_HI ) << 16;
  }

  *pu32Cluster = u32Cluster;

  return EF_RET_OK;
}

/* Store start cluster number */
ef_return_et eEFPrvDirectoryClusterSet
(
  ef_fs_st  * pxFS,
  ef_u08_t  * pu8Dir,
  ef_u32_t    u32Cluster
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );
  EF_ASSERT_PRIVATE( 0 != pu8Dir );

  vEFPortStoreu16( pu8Dir + EF_DIR_FIRST_CLUSTER_LOW, (ef_u16_t) u32Cluster );
  if ( 0 != ( EF_FS_FAT32 & pxFS->u8FsType ) )
  {
    vEFPortStoreu16( pu8Dir + EF_DIR_FIRST_CLUSTER_HI, (ef_u16_t) ( u32Cluster >> 16 ) );
  }
  else
  {
    EF_CODE_COVERAGE( );
  }
  return EF_RET_OK;
}

/* Directory handling - Fill a cluster with zeros */
ef_return_et eEFPrvDirectoryClusterClear (
  ef_fs_st  * pxFS,
  ef_u32_t    u32Cluster
)
{
  /* If Sanity check on path and directory object pointers failed */
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_OK;
  /* Top of the cluster */
  ef_lba_t xSector;

  /* If Flushing disk access window failed */
  if ( EF_RET_OK != eEFPrvFSWindowStore( pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  /* Else, if converting Cluster Number to Sector Number */
  else if ( EF_RET_OK != eEFPrvFATClusterToSector( pxFS, u32Cluster, &xSector ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  /* Else, if clearing window buffer failed */
  else if ( EF_RET_OK != eEFPortMemZero( pxFS->pu8Window, pxFS->u32WinSize ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  else
  {
    /* Set window to top of the cluster */
    pxFS->xWindowSector = xSector;
    ef_u32_t n;
    /* Fill the cluster with 0 */
    for ( n = pxFS->u8ClstSize ; 0 != n ; n-- )
    {
      if ( EF_RET_OK !=  eEFPrvDriveWrite(  pxFS->u8PhysDrv,
                                                  pxFS->pu8Window,
                                                  xSector + ( n - 1 ),
                                                  1 ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
        break;
      }
    }
    if ( 0 != n )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
    }
    else
    {
      eRetVal = EF_RET_OK;
    }
  }

  return eRetVal;
}

/* Directory handling - Set directory index */
ef_return_et eEFPrvDirectoryIndexSet (
  ef_directory_st * pxDir,
  ef_u32_t          u32Offset
)
{
  /* If Sanity check on directory object pointers failed */
  EF_ASSERT_PRIVATE( 0 != pxDir );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_u32_t      u32Cluster;
  ef_fs_st    * pxFS = pxDir->xObject.pxFS;

  /* If offset out of range */
  if ( (ef_u32_t)MAX_DIR <= u32Offset )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_FAT_ERROR );
  }
  /* Else, if offset is not aligned */
  else if ( 0 != u32Offset % EF_DIR_ENTRY_SIZE )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_FAT_ERROR );
  }
  else
  {
    /* Set current offset */
    pxDir->u32Offset = u32Offset;
    /* Table start cluster (0:root) */
    u32Cluster = pxDir->xObject.u32ClstStart;
    if (    ( 0 == u32Cluster )
         && ( 0 != ( EF_FS_FAT32 & pxFS->u8FsType ) ) )
    {
      /* Replace cluster# 0 with root cluster# */
      u32Cluster = (ef_u32_t)pxFS->xDirBase;
    }

    /* IF Cluster points to root directory */
    if ( 0 == u32Cluster )
    { /* Static table (root-directory on the FAT volume) */
      /* If index is out of range */
      if ( ( u32Offset / EF_DIR_ENTRY_SIZE ) >= pxFS->u16RootDirNb )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_FAT_ERROR );
      }
      else
      {
        pxDir->xSector = pxFS->xDirBase;
      }
    } /* Static table (root-directory on the FAT volume) */
    else
    { /* Dynamic table (sub-directory or root-directory on the FAT32 volume) */
      /* Bytes per cluster */
      ef_u32_t  u32ClusterSize = (ef_u32_t)pxFS->u8ClstSize * EF_SECTOR_SIZE(pxFS);
      /* Follow cluster chain */
      while ( u32Offset >= u32ClusterSize )
      {
        /* If get next cluster failed */
        if ( EF_RET_OK != eEFPrvFATGet( pxFS, u32Cluster, &u32Cluster ) )
        {
          /* Reached to end */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_FAT_ERROR );
          break;
        }
        /* Else, if fetched cluster error */
        else if ( EF_RET_OK != eEFPrvFATClusterNbCheck( pxFS->u32FatEntriesNb, u32Cluster ) )
        {
          /* Reached to end */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_FAT_ERROR );
          break;
        }
        else
        {
          u32Offset -= u32ClusterSize;
        }
      } /* while */
      if ( EF_RET_OK != eRetVal )
      {
        EF_CODE_COVERAGE( );
      }
      else
//        if ( u32Offset >= u32ClusterSize ) )
      {
        (void) eEFPrvFATClusterToSector( pxFS, u32Cluster, &(pxDir->xSector) );
      }
    } /* Dynamic table (sub-directory or root-directory on the FAT32 volume) */

    if ( EF_RET_OK != eRetVal )
    {
      EF_CODE_COVERAGE( );
    }
    else
    {
      /* Current cluster# */
      pxDir->u32Clst = u32Cluster;
      if ( 0 == pxDir->xSector )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_FAT_ERROR );
      }
      else
      {
        /* Sector# of the directory entry */
        pxDir->xSector += u32Offset / EF_SECTOR_SIZE(pxFS);
        /* Pointer to the entry in the u8Window[] */
        pxDir->pu8Dir   = pxFS->pu8Window + ( u32Offset % EF_SECTOR_SIZE(pxFS) );
      }
    }
  }
  return eRetVal;
}

/* Directory handling - Move directory table index next */
ef_return_et eEFPrvDirectoryIndexNext (
  ef_directory_st * pxDir,
  ef_bool_t         bStretch
)
{
  EF_ASSERT_PRIVATE( 0 != pxDir );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_u32_t      u32Offset;
  ef_u32_t      u32Cluster;
  ef_fs_st    * pxFS = pxDir->xObject.pxFS;


  /* Next entry */
  u32Offset = pxDir->u32Offset + EF_DIR_ENTRY_SIZE;

  /* If the offset reached the max value */
  if (    ( 0 != ( EF_FS_FATS & pxFS->u8FsType ) )
       && ( ( (ef_u32_t) MAX_DIR )    <= u32Offset ) )
  {
      /* Disable it */
      pxDir->xSector = 0;
      /* Report EOT */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILE );
  }
  /* Else, if the sector did not change */
  else if ( 0 != ( u32Offset % EF_SECTOR_SIZE( pxFS ) ) )
  {
    EF_CODE_COVERAGE( );
  }
  /* Else, we are at then at sector boundary */
  else
  {
    /* Next sector */
    pxDir->xSector++;

    /* If static table */
    if ( 0 == pxDir->u32Clst )
    { /* Static table */

      /* If it reached end of static table */
      if ( ( u32Offset / EF_DIR_ENTRY_SIZE ) >= pxFS->u16RootDirNb )
      {
        /* Report EOT */
        pxDir->xSector = 0;
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILE );
      }
      else
      {
        EF_CODE_COVERAGE( );
      }

    } /* Static table */

    /* Else, if dynamic table & Cluster change (ie current sector is at cluster boundary) */
    else if ( 0 == (   ( u32Offset / EF_SECTOR_SIZE(pxFS) )
                & ( (ef_u32_t) pxFS->u8ClstSize - 1 ) ) )
    { /* Dynamic table & cluster change */
      /* Get next cluster */
      if ( EF_RET_OK != eEFPrvFATGet( pxFS, pxDir->u32Clst, &u32Cluster ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_FAT_ERROR );
      }
      /* Else, if cluster number is wrong */
      else if ( 2 > u32Cluster )
      {
        /* Disk error */
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
      }
      /* Else, if it didn't reach the end of dynamic table */
      else if ( u32Cluster < pxFS->u32FatEntriesNb)
      {
        EF_CODE_COVERAGE( );
      }
      /* Else, if stretching is not requested */
      if ( EF_BOOL_TRUE != bStretch )
      {
        /* Report EOT */
        pxDir->xSector = 0;
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILE );
      }
      /* Else, if stretching the FAT Chain failed / allocating a new cluster */
      else if ( EF_RET_OK != eEFPrvFATChainStretch( &pxDir->xObject, pxDir->u32Clst, &u32Cluster ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DENIED );
      }
      /* Else, if clearing the cluster failed */
      else if ( EF_RET_OK != eEFPrvDirectoryClusterClear( pxFS, u32Cluster ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DENIED );
      }
      else
      {
        /* No problems */
        EF_CODE_COVERAGE( );
      }

      if ( EF_RET_OK != eRetVal )
      {
        EF_CODE_COVERAGE( );
      }
      else
      {
        /* Initialize data for new cluster */
        pxDir->u32Clst = u32Cluster;
        (void) eEFPrvFATClusterToSector( pxFS, u32Cluster, &(pxDir->xSector) );
      }
    } /* Dynamic table & cluster change */

    else
    { /* Dynamic table & NO cluster change */
      EF_CODE_COVERAGE( );
    } /* Dynamic table & NO cluster change */

  }

  if ( EF_RET_OK != eRetVal )
  {
    EF_CODE_COVERAGE( );
  }
  else
  {
    /* Current entry */
    pxDir->u32Offset = u32Offset;
    /* Pointer to the entry in the u8Window[] */
    pxDir->pu8Dir = pxFS->pu8Window + u32Offset % EF_SECTOR_SIZE(pxFS);

  }

  return eRetVal;
}

/* Directory handling - Reserve a block of directory entries */
ef_return_et eEFPrvDirectoryAllocate (
  ef_directory_st * pxDir,
  ef_u32_t          u32EntriesNb
)
{
  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st    * pxFS = pxDir->xObject.pxFS;

//  if ( EF_RET_OK != eEFPrvDirectoryIndexSet( pxDir, 0 ) )
  if ( EF_RET_OK != eEFPrvDirectoryIndexSet( pxDir, 0 ) )
  {
    /* Reached to end */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_FAT_ERROR );
  }
  else
  {
    ef_u32_t n = 0;
    do
    {

      if ( EF_RET_OK != eEFPrvFSWindowLoad( pxFS, pxDir->xSector ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
        break;
      }
      if (    ( EF_DIR_DELETED_MASK != pxDir->pu8Dir[ EF_DIR_NAME_START ] )
           && ( 0 != pxDir->pu8Dir[ EF_DIR_NAME_START ] ) )
      {
        n = 0; /* Not a blank entry. Restart to search */
      }
      else if ( u32EntriesNb == ++n )
      {
        break;  /* A block of contiguous free entries is found */
      }
      else
      {
        /* Keep searching */
        EF_CODE_COVERAGE( );
      }

      if ( EF_RET_OK != eEFPrvDirectoryIndexNext( pxDir, EF_BOOL_TRUE ) )
      {
        /* Reached to end */
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_FAT_ERROR );
      }
      else
      {
        EF_CODE_COVERAGE( );
      }

    /* Next entry with table stretch enabled */
    } while ( EF_RET_OK == eRetVal );
  }

  if ( EF_RET_NO_FILE == eRetVal )
  {
    /* No directory entry to allocate */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DENIED );
  }
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
