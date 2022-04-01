/**
 * ********************************************************************************************************************
 *  @file     ef_prv_file_info_get_fat.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Code file for functions.
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
#include "ef_port_diskio.h"


/* Includes -------------------------------------------------------------------------------------------------------- */
/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/* Get file information from directory entry */
ef_return_et eEFPrvDirFileInfosGetFAT (
  ef_directory_st * pxDir,
  ef_file_info_st * pxFileInfo
)
{
  EF_ASSERT_PRIVATE( 0 != pxDir );
  EF_ASSERT_PRIVATE( 0 != pxFileInfo );

  ef_return_et  eRetVal = EF_RET_ERROR;

  /* Invalidate file info */
  pxFileInfo->xName[ 0 ] = 0;
  /* If read pointer has not reached end of directory */
  if ( 0 != pxDir->xSector )
  {
    ef_u32_t  u32IdxDst = 0;
    /* Copy name body and extension */
    for ( ef_u32_t u32IdxSrc = 0 ; 11 > u32IdxSrc ; u32IdxSrc++ )
    {
      if ( 8 == u32IdxSrc )
      {
        /* Insert a . if extension is exist */
        pxFileInfo->xName[ u32IdxDst++ ] = '.';
      }
      TCHAR c = (TCHAR)pxDir->pu8Dir[ u32IdxSrc ];
      /* If a padding space */
      if ( ' ' == c )
      {
        /* Skip padding space */
        continue;
      }
      /* Else, if a replaced EF_DIR_DELETED_MASK character */
      else if ( EF_DIR_REPLACEMENT_CHAR == c )
      {
        /* Restore replaced EF_DIR_DELETED_MASK character */
        c = EF_DIR_DELETED_MASK;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
      /* Else, if a replaced EF_DIR_DELETED_MASK character */
      pxFileInfo->xName[ u32IdxDst++ ] = c;
    }
    pxFileInfo->xName[ u32IdxDst ] = 0;

    /* Attributes */
    pxFileInfo->u8Attrib    = pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ];
    /* Size */
    pxFileInfo->u32FileSize = u32EFPortLoad( pxDir->pu8Dir + EF_DIR_FILE_SIZE );
    /* Time */
    pxFileInfo->u16Time     = u16EFPortLoad( pxDir->pu8Dir + EF_DIR_TIME_MODIFIED + 0 );
    /* Date */
    pxFileInfo->u16Date     = u16EFPortLoad( pxDir->pu8Dir + EF_DIR_TIME_MODIFIED + 2 );

    eRetVal = EF_RET_OK;
  }
  else
  {
    EF_CODE_COVERAGE( );
  }

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
