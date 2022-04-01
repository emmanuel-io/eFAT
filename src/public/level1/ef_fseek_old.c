/**
 * ********************************************************************************************************************
 *  @file     ef_fseek.c
 *  @ingroup  group_eFAT_Public
 *  @author   Emmanuel AMADIO
 *  @version  V0.1

 *  @brief    Seek File Read/Write Pointer
 *
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
#include "ef_prv_load_store.h"
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
//  else if ( 0 == u32Offset )
//  {
//    EF_CODE_COVERAGE( );
//  }
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

    /* If file offset is more than 0 */
    if ( u32Offset > 0 )
    {
      /* Cluster size (byte) */
      ef_u32_t u32ClusterByteSize = (ef_u32_t) pxFS->u8ClstSize * EF_SECTOR_SIZE(pxFS);

      /* When seek to same or following cluster, */
      if (    ( u32FileOffset > 0 )
           && ( ( ( u32Offset - 1 ) / u32ClusterByteSize ) >= ( ( u32FileOffset - 1 ) / u32ClusterByteSize) ) )
      {
        /* start from the current cluster */
        pxFile->u32FileOffset = ( u32FileOffset - 1 ) & ~(ef_u32_t) (u32ClusterByteSize - 1);
        u32Offset -= pxFile->u32FileOffset;
        u32ClusterNb = pxFile->u32Clst;
      }
      else
      { /* When seek to back cluster, */
        /* start from the first cluster */
        u32ClusterNb = pxFile->xObject.u32ClstStart;
        /* If      no cluster chain
         *    AND  creating a new chain failed */
        if (    ( 0 == u32ClusterNb )
             && ( EF_RET_OK != eEFPrvFATChainCreate(&pxFile->xObject, &u32ClusterNb) ) )
        {
          eRetVal = EF_RET_ERROR;
          (void) eEFPrvFSUnlock(pxFS, eRetVal);
          return eRetVal;
        }
        else
        {
          eRetVal = EF_RET_OK;
          pxFile->xObject.u32ClstStart = u32ClusterNb;
        }

        pxFile->u32Clst = u32ClusterNb;
      }

      if ( 0 != u32ClusterNb )
      {
        /* Cluster following loop */
        while ( u32Offset > u32ClusterByteSize )
        {
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
            /* Follow chain with forced stretch */
            eRetVal = eEFPrvFATChainStretch( &pxFile->xObject, u32ClusterNb, &u32ClusterNb );
            /* Clip file size in case of disk full */
            if ( EF_RET_OK != eRetVal )
            {
              u32Offset = 0;
              break;
            }
            if ( EF_RET_FAT_FULL == eRetVal )
            {
              u32Offset = 0;
              break;
            }
          }
          else
          {
            /* Follow cluster chain if not in write mode */
            eRetVal = eEFPrvFATGet( &pxFile->xObject, u32ClusterNb, &u32ClusterNb );
          }
          if ( EF_RET_OK != eRetVal )
          {
            pxFile->u8ErrorCode = (ef_u08_t) (eRetVal);
            (void) eEFPrvFSUnlock(pxFS, eRetVal);
            return eRetVal;
          }
          if ( u32ClusterNb >= pxFS->u32FatEntriesNb )
          {
            eRetVal = EF_RET_INT_ERR;
            pxFile->u8ErrorCode = (ef_u08_t) (eRetVal);
            (void) eEFPrvFSUnlock(pxFS, eRetVal);
            return eRetVal;
          }
          pxFile->u32Clst = u32ClusterNb;
        }
        pxFile->u32FileOffset += u32Offset;
        if ( 0 != ( u32Offset % EF_SECTOR_SIZE(pxFS) ) )
        {
          /* Current sector */
          eRetVal = eEFPrvFATClusterToSector(pxFS, pxFile->u32Clst, &xSectorNb);
          if ( EF_RET_OK != eRetVal )
          {
            pxFile->u8ErrorCode = (ef_u08_t) (eRetVal);
            (void) eEFPrvFSUnlock(pxFS, eRetVal);
            return eRetVal;
          }
          xSectorNb += (ef_lba_t) ( u32Offset / EF_SECTOR_SIZE(pxFS) );
        }
      }
    }

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

  eRetVal = eEFPrvFSUnlock(pxFS, eRetVal);
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
