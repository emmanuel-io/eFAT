/**
 * ********************************************************************************************************************
 *  @file     ef_chmod.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Change Attributes
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
#include <ef_prv_volume_mount.h>
#include "ef_port_diskio.h"
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
#include "ef_prv_path_follow.h"
#include <ef_port_load_store.h>

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_chmod (
  const TCHAR * pxPath,
  ef_u08_t      u8Attrib,
  ef_u08_t      u8Mask
)
{
  EF_ASSERT_PUBLIC( 0 != pxPath );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st    * pxFS;

  if ( EF_RET_OK != eEFPrvVolumeMountCheck( &pxPath, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    ef_directory_st xDir;
    ef_bool_t bFound = EF_BOOL_FALSE;

    EF_LFN_BUFFER_DEFINE

    xDir.xObject.pxFS = pxFS;

    /* If LFN BUFFER initialization failed */
    if ( EF_RET_OK != EF_LFN_BUFFER_SET( pxFS ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    /* Else, if following file path failed */
    else if ( EF_RET_OK != eEFPrvPathFollow( pxPath, &xDir, &bFound ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    /* Else, if following file path failed */
    else if ( EF_BOOL_TRUE != bFound )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    /* Else, if it is dot entry or no name */
    else if ( 0 != ( (EF_NS_DOT | EF_NS_NONAME) & xDir.u8Name[ EF_NSFLAG ] ) )
    {
      /* Cannot remove dot entry */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    else
    {
      /* Valid attribute mask */
      u8Mask &=   EF_DIR_ATTRIB_BIT_READONLY
                | EF_DIR_ATTRIB_BIT_HIDDEN
                | EF_DIR_ATTRIB_BIT_SYSTEM
                | EF_DIR_ATTRIB_BIT_ARCHIVE;
      /* Apply attribute change */
      u8Attrib &= u8Mask;
      u8Mask = (ef_u08_t)~u8Mask;
      xDir.pu8Dir[ EF_DIR_ATTRIBUTES ] =   u8Attrib
                                         | (   xDir.pu8Dir[ EF_DIR_ATTRIBUTES ]
                                             & u8Mask );
      pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
      if ( EF_RET_OK != eEFPrvFSSync( pxFS ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
    }
    EF_LFN_BUFFER_FREE( );
  }

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

