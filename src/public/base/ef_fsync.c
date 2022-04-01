/**
 * ********************************************************************************************************************
 *  @file     ef_fsync.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Synchronize the File
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
#include "ef_prv_drive.h"
#include "ef_prv_directory.h"
#include "ef_prv_file.h"
#include "ef_prv_fs_window.h"
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

ef_return_et eEF_fsync (
  EF_FILE  * pxFile
)
{
  EF_ASSERT_PUBLIC( 0 != pxFile );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st    * pxFS;
  ef_u08_t    * pu8Dir;

  /* If File object is not valid */
  if ( EF_RET_OK != eEFPrvValidateObject( &pxFile->xObject, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  /* Else, if there is no change to the file? */
  else if ( 0 == ( EF_FILE_MODIFIED & pxFile->u8StatusFlags ) )
  {
    EF_CODE_COVERAGE( );
  }
  /* Else, if Write-back cached data failed */
  else if ( EF_RET_OK != eEFPrvFileWindowDirtyWriteBack( pxFile, pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  /* Else, if updating the FS window failed */
  else if ( EF_RET_OK != eEFPrvFSWindowLoad( pxFS, pxFile->xDirSector ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  /* Else, Update the directory entry */
  else
  {
    pu8Dir = pxFile->pu8DirPtr;
    /* Set archive attribute to indicate that the file has been changed */
    pu8Dir[ EF_DIR_ATTRIBUTES ] |= EF_DIR_ATTRIB_BIT_ARCHIVE;
    /* Update file allocation information  */
    if ( EF_RET_OK != eEFPrvDirectoryClusterSet( pxFile->xObject.pxFS, pu8Dir, pxFile->xObject.u32ClstStart ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    else
    {
      /* Get Modified time */
      ef_u32_t  u32TimeStamp = EF_FATTIME_GET( );

      /* Update file size */
      if ( EF_RET_OK != eEFPortStoreu32( pu8Dir + EF_DIR_FILE_SIZE, (ef_u32_t)pxFile->u32Size ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      /* Update modified time */
      else if ( EF_RET_OK != eEFPortStoreu32( pu8Dir + EF_DIR_TIME_MODIFIED, u32TimeStamp ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      else if ( EF_RET_OK != eEFPortStoreu16( pu8Dir + EF_DIR_DATE_LAST_ACCESS, 0 ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      else
      {
        pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
      }

      /* Restore it to the directory */
      if ( EF_RET_OK != eEFPrvFSSync( pxFS ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      else
      {
        pxFile->u8StatusFlags &= (ef_u08_t)~EF_FILE_MODIFIED;
      }
    }
  }

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

