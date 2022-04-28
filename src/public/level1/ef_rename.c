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

  ef_return_et      eRetVal;
  ef_directory_st   djo;
  ef_directory_st   djn;
  ef_fs_st        * pxFS;
  ef_u08_t          buf[ EF_DIR_ENTRY_SIZE ];
  ef_u08_t        * pu8Dir;
  ef_lba_t          xSector;
  EF_LFN_BUFFER_DEFINE


  /* Snip the drive number of new name off */
  eEFPrvVolumeNbPathRemove( &pxPath_new );
  /* Get logical drive of the old object */
  eRetVal = eEFPrvVolumeMountCheck( &pxPath_old, &pxFS );
  if ( EF_RET_OK == eRetVal )
  {
    ef_return_et  eResult;

    djo.xObject.pxFS = pxFS;
    eRetVal = EF_LFN_BUFFER_SET( pxFS );
    eRetVal = eEFPrvPathFollow( pxPath_old, &djo, &eResult );
    /* Check old object */
    if (    ( EF_RET_OK == eRetVal )
         && ( 0 != ( ( EF_NS_DOT | EF_NS_NONAME ) & djo.u8Name[ EF_NSFLAG ] ) ) )
    {
      /* Check validity of name */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    if ( EF_RET_OK == eRetVal )
    {
      eRetVal = eEFPrvLockCheck( &djo, 2 );
    }
    /* Object to be renamed is found */
    if ( EF_RET_OK == eRetVal )
    {
      ef_return_et  eResult;

      /* Save directory entry of the object */
      eEFPortMemCopy( djo.pu8Dir, buf, EF_DIR_ENTRY_SIZE );
      /* Duplicate the directory object */
      eEFPortMemCopy( &djo, &djn, sizeof(ef_directory_st) );
      /* Make sure if new object name is not in use */
      eRetVal = eEFPrvPathFollow( pxPath_new, &djn, &eResult );
      if ( EF_RET_OK == eRetVal )
      {/* Is new name already in use by any other object? */
        if (    ( djn.xObject.u32ClstStart == djo.xObject.u32ClstStart )
             && ( djn.u32Offset == djo.u32Offset ) )
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
        eRetVal = eEFPrvDirRegister( &djn );
        if ( EF_RET_OK == eRetVal )
        {
          /* Copy directory entry of the object except name */
          pu8Dir = djn.pu8Dir;
          eEFPortMemCopy( buf + 13, pu8Dir + 13, EF_DIR_ENTRY_SIZE - 13 );
          pu8Dir[ EF_DIR_ATTRIBUTES ] = buf[ EF_DIR_ATTRIBUTES ];
          if ( 0 == ( pu8Dir[ EF_DIR_ATTRIBUTES ] & EF_DIR_ATTRIB_BIT_DIRECTORY ) )
          {
            /* Set archive attribute if it is a file */
            pu8Dir[EF_DIR_ATTRIBUTES] |= EF_DIR_ATTRIB_BIT_ARCHIVE;
          }
          pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
          if (    ( 0 != ( EF_DIR_ATTRIB_BIT_DIRECTORY & pu8Dir[ EF_DIR_ATTRIBUTES ] ) )
               && ( djo.xObject.u32ClstStart != djn.xObject.u32ClstStart ) )
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
                (void) eEFPrvDirectoryClusterSet( pxFS, pu8Dir, djn.xObject.u32ClstStart );
                pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
              }
            }
          }
        }
      }
      if ( EF_RET_OK == eRetVal )
      {
        /* Remove old entry */
        eRetVal = eEFPrvDirRemove( &djo );
        if ( EF_RET_OK == eRetVal )
        {
          eRetVal = eEFPrvFSSync( pxFS );
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
