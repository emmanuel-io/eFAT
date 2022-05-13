/**
 * ********************************************************************************************************************
 *  @file     ef_rename.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Rename a File/Directory
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
#include "ef_port_diskio.h"
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

ef_return_et eEF_rename (
  const TCHAR * pxPath_old,
  const TCHAR * pxPath_new
)
{
  EF_ASSERT_PUBLIC( 0 != pxPath_old );
  EF_ASSERT_PUBLIC( 0 != pxPath_new );

  ef_return_et      eRetVal = EF_RET_OK;
  ef_directory_st   xDirOld;
  ef_directory_st   xDirNew;
  ef_fs_st        * pxFS;
  ef_u08_t          buf[ EF_DIR_ENTRY_SIZE ];
  ef_u08_t        * pu8Dir;
  ef_lba_t          xSector;
  EF_LFN_BUFFER_DEFINE


  /* Snip the drive number of new name off */
  if ( EF_RET_OK != eEFPrvVolumeNbPathRemove( &pxPath_new ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
  }
  /* Get logical drive of the old object */
  else if ( EF_RET_OK != eEFPrvVolumeMountCheck( &pxPath_old, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
  }
  else
  {
    ef_bool_t bFound = EF_BOOL_FALSE;

    xDirOld.xObject.pxFS = pxFS;

    /* If LFN BUFFER initialization failed */
    if ( EF_RET_OK != EF_LFN_BUFFER_SET( pxFS ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    /* Else, if following file path failed */
    else if ( EF_RET_OK != eEFPrvPathFollow( pxPath_old, &xDirOld, &bFound ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    else if ( EF_BOOL_TRUE != bFound )
    {
      /* Check validity of name */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    /* Check old object */
    else if ( 0 != ( ( EF_NS_DOT | EF_NS_NONAME ) & xDirOld.u8Name[ EF_NSFLAG ] ) )
    {
      /* Check validity of name */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    /* Check if the file can be accessed */
    else if ( EF_RET_OK != eEFPrvLockCheck( &xDirOld, 2 ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    /* Object to be renamed is found */
    else
//      if ( EF_RET_OK == eRetVal )
    {
      ef_bool_t bFound = EF_BOOL_FALSE;

      /* Save directory entry of the object */
      if ( EF_RET_OK != eEFPortMemCopy( xDirOld.pu8Dir, buf, EF_DIR_ENTRY_SIZE ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      /* Duplicate the directory object */
      else if ( EF_RET_OK != eEFPortMemCopy( &xDirOld, &xDirNew, sizeof(ef_directory_st) ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      /* Make sure if new object name is not in use */
      else if ( EF_RET_OK != eEFPrvPathFollow( pxPath_new, &xDirNew, &bFound ) )
      {
        /* Check validity of name */
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
      }
      else if ( EF_BOOL_TRUE == bFound )
      {/* Is new name already in use by any other object? */
        if (    ( xDirNew.xObject.u32ClstStart == xDirOld.xObject.u32ClstStart )
             && ( xDirNew.u32Offset == xDirOld.u32Offset ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILE );
        }
        else
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_EXIST );
        }
      }
      /* It is a valid pxPath and no name collision */
      if ( EF_RET_NO_FILE == eRetVal )
      {
        /* Register the new entry */
        eRetVal = eEFPrvDirRegister( &xDirNew );
        if ( EF_RET_OK == eRetVal )
        {
          /* Copy directory entry of the object except name */
          pu8Dir = xDirNew.pu8Dir;
          eEFPortMemCopy( buf + 13, pu8Dir + 13, EF_DIR_ENTRY_SIZE - 13 );
          pu8Dir[ EF_DIR_ATTRIBUTES ] = buf[ EF_DIR_ATTRIBUTES ];
          if ( 0 == ( pu8Dir[ EF_DIR_ATTRIBUTES ] & EF_DIR_ATTRIB_BIT_DIRECTORY ) )
          {
            /* Set archive attribute if it is a file */
            pu8Dir[EF_DIR_ATTRIBUTES] |= EF_DIR_ATTRIB_BIT_ARCHIVE;
          }
          pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
          if (    ( 0 != ( EF_DIR_ATTRIB_BIT_DIRECTORY & pu8Dir[ EF_DIR_ATTRIBUTES ] ) )
               && ( xDirOld.xObject.u32ClstStart != xDirNew.xObject.u32ClstStart ) )
          {
            ef_u32_t  u32Cluster;
            if ( EF_RET_OK != eEFPrvDirectoryClusterGet(  pxFS,
                                                  pu8Dir,
                                                  &u32Cluster ) )
            {
              eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
            }
            /* Update .. entry in the sub-directory if needed */
            else if ( EF_RET_OK != eEFPrvFATClusterToSector( pxFS, u32Cluster, &xSector ) )
            {
              eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
            }
            else
            {
              /* Start of critical section where an interruption can cause a cross-link */
              eRetVal = eEFPrvFSWindowLoad( pxFS, xSector );
              /* Ptr to .. entry */
              pu8Dir = pxFS->pu8Window + EF_DIR_ENTRY_SIZE * 1;
              if (    ( EF_RET_OK == eRetVal )
                   && ( '.' == pu8Dir[ 1 ] ) )
              {
                (void) eEFPrvDirectoryClusterSet( pxFS, pu8Dir, xDirNew.xObject.u32ClstStart );
                pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
              }
            }
          }
        }
      }
      if ( EF_RET_OK != eRetVal )
      {
        EF_CODE_COVERAGE( );
      }
      else
      {
        /* Remove old entry */
        if ( EF_RET_OK != eEFPrvDirRemove( &xDirOld ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
        }
        else if ( EF_RET_OK != eEFPrvFSSync( pxFS ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
      }
      /* End of the critical section */
    }
    EF_LFN_BUFFER_FREE();
  }

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
