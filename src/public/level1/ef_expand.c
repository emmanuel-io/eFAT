/**
 * ********************************************************************************************************************
 *  @file     ef_expand.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Allocate a Contiguous Blocks to the File
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
#include "ef_port_diskio.h"
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
#include "ef_port_diskio.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_expand (
  EF_FILE * pxFile, /* Pointer to the file object */
  ef_u32_t  fsz,    /* File size to be expanded to */
  ef_u08_t  opt     /* Operation u8Mode 0:Find and prepare or 1:Find and allocate */
)
{
  EF_ASSERT_PUBLIC( 0 != pxFile );

  ef_return_et  eRetVal;
  ef_fs_st    * pxFS;
  ef_u32_t      n;
  ef_u32_t      clst;
  ef_u32_t      stcl;
  ef_u32_t      scl;
  ef_u32_t      ncl;
  ef_u32_t      tcl;
  ef_u32_t      lclst;


  eRetVal = eEFPrvValidateObject( &pxFile->xObject, &pxFS );    /* Check validity of the file object */
  if (( EF_RET_OK != eRetVal ) || (eRetVal = (ef_return_et)pxFile->u8ErrorCode) != EF_RET_OK)
  {
    (void) eEFPrvFSUnlock( pxFS, eRetVal );
    return eRetVal;
  }
  if (    ( 0 == fsz )
       || ( 0 != pxFile->u32Size )
       || ( 0 == ( pxFile->u8StatusFlags & EF_FILE_OPEN_WRITE ) ) )
  {
    eRetVal = EF_RET_DENIED;
    (void) eEFPrvFSUnlock( pxFS, eRetVal );
    return eRetVal;
  }
  if ( EF_FILE_SIZE_MAX < fsz )
  {
    eRetVal = EF_RET_DENIED;  /* Check if in size limit */
    (void) eEFPrvFSUnlock( pxFS, eRetVal );
    return eRetVal;
   }
  n = (ef_u32_t)pxFS->u8ClstSize * EF_SECTOR_SIZE( pxFS );  /* Cluster size */
  tcl = (ef_u32_t)(fsz / n) + ((fsz & (n - 1)) ? 1 : 0);  /* Number of clusters needed */
  stcl = pxFS->u32ClstLast; lclst = 0;
  if ( EF_RET_OK != eEFPrvFATClusterNbCheck( pxFS->u32FatEntriesNb, stcl ) )
  {
    stcl = 2;
  }

  scl   = stcl;
  clst  = stcl;
  ncl = 0;
  for ( ; ; )
  {  /* Find a contiguous cluster block */
    eRetVal = eEFPrvFATGet( pxFS, clst, &n );
    if ( ++clst >= pxFS->u32FatEntriesNb )
    {
      clst = 2;
    }
    if ( EF_RET_OK != eRetVal )
    {
      break;
    }
    /* If it is a free cluster? */
    if ( 0 == n )
    {
      /* If a contiguous cluster block is found */
      if ( ++ncl == tcl )
      {
        break;
      }
    }
    else
    {
      /* Not a free cluster */
      scl = clst;
      ncl = 0;
    }
    if ( clst == stcl )
    {
      eRetVal = EF_RET_DENIED;
      break;
    }  /* No contiguous cluster? */
  }
  if ( EF_RET_OK == eRetVal )  /* A contiguous free area is found */
  {
    if ( 0 != opt )    /* Allocate it now */
    {
      for ( clst = scl, n = tcl; n; clst++, n-- )  /* Create a cluster chain on the FAT */
      {
        eRetVal = eEFPrvFATSet( pxFS, clst, (n == 1) ? 0xFFFFFFFF : clst + 1);
        if ( EF_RET_OK != eRetVal )
        {
          break;
        }
        lclst = clst;
      }
    }
    else
    {
      /* Set it as suggested point for next allocation */
      lclst = scl - 1;
    }
  }

  if ( EF_RET_OK == eRetVal )
  {
    pxFS->u32ClstLast = lclst;    /* Set suggested start cluster to start next */
    /* Is it allocated now? */
    if ( 0 != opt )
    {
      pxFile->xObject.u32ClstStart = scl;    /* Update object allocation information */
      pxFile->u32Size = fsz;
      pxFile->u8StatusFlags |= EF_FILE_MODIFIED;
      if ( pxFS->u32ClstFreeNb <= ( pxFS->u32FatEntriesNb - 2 ) ) /* Update FSINFO */
      {
        pxFS->u32ClstFreeNb -= tcl;
        pxFS->u8FsInfoFlags |= 1;
      }
    }
  }

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

