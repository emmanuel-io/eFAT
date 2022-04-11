/**
 * ********************************************************************************************************************
 *  @file     ef_getcwd.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Get current working directory
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
#include "ef_prv_file.h"
#include "ef_prv_fs_window.h"
#include "ef_prv_lock.h"
#include "ef_prv_string.h"
#include "ef_prv_volume.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_unicode.h"
#include "ef_prv_validate.h"
#include "ef_prv_volume_nb.h"
#include "ef_prv_volume.h"
#include "ef_port_diskio.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_getcwd (
  TCHAR   * pxString,
  ef_u32_t  len
)
{
  EF_ASSERT_PUBLIC( 0 != pxString );

  ef_fs_st    * pxFS;
  ef_return_et  eRetVal = EF_RET_OK;
  TCHAR       * pxTempString = pxString;

  /* Get logical drive */
  /* Set null string to get current volume */
  pxString[ 0 ] = 0;

  /* Get current volume */
  if ( EF_RET_OK != eEFPrvVolumeMountCheck( (const TCHAR**) &pxString, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    ef_directory_st xDir;
    ef_u32_t        i;
    ef_u32_t        n;
    ef_u32_t        ccl;
    ef_file_info_st xFileInfo;

    EF_LFN_BUFFER_DEFINE

    xDir.xObject.pxFS = pxFS;
    eRetVal = EF_LFN_BUFFER_SET( pxFS );

    /* Follow parent directories and create the pxPath */
    i = len;      /* Bottom of pxStringer (directory stack base) */
    /* Start to follow upper directory from current directory */
    xDir.xObject.u32ClstStart = pxFS->u32DirClstCurrent;

    /* Repeat while current directory is a sub-directory */
    while ( 0 != ( xDir.xObject.u32ClstStart ) )
    {
      /* Get parent directory */
      eRetVal = eEFPrvDirectoryIndexSet( &xDir, 1 * EF_DIR_ENTRY_SIZE );
      if ( EF_RET_OK != eRetVal )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
        break;
      }
      eRetVal = eEFPrvFSWindowLoad( pxFS, xDir.xSector );
      if ( EF_RET_OK != eRetVal )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
        break;
      }
      /* Goto parent directory */
      //xDir.xObject.u32ClstStart = eEFPrvDirectoryClusterGet( pxFS, xDir.pu8Dir );
      if ( EF_RET_OK != eEFPrvDirectoryClusterGet( pxFS, xDir.pu8Dir, &(xDir.xObject.u32ClstStart) ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
      }
      eRetVal = eEFPrvDirectoryIndexSet( &xDir, 0 );
      if ( EF_RET_OK != eRetVal )
      {
        break;
      }
      do {
        /* Find the entry links to the child directory */
        eRetVal = DIR_READ_FILE( &xDir );
        if ( EF_RET_OK != eRetVal )
        {
          break;
        }
        else
        {
          ef_u32_t  u32Cluster;
          if ( EF_RET_OK != eEFPrvDirectoryClusterGet( pxFS, xDir.pu8Dir, &u32Cluster ) )
          {
            eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
          }
          //if ( ccl == eEFPrvDirectoryClusterGet( pxFS, xDir.pu8Dir ) )
          if ( ccl == u32Cluster )
          {
            break; /* Found the entry */
          }
        }
        eRetVal = eEFPrvDirectoryIndexNext( &xDir, EF_BOOL_FALSE );
      } while ( EF_RET_OK == eRetVal );

      if  (EF_RET_NO_FILE == eRetVal )
      {
        /* It cannot be 'not found'. */
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
      }
      if ( EF_RET_OK != eRetVal )
      {
        break;
      }
      /* Get the directory name and push it to the pxStringer */
      (void) eEFPrvDirFileInfosGet( &xDir, &xFileInfo );
      /* Name length */
      for ( n = 0 ; xFileInfo.xName[ n ] ; n++ ) ;
      /* Insufficient space to store the path name? */
      if ( i < ( n + 1 ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NOT_ENOUGH_CORE );
        break;
      }
      while ( 0 != n )
      {
        /* Stack the name */
        pxString[ --i ] = xFileInfo.xName[ --n ];
      }
      pxString[ --i ] = '/';
    }
    if ( EF_RET_OK == eRetVal )
    {
      if ( i == len )
      {
        pxString[--i] = '/';  /* Is it the root-directory? */
      }
      if ( EF_CONF_VOLUMES_NB >= 2 )     /* Put drive prefix */
      {
        ef_u32_t vl = 0;
        /* Numeric volume ID */
        if ( i >= 3 )
        {
          int8_t  s8VolumeNb;
          (void) eEFPrvVolumeNbCurrentGet( &s8VolumeNb );
          *pxTempString++ = (TCHAR)'A' + s8VolumeNb;
          *pxTempString++ = (TCHAR)':';
          vl = 2;
        }
        if ( 0 == vl )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NOT_ENOUGH_CORE );
        }
        else
        {
          EF_CODE_COVERAGE( );
        }

      }
      /* Add current directory pxPath */
      if ( EF_RET_OK != eRetVal )
      {
        EF_CODE_COVERAGE( );
      }
      else
      {
        do
        {
          *pxTempString++ = pxString[ i++ ];
        }
        while ( i < len );  /* Copy stacked pxPath string */
      }
    }
    EF_LFN_BUFFER_FREE();
  }

  *pxTempString = 0;

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
