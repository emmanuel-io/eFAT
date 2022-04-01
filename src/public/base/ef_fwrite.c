/**
 * ********************************************************************************************************************
 *  @file     ef_fwrite.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Write File
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
#include <ef_port_load_store.h>
#include <ef_port_memory.h>

#include "stdio.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */

/**
 *  @brief  Update the file structure cluster number for next write access (on cluster crossing)
 *
 *  @param  pxFile  Pointer to the file object
 *
 *  @return Operation result
 *  @retval EF_RET_OK     Success
 *  @retval EF_RET_ERROR  An error occurred
 *  @retval EF_RET_ASSERT Assertion failed
 */
ef_return_et eEFPrvFileWriteClusterNbUpdate (
  ef_file_st  * pxFile
);

/* Local functions ------------------------------------------------------------------------------------------------- */

//static ef_return_et eEFPrvFileWriteClusterNbUpdate (
ef_return_et eEFPrvFileWriteClusterNbUpdate (
  ef_file_st  * pxFile
)
{
  EF_ASSERT_PUBLIC( 0 != pxFile );

  ef_return_et  eRetVal = EF_RET_OK;

  ef_u32_t u32ClusterNb;
  /* On the top of the file? */
  if ( 0 == pxFile->u32FileOffset )
  {
    /* Follow from the origin */
    u32ClusterNb = pxFile->xObject.u32ClstStart;
    /* If a cluster is allocated, */
    if ( 0 != u32ClusterNb )
    {
      EF_CODE_COVERAGE( );
    }
     /* Create a new cluster chain */
    else if ( EF_RET_OK != eEFPrvFATChainCreate( &pxFile->xObject, &u32ClusterNb ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
  }
  /* Middle or end of the file */
  /* Follow or stretch cluster chain on the FAT */
  else if ( EF_RET_OK != eEFPrvFATChainStretch( &pxFile->xObject, pxFile->u32Clst, &u32ClusterNb ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    EF_CODE_COVERAGE( );
  }
  if ( EF_RET_OK == eRetVal )
  {
    /* Update current cluster */
    pxFile->u32Clst = u32ClusterNb;
    /* If the first write */
    if ( 0 == pxFile->xObject.u32ClstStart )
    {
      /* Set start cluster */
      pxFile->xObject.u32ClstStart = u32ClusterNb;
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
  }
  else
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    EF_CODE_COVERAGE( );
  }

  return eRetVal;
}

/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_fwrite (
  EF_FILE     * pxFile,
  const void  * pvDataPtr,
  ef_u32_t      u32BytesToWrite,
  ef_u32_t    * pu32BytesWritten
)
{
  EF_ASSERT_PUBLIC( 0 != pxFile );
  EF_ASSERT_PUBLIC( 0 != pvDataPtr );
  EF_ASSERT_PUBLIC( 0 != pu32BytesWritten );

  ef_return_et    eRetVal = EF_RET_OK;
  ef_fs_st      * pxFS;

  /* Clear written bytes counter */
  *pu32BytesWritten = 0;

  /* If access mode is not compatible */
  if ( 0 == ( pxFile->u8StatusFlags & EF_FILE_OPEN_WRITE ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DENIED );
  }
  /* Else, if File object is not valid */
  else if ( EF_RET_OK != eEFPrvValidateObject( &pxFile->xObject, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_OBJECT );
  }
  /* Else, if Nothing to write */
  else if ( 0 == u32BytesToWrite )
  {
    /* Nothing to do, success */
    EF_CODE_COVERAGE( );
  }
  else
  {
    const ef_u08_t * pu8DataBuffer = (const ef_u08_t*) pvDataPtr;
    /* Assume everything will go well */
//    *pu32BytesWritten = u32BytesToWrite;

    /* Check u32FileOffset wrap-around (file size cannot reach 4 GiB at FAT volume) */
    if ( pxFile->u32FileOffset > ( (ef_u32_t) EF_FILE_SIZE_MAX - ((ef_u32_t) u32BytesToWrite) ) )
    {
      u32BytesToWrite = (ef_u32_t)( EF_FILE_SIZE_MAX - pxFile->u32FileOffset );
    }
    else
    {
      EF_CODE_COVERAGE( );
    }

    ef_lba_t xSector = pxFile->xSector;

    /* Unless something goes wrong it will be a success */
    eRetVal = EF_RET_OK;

    /* Repeat until u32BytesToWrite gets down to zero (or we breaked out of the loop) */
    while ( 0 != u32BytesToWrite )
    { /* Loop */

      /* Number of bytes transferred */
      ef_u32_t  u32BytesTransfered = 0;

      /* Offset in the sector */
      ef_u32_t  u32OffsetInSector = (ef_u32_t)( pxFile->u32FileOffset ) % EF_SECTOR_SIZE( pxFS );

      /* If on the sector boundary */
      if ( 0 != u32OffsetInSector )
      { /* TRANSFER NOT ON THE SECTOR BOUNDARY BEGIN */

        /* Number of bytes remaining in the sector */
        ef_u32_t  u32BytesRemaining = EF_SECTOR_SIZE( pxFS ) - u32OffsetInSector;
        if ( u32BytesRemaining >= u32BytesToWrite )
        {
          /* Clip it by u32BytesToWrite if needed */
          u32BytesRemaining = u32BytesToWrite;
        }
        else
        {
          /* More bytes to write than what remains in the sector, we'll jump to next sector on next write */
          xSector++;
//          EF_CODE_COVERAGE( );
        }
        /* If filling the remaining bytes into the window failed */
        if ( EF_RET_OK != eEFPortMemCopy(  pu8DataBuffer,
                                          pxFile->u8Window + u32OffsetInSector,
                                          u32BytesRemaining ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
          break;
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
        /* Flag the window as dirty */
        pxFile->u8StatusFlags |= EF_FILE_WIN_DIRTY;
        /* Number of bytes transferred */
        u32BytesTransfered = u32BytesRemaining;

      } /* TRANSFER NOT ON THE SECTOR BOUNDARY END */
      /* Else we are not on the sector boundary */
      else
      { /* TRANSFER ON THE SECTOR BOUNDARY BEGIN */

        /* Sector offset in the cluster */
        ef_u32_t  u32ClusterOffset = EF_CLUSTER_OFFSET_GET( pxFS );

        /* If     On the cluster boundary
         *    AND (    Updating the current cluster failed
         *          OR Getting the base sector of the cluster failed )
         */
        if ( 0 != u32ClusterOffset )
        {
          EF_CODE_COVERAGE( );
        }
        else if ( EF_RET_OK != eEFPrvFileWriteClusterNbUpdate( pxFile ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
          break;
        }
        else if ( EF_RET_OK != eEFPrvFATClusterToSector( pxFS, pxFile->u32Clst, &xSector ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
          break;
        }
        else
        {
          EF_CODE_COVERAGE( );
        }

        /* Get the number of remaining sectors */
        ef_u32_t  u32SectorsNb = u32BytesToWrite / EF_SECTOR_SIZE( pxFS );

        /* If there is full sectors to write */
        if ( 0 != u32SectorsNb )
        { /* TRANSFER WHOLE SECTORS ONLY BEGIN */

          /* If the sectors remaining to write in the cluster is larger than the cluster size */
          if ( ( u32SectorsNb + u32ClusterOffset ) > pxFS->u8ClstSize )
          {
            /* Clip at cluster boundary, Limit the number of sectors to what remains in the cluster */
            u32SectorsNb = pxFS->u8ClstSize - u32ClusterOffset;
          }
          else
          {
            EF_CODE_COVERAGE( );
          }

          /* Writing whole sectors */
          /* If writing the maximum contiguous sectors directly failed */
          if ( EF_RET_OK != eEFPrvDriveWrite( pxFS->u8PhysDrv, pu8DataBuffer, xSector, u32SectorsNb ) )
          {
            eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR);
            break;
          }
          else
          {
            /* Number of bytes transferred */
            u32BytesTransfered = EF_SECTOR_SIZE( pxFS ) * u32SectorsNb;
            /* Add the sector offset in the cluster to the sector number */
            xSector += u32SectorsNb;
          }

        } /* TRANSFER WHOLE SECTORS ONLY END */
        /* Else, there is a partial sector to write which start at sector boundary */
        else
        {
          /* Get out of the loop, this happens out of the loop */
          break;
        }
      } /* TRANSFER ON THE SECTOR BOUNDARY END */

      /* Update counters and pointers */
      u32BytesToWrite       -= u32BytesTransfered; /* Bytes remaining to write */
      pu8DataBuffer         += u32BytesTransfered; /* Write data buffer pointer */
      pxFile->u32FileOffset += u32BytesTransfered; /* File offset */
      /* Update bytes effectively written */
      *pu32BytesWritten     += u32BytesTransfered; /* Bytes effectively written */
    } /* Loop */

    /* If something failed */
    if ( EF_RET_OK != eRetVal )
    {
      /* Invalidate the window */
      xSector = 0;
    }
    /* Else, if Data sector window update failed */
    else if ( EF_RET_OK != eEFPrvFileWindowUpdate ( pxFile, pxFS, xSector ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
    }
    /* Else, if there are no more bytes to write */
    else if ( 0 == u32BytesToWrite )
    {
      /* We are done */
      EF_CODE_COVERAGE( );
    }
    /* Else, if filling the remaining bytes into the window */
    else if ( EF_RET_OK != eEFPortMemCopy( pu8DataBuffer, pxFile->u8Window, u32BytesToWrite ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
    }
    else
    {
      /* TRANSFERED PARTIAL SECTOR FROM BOUNDARY SUCCESS */
      /* Flag the window as dirty */
      pxFile->u8StatusFlags |= (ef_u08_t) EF_FILE_WIN_DIRTY;
      /* Update File offset */
      pxFile->u32FileOffset += u32BytesToWrite;
      *pu32BytesWritten     += u32BytesToWrite; /* Bytes effectively written */
      /* No more bytes to write */
      u32BytesToWrite = 0;
    }

    /* Now the sector in the window is where the FileOffset belong */
    pxFile->xSector = xSector;

    /* If something failed */
    if ( EF_RET_OK != eRetVal )
    {
      EF_CODE_COVERAGE( );
    }
    else
    {
      /* Set file change flags */
      pxFile->u8StatusFlags |= EF_FILE_MODIFIED;
      /* If file offset is bigger than file size */
      if ( pxFile->u32FileOffset > pxFile->u32Size )
      {
        /* Update File size */
        pxFile->u32Size = pxFile->u32FileOffset;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
    }
    /* Update bytes effectively written */
//    *pu32BytesWritten -= u32BytesToWrite; /* Bytes effectively written */
  }

  /* Unlock filesystem if eRetVal allows */
  (void) eEFPrvFSUnlock( pxFS, eRetVal );
//  eRetVal = eEFPrvFSUnlock( pxFS, eRetVal );
//  printf("ErrocCode %d", u8Code);

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

