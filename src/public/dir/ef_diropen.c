/**
 * ********************************************************************************************************************
 *  @file     ef_diropen.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Create a Directory Object
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
#include <ef_prv_volume_mount.h>
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
#include "ef_prv_path_follow.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_diropen (
  EF_DIR      * pxDir,
  const TCHAR * pxPath
)
{
  EF_ASSERT_PUBLIC( 0 != pxDir );
  EF_ASSERT_PUBLIC( 0 != pxPath );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st    * pxFS;
  EF_LFN_BUFFER_DEFINE

  if ( 0 == pxDir )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_OBJECT );
  }
  /* Get logical drive */
  else if ( EF_RET_OK != eEFPrvVolumeMountCheck( &pxPath, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    ef_return_et  eResult;

    pxDir->xObject.pxFS = pxFS;

    /* If LFN BUFFER initialization failed */
    if ( EF_RET_OK != EF_LFN_BUFFER_SET( pxFS ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    /* Follow the pxPath to the directory */
    else if ( EF_RET_OK != eEFPrvPathFollow( pxPath, pxDir, &eResult ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    else if ( EF_RET_NO_FILE == eResult )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_PATH );
    }
    else if ( EF_RET_OK != eResult )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    else
    { /* Follow completed */
      /* It is not the origin directory itself */
      if ( 0 == ( EF_NS_NONAME & pxDir->u8Name[EF_NSFLAG] ) )
      {
        /* This object is not a sub-directory */
        if ( 0 == (pxDir->xObject.u8Attrib & EF_DIR_ATTRIB_BIT_DIRECTORY) )
        {
          /* This object is a file */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_PATH );
        }
        else
        {
          /* Get object allocation info */
          if ( EF_RET_OK != eEFPrvDirectoryClusterGet( pxFS, pxDir->pu8Dir, &(pxDir->xObject.u32ClstStart) ) )
          {
            eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
          }
          else
          {
            EF_CODE_COVERAGE( );
          }
        }
      }
      if ( EF_RET_OK != eRetVal )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      else
      {
        pxDir->xObject.u16MountId = pxFS->u16MountId;
        /* Rewind directory */
        if ( EF_RET_OK != eEFPrvDirectoryIndexSet( pxDir, 0 ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
        }
        else if ( 0 != pxDir->xObject.u32ClstStart )
        {
          (void) eEFPrvLockInc( pxDir, 0, &(pxDir->xObject.u32LockId) );  /* Lock the sub directory */
          if ( 0 == pxDir->xObject.u32LockId )
          {
            eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_TOO_MANY_OPEN_FILES );
          }
          else
          {
            EF_CODE_COVERAGE( );
          }
        }
        else
        {
          pxDir->xObject.u32LockId = 0;  /* Root directory need not to be locked */
        }
      }
    } /* Follow completed */
    EF_LFN_BUFFER_FREE();
  }
  if ( EF_RET_OK != eRetVal )
  {
    pxDir->xObject.pxFS = 0;    /* Invalidate the directory object if function failed */
  }
  else
  {
    EF_CODE_COVERAGE( );
  }

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
