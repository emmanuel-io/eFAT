/**
 * ********************************************************************************************************************
 *  @file     ef_remove.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Delete a File/Directory
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
#include "ef_port_diskio.h"
#include <ef_prv_def.h>
#include <ef_prv_fat.h>
#include <ef_prv_volume_mount.h>
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

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_remove (
  const TCHAR * pxPath
)
{
  EF_ASSERT_PUBLIC( 0 != pxPath );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st    * pxFS;


  /* Get logical drive */
  if ( EF_RET_OK != eEFPrvVolumeMountCheck( &pxPath, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    ef_directory_st   xDir;
    ef_u32_t          u32DirCluster = 0;
    ef_bool_t         bFound = EF_BOOL_FALSE;

    EF_LFN_BUFFER_DEFINE

    xDir.xObject.pxFS = pxFS;

    /* If LFN BUFFER initialisation failed */
    if ( EF_RET_OK != EF_LFN_BUFFER_SET( pxFS ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    /* Else, if following file path failed */
    else if ( EF_RET_OK != eEFPrvPathFollow( pxPath, &xDir, &bFound ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    else if ( EF_BOOL_TRUE != bFound )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    /* Else, if it is dot entry */
    else if (    ( 0 != EF_CONF_RELATIVE_PATH )
              && ( 0 != ( EF_NS_DOT & xDir.u8Name[ EF_NSFLAG ] ) ) )
    {
      /* Cannot remove dot entry */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    /* Else, If it is a locked object */
    else if ( EF_RET_OK != eEFPrvLockCheck( &xDir, 2 ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    /* Else, if it is the origin directory */
    else if ( 0 != ( EF_NS_NONAME & xDir.u8Name[ EF_NSFLAG ] ) )
    {
      /* Cannot remove the origin directory */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    /* Else, if it is a read-only object */
    else if ( 0 != ( EF_DIR_ATTRIB_BIT_READONLY & xDir.xObject.u8Attrib ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DENIED );
    }
    else if ( EF_RET_OK != eEFPrvDirectoryClusterGet( pxFS, xDir.pu8Dir, &u32DirCluster ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
    /* Is it a sub-directory? */
    if ( 0 != ( xDir.xObject.u8Attrib & EF_DIR_ATTRIB_BIT_DIRECTORY ) )
    {
      /* Is it the current directory? */
      if (    ( 0 != EF_CONF_RELATIVE_PATH )
           && ( u32DirCluster == pxFS->u32DirClstCurrent ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DENIED );
      }
      else
      {
        ef_bool_t bEmpty = EF_BOOL_FALSE;
        ef_directory_st xSubDir;
        /* Open the sub-directory */
        xSubDir.xObject.pxFS = pxFS;
        xSubDir.xObject.u32ClstStart = u32DirCluster;

        if ( EF_RET_OK != eEFPrvDirectoryIndexSet( &xSubDir, 0 ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
        }
        /* Test if the directory is empty */
        else if ( EF_RET_OK != eEFPrvDirRead( &xSubDir, &bEmpty ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
        }
        /* Not empty? */
        else if ( EF_BOOL_FALSE == bEmpty )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DENIED );
        }
        /* Empty? */
        else if ( EF_BOOL_FALSE == bEmpty )
        {
          eRetVal = EF_RET_OK;
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
      }
    }

    if ( EF_RET_OK != eRetVal )
    {
      EF_CODE_COVERAGE( );
    }
    else
    {
      /* If removing the directory failed */
      if ( EF_RET_OK != eEFPrvDirRemove( &xDir ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      /* Else, if directory cluster is zero */
      else if ( 0 == u32DirCluster )
      {
        EF_CODE_COVERAGE( );
      }
      /* Else, if removing the cluster chain failed */
      else if ( EF_RET_OK != eEFPrvFATChainRemove( &xDir.xObject, u32DirCluster, 0 ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
      if ( EF_RET_OK == eRetVal )
      {
        eRetVal = eEFPrvFSSync( pxFS );
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

