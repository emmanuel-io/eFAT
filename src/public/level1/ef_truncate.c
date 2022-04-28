/**
 * ********************************************************************************************************************
 *  @file     ef_truncate.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Truncate File
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
#include <ef_prv_fat.h>
#include "ef_prv_def.h"
#include "ef_prv_lock.h"
#include "ef_prv_validate.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_truncate (
  EF_FILE  *  pxFile
)
{
  EF_ASSERT_PUBLIC( 0 != pxFile );

  ef_return_et  eRetVal;
  ef_fs_st    * pxFS;
  ef_u32_t     ncl;


  /* Check validity of the file object */
  eRetVal = eEFPrvValidateObject( &pxFile->xObject, &pxFS );
  if ( EF_RET_OK != eRetVal )
  {
    (void) eEFPrvFSUnlock( pxFS, eRetVal );
    return eRetVal;
  }
  eRetVal = (ef_return_et) pxFile->u8ErrorCode;
  if ( EF_RET_OK != eRetVal )
  {
    (void) eEFPrvFSUnlock( pxFS, eRetVal );
    return eRetVal;
  }
  if ( 0 == ( pxFile->u8StatusFlags & EF_FILE_OPEN_WRITE ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DENIED );  /* Check access u8Mode */
    (void) eEFPrvFSUnlock( pxFS, eRetVal );
    return eRetVal;
  }

  if ( pxFile->u32FileOffset < pxFile->u32Size )
  {  /* Process when u32FileOffset is not on the eof */
    if ( 0 == pxFile->u32FileOffset )
    {  /* When set file size to zero, remove entire cluster chain */
      eRetVal = eEFPrvFATChainRemove( &pxFile->xObject, pxFile->xObject.u32ClstStart, 0 );
      pxFile->xObject.u32ClstStart = 0;
    }
    else
    {
      eRetVal = EF_RET_OK;
      /* When truncate a part of the file, remove remaining clusters */
      eRetVal = eEFPrvFATGet( pxFS, pxFile->u32Clst, &ncl );
      if (    ( EF_RET_OK == eRetVal )
           && ( ncl < pxFS->u32FatEntriesNb ) )
      {
        eRetVal = eEFPrvFATChainRemove( &pxFile->xObject, ncl, pxFile->u32Clst );
      }
    }
    /* Set file size to current read/write point */
    pxFile->u32Size = pxFile->u32FileOffset;
    pxFile->u8StatusFlags |= EF_FILE_MODIFIED;
    if ( EF_RET_OK != eRetVal )
    {
      pxFile->u8ErrorCode = (ef_u08_t)(eRetVal);
      (void) eEFPrvFSUnlock( pxFS, eRetVal );
      return eRetVal;
    }
  }

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
