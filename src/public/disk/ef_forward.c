/**
 * ********************************************************************************************************************
 *  @file     ef_forward.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Forward Data to the Stream Directly
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
#include <efat_level3.h>
#include <ef_prv_def.h>
#include <ef_prv_fat.h>
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

ef_return_et eEF_forward (
  EF_FILE   * pxFile,
  StreamFn  * pxFunc,
  ef_u32_t    u32BFw,
  ef_u32_t  * pu32BFw
)
{
  EF_ASSERT_PUBLIC( 0 != pxFile );
  EF_ASSERT_PUBLIC( 0 != pxFunc );
  EF_ASSERT_PUBLIC( 0 != pu32BFw );

  ef_return_et  eRetVal;
  ef_fs_st    * pxFS;
  ef_u32_t     u32Clst;
  ef_lba_t         xSector;
  ef_u32_t     xRemain;
  ef_u32_t     rcnt;
  ef_u32_t     csect;
  ef_u08_t   * pu8dbuf;

  /* Clear transfer byte counter */
  *pu32BFw = 0;
  /* Check validity of the file object */
  eRetVal = eEFPrvValidateObject( &pxFile->xObject, &pxFS );
  if (    ( EF_RET_OK != eRetVal )
       || (eRetVal = (ef_return_et)pxFile->u8ErrorCode) != EF_RET_OK )
  {
    (void) eEFPrvFSUnlock( pxFS, eRetVal );
    return eRetVal;
  }

  xRemain = pxFile->u32Size - pxFile->u32FileOffset;
  if ( u32BFw > xRemain )
  {
    /* Truncate btf by remaining bytes */
    u32BFw = (ef_u32_t)xRemain;
  }

  /* Repeat until all data transferred or stream goes busy */
  for ( ;  u32BFw && (*pxFunc)(0, 0);
    pxFile->u32FileOffset += rcnt, *pu32BFw += rcnt, u32BFw -= rcnt)
  {
    /* Sector offset in the cluster */
    csect = (ef_u32_t)( ( pxFile->u32FileOffset / EF_SECTOR_SIZE( pxFS ) ) & ( pxFS->u8ClstSize - 1 ) );
    /* On the sector boundary? */
    if (pxFile->u32FileOffset % EF_SECTOR_SIZE( pxFS ) == 0)
    {
      /* On the cluster boundary? */
      if ( 0 == csect )
      {
        /* On the top of the file? */
        if ( 0 == pxFile->u32FileOffset )
        {
          u32Clst = pxFile->xObject.u32ClstStart;
        }
        else
        {
          eRetVal = eEFPrvFATGet( pxFS, pxFile->u32Clst, &u32Clst );
        }
        if ( EF_RET_OK != eRetVal )
        {
          pxFile->u8ErrorCode = (ef_u08_t)(eRetVal);
          (void) eEFPrvFSUnlock( pxFS, eRetVal );
          return eRetVal;
        }
        /* Update current cluster */
        pxFile->u32Clst = u32Clst;
      }
    }
    /* Get current data sector */
    if ( EF_RET_OK != eEFPrvFATClusterToSector( pxFS, pxFile->u32Clst, &xSector ) )
    {
      eRetVal = EF_RET_INT_ERR;
      pxFile->u8ErrorCode = (ef_u08_t)(eRetVal);
      (void) eEFPrvFSUnlock( pxFS, eRetVal );
      return eRetVal;
    }
    xSector += csect;
    if (pxFile->xSector != xSector)
    {
      /* Fill sector cache with file data */
        if ( 0 != ( EF_FILE_WIN_DIRTY & pxFile->u8StatusFlags ) )
        {    /* Write-back dirty sector cache */
          if ( EF_RET_OK !=  eEFPrvDriveWrite( pxFS->u8PhysDrv, pxFile->u8Window, pxFile->xSector, 1 ) )
          {
            eRetVal = EF_RET_DISK_ERR;
            pxFile->u8ErrorCode = (ef_u08_t)(eRetVal);
            (void) eEFPrvFSUnlock( pxFS, eRetVal );
            return eRetVal;
          }
          pxFile->u8StatusFlags &= (ef_u08_t)~EF_FILE_WIN_DIRTY;
        }
      if ( EF_RET_OK !=  eEFPrvDriveRead( pxFS->u8PhysDrv, pxFile->u8Window, xSector, 1 ) )
      {
        eRetVal = EF_RET_DISK_ERR;
        pxFile->u8ErrorCode = (ef_u08_t)(eRetVal);
        (void) eEFPrvFSUnlock( pxFS, eRetVal );
        return eRetVal;
      }
    }
    pu8dbuf = pxFile->u8Window;
    pxFile->xSector = xSector;
    /* Number of bytes remains in the sector */
    rcnt = EF_SECTOR_SIZE( pxFS ) - (ef_u32_t)pxFile->u32FileOffset % EF_SECTOR_SIZE( pxFS );
    if ( rcnt > u32BFw )
    {
      /* Clip it by btr if needed */
      rcnt = u32BFw;
    }
    /* Forward the file data */
    rcnt = (*pxFunc)( pu8dbuf + ( (ef_u32_t)pxFile->u32FileOffset % EF_SECTOR_SIZE( pxFS ) ),
                      rcnt );
    if ( 0 == rcnt )
    {
      eRetVal = EF_RET_INT_ERR;
      pxFile->u8ErrorCode = (ef_u08_t)(eRetVal);
      (void) eEFPrvFSUnlock( pxFS, eRetVal );
      return eRetVal;
    }
  }

  eRetVal = EF_RET_OK;
  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

