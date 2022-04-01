/**
 * ********************************************************************************************************************
 *  @file     ef_fseek.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Seek File Read/Write Pointer
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
#include <ef_prv_fat.h>
#include <ef_prv_file.h>
#include "ef_prv_drive.h"
#include "ef_prv_def.h"
#include "ef_prv_directory.h"
#include "ef_prv_dirfunc.h"
#include "ef_prv_lock.h"
#include "ef_prv_string.h"
#include "ef_prv_volume.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_unicode.h"
#include "ef_prv_validate.h"
#include "ef_prv_volume_nb.h"
#include "ef_prv_volume.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_fseek (
  EF_FILE   * pxFile,
  ef_u32_t    u32Offset
)
{
  EF_ASSERT_PUBLIC( 0 != pxFile );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st    * pxFS;

  /* If File object is not valid */
  if ( EF_RET_OK != eEFPrvValidateObject( &pxFile->xObject, &pxFS ) )
  {
    eRetVal = EF_RET_ERROR;
  }
  /* Else, if file offset is  0 */
  else
  {
    /* If     In read-only mode
     *    AND Offset is more than the file size
     */
    if (   ( 0 == ( EF_FILE_OPEN_WRITE & pxFile->u8StatusFlags ) )
        && ( u32Offset > pxFile->u32Size ) )
    {
      /* Clip offset with the file size */
      u32Offset = pxFile->u32Size;
    }
    /* Else, if file size is more than the 4GB limit */
    else if ( EF_FILE_SIZE_MAX < u32Offset )
    {
      /* Clip at 4 GiB - 1 if at FATxx */
      u32Offset = EF_FILE_SIZE_MAX;
    }
    else
    {
      EF_CODE_COVERAGE( );
    }

    ef_u32_t  u32ClusterNb;
    ef_lba_t  xSectorNb = 0;
    ef_u32_t  u32FileOffset = pxFile->u32FileOffset;

    pxFile->u32FileOffset = 0;

    /* If seeked offset is not at beginning */
    if ( 0 != u32Offset )
    {
      /* Cluster size in bytes */
      ef_u32_t u32ClusterByteSize = (ef_u32_t) pxFS->u8ClstSize * EF_SECTOR_SIZE(pxFS);

      /* If     Files offset is not null
       *    AND Seeked offset stays in the same cluster as we are
       */
      if (    ( 0 != u32FileOffset )
           && ( ( ( u32Offset - 1 ) / u32ClusterByteSize ) >= ( ( u32FileOffset - 1 ) / u32ClusterByteSize) ) )
      { /* SEEKING TO SAME OR NEXT CLUSTER BEGIN */
        /* start from the current cluster */
        pxFile->u32FileOffset = ( u32FileOffset - 1 ) & ~(ef_u32_t) (u32ClusterByteSize - 1);
        u32Offset -= pxFile->u32FileOffset;
        u32ClusterNb = pxFile->u32Clst;
      } /* SEEKING TO SAME OR NEXT CLUSTER END */
      else
      { /* SEEKING TO PREVIOUS CLUSTER BEGIN */
        /* Start from the first cluster */
        u32ClusterNb = pxFile->xObject.u32ClstStart;
        /* If an existing cluster chain */
        if ( 0 != u32ClusterNb )
        {
          pxFile->u32Clst = u32ClusterNb;
        }
          /* Else, If creating a new chain failed */
        else if ( EF_RET_OK != eEFPrvFATChainCreate(&pxFile->xObject, &u32ClusterNb) )
        {
          eRetVal = EF_RET_ERROR;
          (void) eEFPrvFSUnlock(pxFS, eRetVal);
          return eRetVal;
        }
        else
        {
          pxFile->xObject.u32ClstStart = u32ClusterNb;
          pxFile->u32Clst = u32ClusterNb;
        }
      } /* SEEKING TO PREVIOUS CLUSTER END */

      if ( EF_RET_OK == eRetVal )
      {
        /* While the offset is larger than the cluster size in bytes */
        while ( u32Offset > u32ClusterByteSize )
        { /* Cluster following loop Begin */
          u32Offset -= u32ClusterByteSize;
          pxFile->u32FileOffset += u32ClusterByteSize;
          /* If in write mode */
          if ( 0 != ( EF_FILE_OPEN_WRITE & pxFile->u8StatusFlags) )
          {
            /* No FAT chain object needs correct u32Size to generate FAT value */
            if ( pxFile->u32FileOffset > pxFile->u32Size )
            {
              pxFile->u32Size = pxFile->u32FileOffset;
              pxFile->u8StatusFlags |= EF_FILE_MODIFIED;
            }
            /* If Following chain with forced stretch failed */
            if ( EF_RET_OK != eEFPrvFATChainStretch( &pxFile->xObject, u32ClusterNb, &u32ClusterNb ) )
            {
              /* Clip file size in case of disk full */
              u32Offset = 0;
              eRetVal = EF_RET_INT_ERR;
              break;
            }
            else
            {
              EF_CODE_COVERAGE( );
            }
          }
          /* Else, if Following cluster chain if not in write mode failed */
          else if ( EF_RET_OK != eEFPrvFATGet( pxFS, u32ClusterNb, &u32ClusterNb ) )
          {
            eRetVal = EF_RET_INT_ERR;
            break;
          }
          else if ( u32ClusterNb >= pxFS->u32FatEntriesNb )
          {
            eRetVal = EF_RET_INT_ERR;
            break;
          }
          else
          {
            EF_CODE_COVERAGE( );
          }
          pxFile->u32Clst = u32ClusterNb;
        } /* Cluster following loop End */

        if ( EF_RET_OK == eRetVal )
        {
          pxFile->u32FileOffset += u32Offset;
          if ( 0 != ( u32Offset % EF_SECTOR_SIZE(pxFS) ) )
          {
            /* Current sector */
            if ( EF_RET_OK != eEFPrvFATClusterToSector(pxFS, pxFile->u32Clst, &xSectorNb) )
            {
              eRetVal = EF_RET_INT_ERR;
            }
            else
            {
              xSectorNb += (ef_lba_t) ( u32Offset / EF_SECTOR_SIZE(pxFS) );
            }
          }
          else
          {
            EF_CODE_COVERAGE( );
          }
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
      }
    }

    if ( EF_RET_OK == eRetVal )
    {
      /* Set file change Flag if the file size is extended */
      if ( pxFile->u32FileOffset > pxFile->u32Size )
      {
        pxFile->u32Size = pxFile->u32FileOffset;
        pxFile->u8StatusFlags |= EF_FILE_MODIFIED;
      }

      /* If    On the sector boundary
       *    OR Sector number has changed
       */
      if (    ( 0 == ( pxFile->u32FileOffset % EF_SECTOR_SIZE(pxFS) ) )
           || ( xSectorNb == pxFile->xSector ) )
      {
        EF_CODE_COVERAGE( );
      }
      /* Else, if Write-back dirty sector cache if needed failed */
      else if ( EF_RET_OK != eEFPrvFileWindowDirtyWriteBack ( pxFile, pxFS ) )
      {
        eRetVal = EF_RET_DISK_ERR;
      }
      /* Else, if Reload sector cache failed */
      else if ( EF_RET_OK != eEFPrvDriveRead( pxFS->u8PhysDrv, pxFile->u8Window, xSectorNb, 1 ) )
      {
        /* Fill sector cache */
        eRetVal = EF_RET_DISK_ERR;
      }
      else
      {
        eRetVal = EF_RET_OK;
        pxFile->xSector = xSectorNb;
      }
    }
  }

  eRetVal = eEFPrvFSUnlock(pxFS, eRetVal);
  return eRetVal;
}
/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
