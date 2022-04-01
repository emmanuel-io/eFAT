/**
 * ********************************************************************************************************************
 *  @file     ef_prv_dirfunc_fat.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Directory handling.
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
#include <ef_port_load_store.h>
#include <ef_port_memory.h>

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/* Read an object from the directory */
ef_return_et eEFPrvDirReadFAT (
  ef_directory_st * pxDir
)
{
  ef_return_et    eRetVal = EF_RET_NO_FILE;
  ef_fs_st      * pxFS    = pxDir->xObject.pxFS;
  ef_u08_t       u8Attrib;
  ef_u08_t       b;

  while ( 0 != pxDir->xSector )
  {
    eRetVal = eEFPrvFSWindowLoad( pxFS, pxDir->xSector );
    if ( EF_RET_OK != eRetVal )
    {
      break;
    }
    /* Test for the entry type */
    b = pxDir->pu8Dir[ EF_DIR_NAME_START ];
    /* Reached to end of the directory */
    if ( 0 == b )
    {
      eRetVal = EF_RET_NO_FILE;
      break;
    }
    /* Get attribute */
    u8Attrib = pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ] & EF_DIR_ATTRIB_BITS_DEFINED;
    pxDir->xObject.u8Attrib = u8Attrib;
    /* Is it a valid entry? */
    if (    ( EF_DIR_DELETED_MASK != b )
         && ( '.' != b )
         && ( EF_DIR_ATTRIB_BITS_LFN != u8Attrib )
         && ( EF_DIR_ATTRIB_BIT_VOLUME_ID != ( ~EF_DIR_ATTRIB_BIT_ARCHIVE & u8Attrib ) ) )
    {
      break;
    }
    /* Next entry */
    eRetVal = eEFPrvDirectoryIndexNext( pxDir, EF_BOOL_FALSE );
    if ( EF_RET_OK != eRetVal )
    {
      break;
    }
  }

  if ( EF_RET_OK != eRetVal )
  {
    /* Terminate the read operation on error or EOT */
    pxDir->xSector = 0;
  }
  return eRetVal;
}

/* Directory handling - Find an object in the directory */
ef_return_et eEFPrvDirFindFAT (
  ef_directory_st * pxDir
)
{
  ef_fs_st  * pxFS = pxDir->xObject.pxFS;

  /* Rewind directory object */
  ef_return_et eRetVal = eEFPrvDirectoryIndexSet( pxDir, 0 );
  if ( EF_RET_OK != eRetVal ) { (void) EF_RETURN_CODE_HANDLER( eRetVal );}
  if ( EF_RET_OK == eRetVal )
  {
    do
    {
      eRetVal = eEFPrvFSWindowLoad( pxFS, pxDir->xSector );
      if ( EF_RET_OK != eRetVal ) { (void) EF_RETURN_CODE_HANDLER( eRetVal );}
      if ( EF_RET_OK != eRetVal )
      {
        break;
      }
      /* If we Reached to end of table */
      if ( 0 == pxDir->pu8Dir[ EF_DIR_NAME_START ] )
      {
        eRetVal = EF_RET_NO_FILE;
        break;
      }
      /* Non LFN configuration */
      pxDir->xObject.u8Attrib = EF_DIR_ATTRIB_BITS_DEFINED & pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ];
      /* If it is a NOT valid entry */
      if (   ( 0 == ( EF_DIR_ATTRIB_BIT_VOLUME_ID & pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ] ) )
          && ( EF_RET_OK == eEFPortMemCompare( pxDir->pu8Dir, pxDir->u8Name, 11 ) ) )
      {
        break;
      }
      /* Next entry */
      eRetVal = eEFPrvDirectoryIndexNext( pxDir, EF_BOOL_FALSE );
      if ( EF_RET_OK != eRetVal ) { (void) EF_RETURN_CODE_HANDLER( eRetVal );}
    }
    while ( EF_RET_OK == eRetVal );
  }
  return eRetVal;
}

/* Register an object to the directory */
ef_return_et eEFPrvDirRegisterFAT (
  ef_directory_st * pxDir
)
{
  ef_fs_st  * pxFS = pxDir->xObject.pxFS;
  ef_return_et eRetVal = EF_RET_OK;

  /* Allocate an entry for SFN */
  if ( EF_RET_OK != eEFPrvDirectoryAllocate( pxDir, 1 ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
  }
  /* Set SFN entry */
  else if ( EF_RET_OK != eEFPrvFSWindowLoad( pxFS, pxDir->xSector ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
  }
  /* Clean the entry */
  else if ( EF_RET_OK != eEFPortMemZero( pxDir->pu8Dir, EF_DIR_ENTRY_SIZE ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  /* Put SFN */
  else if ( EF_RET_OK != eEFPortMemCopy( pxDir->u8Name, pxDir->pu8Dir + EF_DIR_NAME_START, 11 ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  else
  {
    pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
  }

  return eRetVal;
}
//if ( EF_RET_OK != eRetVal ) { (void) EF_RETURN_CODE_HANDLER( eRetVal );}

/* Remove an object from the directory */
ef_return_et eEFPrvDirRemoveFAT (
  ef_directory_st * pxDir
)
{
  ef_fs_st  * pxFS = pxDir->xObject.pxFS;
  ef_return_et eRetVal = EF_RET_OK;

  if ( EF_RET_OK != eEFPrvFSWindowLoad( pxFS, pxDir->xSector ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  else
  {
    /* Mark the entry 'deleted'.*/
    pxDir->pu8Dir[ EF_DIR_NAME_START ] = EF_DIR_DELETED_MASK;
    pxFS->u8WinFlags          = EF_FS_WIN_DIRTY;
  }

  return eRetVal;
}
/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

