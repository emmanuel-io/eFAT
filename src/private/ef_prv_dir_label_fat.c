/**
 * ********************************************************************************************************************
 *  @file     ef_prv_dir_label_vfat.c
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
 *
 *  The copyright owners or contributors be NOT LIABLE for any damages caused by use of this software.
 * ********************************************************************************************************************
 */

/* START OF FILE *************************************************************************************************** */
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include <ef_port_load_store.h>
#include <ef_port_memory.h>
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

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/* Read an object from the directory */
ef_return_et eEFPrvLabelReadFAT (
  ef_directory_st * pxDir
)
{
  EF_ASSERT_PRIVATE( 0 != pxDir );

  ef_return_et    eRetVal = EF_RET_NO_FILE;
  ef_fs_st      * pxFS    = pxDir->xObject.pxFS;
  ef_u08_t        u8Attrib;
  ef_u08_t        u8EntryType;

  while ( 0 != pxDir->xSector )
  {
    if ( EF_RET_OK != eEFPrvFSWindowLoad( pxFS, pxDir->xSector ) )
    {
      eRetVal = EF_RET_ERROR;
      break;
    }
    /* Test for the entry type */
    u8EntryType = pxDir->pu8Dir[ EF_DIR_NAME_START ];
    /* Reached to end of the directory */
    if ( 0 == u8EntryType )
    {
      break;
    }
    /* Get attribute */
    u8Attrib = pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ] & EF_DIR_ATTRIB_BITS_DEFINED;
    pxDir->xObject.u8Attrib = u8Attrib;
    /* Is it a valid entry? */
//    if (    ( EF_DIR_DELETED_MASK != u8EntryType )
//         && ( '.' != u8EntryType )
//         && ( EF_DIR_ATTRIB_BITS_LFN != u8Attrib )
//         && (int) ((u8Attrib & ~EF_DIR_ATTRIB_BIT_ARCHIVE) == EF_DIR_ATTRIB_BIT_VOLUME_ID) == vol )
//         && ( EF_DIR_ATTRIB_BIT_VOLUME_ID == ( ~EF_DIR_ATTRIB_BIT_ARCHIVE & u8Attrib ) ) )
    if ( EF_DIR_DELETED_MASK == u8EntryType )
    {
      EF_CODE_COVERAGE( );
    }
    else if ( '.' == u8EntryType )
    {
      EF_CODE_COVERAGE( );
    }
    else if ( EF_DIR_ATTRIB_BITS_LFN == u8Attrib )
    {
      EF_CODE_COVERAGE( );
    }
    else if ( EF_DIR_ATTRIB_BIT_VOLUME_ID != ( ~EF_DIR_ATTRIB_BIT_ARCHIVE & u8Attrib ) )
    {
      EF_CODE_COVERAGE( );
    }
    else
    {
      break;
      eRetVal = EF_RET_OK;
    }
    /* Next entry */
    if ( EF_RET_OK != eEFPrvDirectoryIndexNext( pxDir, EF_BOOL_FALSE ) )
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

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

