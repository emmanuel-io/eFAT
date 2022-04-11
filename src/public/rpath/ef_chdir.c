/**
 * ********************************************************************************************************************
 *  @file     ef_chdir.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Change directory
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
#include <ef_prv_lfn.h>
#include <ef_prv_volume_mount.h>
#include "ef_prv_directory.h"
#include "ef_prv_lock.h"
#include "ef_prv_path_follow.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_chdir (
  const TCHAR * pxPath
)
{
  EF_ASSERT_PUBLIC( 0 != pxPath );

  ef_return_et      eRetVal = EF_RET_OK;
  ef_fs_st        * pxFS;

  /* Get logical drive */
  if ( EF_RET_OK != eEFPrvVolumeMountCheck( &pxPath, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    ef_directory_st xDir;
    ef_return_et    eResult;

    EF_LFN_BUFFER_DEFINE

    xDir.xObject.pxFS = pxFS;

    /* If LFN BUFFER initialization failed */
    if ( EF_RET_OK != EF_LFN_BUFFER_SET( pxFS ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    /* Else, if following file path failed */
    else if ( EF_RET_OK != eEFPrvPathFollow( pxPath, &xDir, &eResult ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    else if ( EF_RET_OK != eResult )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    /* Else, if it is the start directory */
    else if ( 0 != ( EF_NS_NONAME & xDir.u8Name[ EF_NSFLAG ] ) )
    {
      pxFS->u32DirClstCurrent = xDir.xObject.u32ClstStart;
    }
    /* Else, if it is not a sub-directory */
    else if ( 0 != ( EF_DIR_ATTRIB_BIT_DIRECTORY & xDir.xObject.u8Attrib ) )
    {
      /* Reached but a file */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_PATH );
    }
    /* Else if getting Sub-directory cluster failed */
    else if ( EF_RET_OK != eEFPrvDirectoryClusterGet( pxFS, xDir.pu8Dir, &(pxFS->u32DirClstCurrent) ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
    EF_LFN_BUFFER_FREE( );
  }

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

