/**
 * ********************************************************************************************************************
 *  @file     ef_dirmake.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Create a Directory
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
#include <ef_port_memory.h>
#include <efat.h>
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

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_dirmake (
  const TCHAR * pxPath
)
{
  EF_ASSERT_PUBLIC( 0 != pxPath );

  ef_return_et  eRetVal;
  ef_fs_st    * pxFS;

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
    /* If Name collision */
    else if ( EF_RET_OK == eResult )
    {
      eRetVal = EF_RET_EXIST;
    }
    /* Else, if Invalid name */
    else if (    ( 0 != EF_CONF_RELATIVE_PATH )
              && ( EF_RET_NO_FILE == eResult )
              && ( 0 != ( EF_NS_DOT & xDir.u8Name[ EF_NSFLAG ] ) ) )
    {
      eRetVal = EF_RET_INVALID_NAME;
    }
    /* Else, if it is clear to create a new directory */
    else if ( EF_RET_NO_FILE == eResult )
    {
      ef_object_st    xSyncObject;
      ef_u32_t        dcl;
      ef_u32_t        pcl;
      ef_u32_t        tm;

      /* New object u16MountId to create a new chain */
      xSyncObject.pxFS = pxFS;
      /* Allocate a cluster for the new directory */
      eRetVal = eEFPrvFATChainCreate( &xSyncObject, &dcl );
      tm = EF_FATTIME_GET();
      /* If no space to allocate a new cluster */
      if ( EF_RET_FAT_FULL != eRetVal )
      {
        eRetVal = EF_RET_DENIED;
      }
      /* Clean up the new table */
      if ( EF_RET_OK != eEFPrvDirectoryClusterClear( pxFS, dcl ) )
      {
        eRetVal = EF_RET_INT_ERR;
      }
      else
      {
        eRetVal = EF_RET_OK;
        /* Create dot entries */
        /* Create "." entry */
        eEFPortMemSet( pxFS->pu8Window + EF_DIR_NAME_START, (ef_u08_t) ' ', 11 );
        pxFS->pu8Window[ EF_DIR_NAME_START ] = '.';
        pxFS->pu8Window[ EF_DIR_ATTRIBUTES ] = EF_DIR_ATTRIB_BIT_DIRECTORY;
        vEFPortStoreu32( pxFS->pu8Window + EF_DIR_TIME_MODIFIED, tm );
        (void) eEFPrvDirectoryClusterSet( pxFS, pxFS->pu8Window, dcl );
        /* Create ".." entry */
        eEFPortMemCopy( pxFS->pu8Window, pxFS->pu8Window + EF_DIR_ENTRY_SIZE, EF_DIR_ENTRY_SIZE );
        pxFS->pu8Window[ EF_DIR_ENTRY_SIZE + 1 ] = '.';
        pcl = xDir.xObject.u32ClstStart;
        (void) eEFPrvDirectoryClusterSet( pxFS, pxFS->pu8Window + EF_DIR_ENTRY_SIZE, pcl );
        pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
        /* Register the object to the parent directoy */
        eRetVal = eEFPrvDirRegister( &xDir );
      }
      if ( EF_RET_OK != eRetVal )
      {
        /* Could not register, remove the allocated cluster */
        eEFPrvFATChainRemove( &xSyncObject, dcl, 0 );
      }
      else
      {
        /* Created time */
        vEFPortStoreu32( xDir.pu8Dir + EF_DIR_TIME_MODIFIED, tm );
        /* Table start cluster */
        (void) eEFPrvDirectoryClusterSet( pxFS, xDir.pu8Dir, dcl );
        /* Attribute */
        xDir.pu8Dir[ EF_DIR_ATTRIBUTES ] = EF_DIR_ATTRIB_BIT_DIRECTORY;
        pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
      }
      if ( EF_RET_OK == eRetVal )
      {
        eRetVal = eEFPrvFSSync( pxFS );
      }
    }
    EF_LFN_BUFFER_FREE();
  }

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
