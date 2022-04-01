/**
 * ********************************************************************************************************************
 *  @file     ef_fread.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Read File
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
#include <ef_port_memory.h>
#include <efat.h>
#include <ef_prv_def.h>
#include <ef_prv_fat.h>
#include <ef_prv_file.h>
#include "ef_port_diskio.h"
#include "ef_prv_directory.h"
#include "ef_prv_drive.h"
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

#include <stdio.h>

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */

/**
 *  @brief  Update the file structure cluster number for next read access (on cluster crossing)
 *
 *  @param  pxFile  Pointer to the file object
 *
 *  @return Operation result
 *  @retval EF_RET_OK     Success
 *  @retval EF_RET_ERROR  An error occurred
 *  @retval EF_RET_ASSERT Assertion failed
 */
ef_return_et eEFPrvFileReadClusterNbUpdate (
  ef_file_st  * pxFile
);

/* Local functions ------------------------------------------------------------------------------------------------- */

ef_return_et eEFPrvFileReadClusterNbUpdate (
//static ef_return_et eEFPrvFileReadClusterNbUpdate (
  ef_file_st  * pxFile
)
{
  EF_ASSERT_PRIVATE( 0 != pxFile );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_u32_t     u32ClusterNb;

  /* If on the top of the file? */
  if ( 0 == pxFile->u32FileOffset )
  {
    /* Follow cluster chain from the origin */
    pxFile->u32Clst = pxFile->xObject.u32ClstStart;
  }
  /* Else, if Following cluster chain on the FAT failed (Middle or end of the file) */
  else if ( EF_RET_OK != eEFPrvFATGet( pxFile->xObject.pxFS, pxFile->u32Clst, &u32ClusterNb ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    /* Update current cluster */
    pxFile->u32Clst = 0;
  }
  else
  {
    /* Update current cluster */
    pxFile->u32Clst = u32ClusterNb;
  }

  return eRetVal;
}

/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_fread (
  EF_FILE   * pxFile,
  void      * pvDataPtr,
  ef_u32_t    u32BytesToRead,
  ef_u32_t  * pu32BytesRead
)
{
  EF_ASSERT_PUBLIC( 0 != pxFile );
  EF_ASSERT_PUBLIC( 0 != pvDataPtr );
  EF_ASSERT_PUBLIC( 0 != pu32BytesRead );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st    * pxFS;

  /* Clear read byte counter */
  *pu32BytesRead = 0;
  /* Check validity of the file object */
  /* If File object is not valid */
  if ( EF_RET_OK != eEFPrvValidateObject( &pxFile->xObject, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_OBJECT );
  }
  /* Else, if Nothing to read */
  else if ( 0 == u32BytesToRead )
  {
    /* Nothing to do, success */
    EF_CODE_COVERAGE( );
  }
  else if ( 0 == ( pxFile->u32Size - pxFile->u32FileOffset ) )
  {
    /* Nothing to do, success */
    EF_CODE_COVERAGE( );
  }
  else
  {

    /* On the top of the file? */
    if ( 0 == pxFile->u32FileOffset )
    {
      /* Follow from the origin */
      /* Update current cluster */
      pxFile->u32Clst = pxFile->xObject.u32ClstStart;
    }
    else
    {
      EF_CODE_COVERAGE( );
    }

    ef_u08_t * pu8DataBuffer = (ef_u08_t*) pvDataPtr;

    /* Number of bytes readable */
    ef_u32_t   u32BytesRemaining   = pxFile->u32Size - pxFile->u32FileOffset;

    /* Truncate xBytesRemaining by the maximum value of ef_u32_t type */
    if ( u32BytesRemaining > ( (ef_u32_t) ( (ef_u32_t) -1 ) ) )
    {
      u32BytesRemaining = ( (ef_u32_t) ( (ef_u32_t) -1 ) );
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
    /* Truncate u32BytesToRead by remaining bytes */
    if ( u32BytesToRead > (ef_u32_t) u32BytesRemaining )
    {
      u32BytesToRead = (ef_u32_t) u32BytesRemaining;
    }
    else
    {
      EF_CODE_COVERAGE( );
    }

    /* Assume everything will go well */
//    *pu32BytesRead = u32BytesToRead;

    ef_lba_t xSector = pxFile->xSector;

    /* Unless something goes wrong it will be a success */
//    eRetVal = EF_RET_OK;

    /* Repeat until u32BytesToRead gets down to zero (or we breaked out of the loop) */
    while ( 0 != u32BytesToRead )
    { /* Loop */

      /* Number of bytes transferred */
      ef_u32_t  u32BytesTransfered = 0;

      /* Offset in the sector */
      ef_u32_t  u32OffsetInSector = (ef_u32_t)( pxFile->u32FileOffset ) % EF_SECTOR_SIZE( pxFS );

      /* If Not on the sector boundary */
      if ( 0 != u32OffsetInSector )
      { /* TRANSFER NOT ON THE SECTOR BOUNDARY BEGIN */

        /* Number of bytes remaining in the sector */
        ef_u32_t u32BytesRemaining =  EF_SECTOR_SIZE( pxFS ) - u32OffsetInSector;
        if ( u32BytesRemaining > u32BytesToRead )
        {
          /* Clip it by u32BytesToRead if needed */
          u32BytesRemaining = u32BytesToRead;
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
        /* If extracting the remaining bytes from the sector failed */
        if ( EF_RET_OK != eEFPortMemCopy( pxFile->u8Window + u32OffsetInSector, pu8DataBuffer, u32BytesRemaining ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
          break;
        }
        else
        {
          /* Number of bytes transferred */
          u32BytesTransfered = u32BytesRemaining;
        }
      } /* TRANSFER NOT ON THE SECTOR BOUNDARY END */
      /* Else we are on the sector boundary */
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
        else if ( EF_RET_OK != eEFPrvFileReadClusterNbUpdate( pxFile ) )
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
        ef_u32_t  u32SectorsNb = u32BytesToRead / EF_SECTOR_SIZE( pxFS );

        /* If there is full sectors to read */
        if ( 0 != u32SectorsNb )
        { /* TRANSFER WHOLE SECTORS ONLY BEGIN */

          /* Add the offset in the cluster to the Sector number to get the real value */
          xSector += u32ClusterOffset;

          /* If the sectors remaining to read in the cluster is larger than the cluster size */
          if ( ( u32SectorsNb + u32ClusterOffset ) > pxFS->u8ClstSize )
          {
            /* Clip at cluster boundary, Limit the number of sectors to what remains in the cluster */
            u32SectorsNb = pxFS->u8ClstSize - u32ClusterOffset;
          }
          else
          {
            EF_CODE_COVERAGE( );
          }

          /* Reading whole sectors */
          /* If reading the maximum contiguous sectors directly failed */
          if ( EF_RET_OK != eEFPrvDriveRead( pxFS->u8PhysDrv, pu8DataBuffer, xSector, u32SectorsNb ) )
          {
            eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
            break;
          }
          else
          {
            /* Number of bytes transferred */
            u32BytesTransfered = EF_SECTOR_SIZE( pxFS ) * u32SectorsNb;
            /* Add the sector offset in the cluster to the sector number */
            xSector += u32ClusterOffset;
          }

        } /* TRANSFER WHOLE SECTORS ONLY END */
        /* Else, there is a partial sector to read which start at sector boundary */
        else
        {
          /* Get out of the loop, this happens out of the loop */
          break;
        }

      } /* TRANSFER ON THE SECTOR BOUNDARY END */

      /* Update counters and pointers */
      u32BytesToRead        -= u32BytesTransfered; /* Bytes remaining to read */
      pu8DataBuffer         += u32BytesTransfered; /* Read data buffer pointer */
      pxFile->u32FileOffset += u32BytesTransfered; /* File offset */
      /* Update bytes effectively read */
      *pu32BytesRead        += u32BytesTransfered; /* Bytes effectively read */

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
    /* Else, if there are no more bytes to read */
    else if ( 0 == u32BytesToRead )
    {
      /* We are done */
      EF_CODE_COVERAGE( );
    }
    /* Else, if filling the remaining bytes into the window */
    else if ( EF_RET_OK != eEFPortMemCopy( pxFile->u8Window, pu8DataBuffer, u32BytesToRead ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
    }
    else
    {
      /* TRANSFERED PARTIAL SECTOR FROM BOUNDARY SUCCESS */
      /* Update File offset */
      pxFile->u32FileOffset += u32BytesToRead;
      /* Update bytes effectively read */
      *pu32BytesRead += u32BytesToRead; /* Bytes effectively read */
      /* No more bytes to read */
      u32BytesToRead = 0;
    }

    /* Now the sector in the window is where the FileOffset belong */
    pxFile->xSector = xSector;

  }

  /* Unlock filesystem if eRetVal allows */
  (void) eEFPrvFSUnlock( pxFS, eRetVal );

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

