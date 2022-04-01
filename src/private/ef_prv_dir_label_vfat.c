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
ef_return_et eEFPrvLabelReadVFAT (
    ef_directory_st * pxDir
)
{
  ef_return_et  eRetVal = EF_RET_NO_FILE;

#if ( 0 != EF_CONF_VFAT )
  ef_fs_st      * pxFS  = pxDir->xObject.pxFS;
  ef_u08_t        u8Attrib;
  ef_u08_t        ord = 0xFF;
  ef_u08_t        sum = 0xFF;
  int             vol = 1;

  while ( pxDir->xSector )
  {
    eRetVal = eEFPrvFSWindowLoad( pxFS, pxDir->xSector );
    if ( EF_RET_OK != eRetVal )
    {
      break;
    }
    ef_u08_t u8EntryType = pxDir->pu8Dir[ EF_DIR_NAME_START ];  /* Test for the entry type */
    if ( 0 == u8EntryType )
    {
      eRetVal = EF_RET_NO_FILE;
      break; /* Reached to end of the directory */
    }
    u8Attrib = pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ] & EF_DIR_ATTRIB_BITS_DEFINED;
    pxDir->xObject.u8Attrib = u8Attrib;
    /* Get attribute */
    /* Is it a valid entry? */
    if (    ( EF_DIR_DELETED_MASK == u8EntryType )
         || ( '.' == u8EntryType )
//         || ((int)((u8Attrib & ~EF_DIR_ATTRIB_BIT_ARCHIVE) == EF_DIR_ATTRIB_BIT_VOLUME_ID) != vol ) )
         || (    (    ( 0 == vol )
                   && ( EF_DIR_ATTRIB_BIT_VOLUME_ID == ( ~EF_DIR_ATTRIB_BIT_ARCHIVE & u8Attrib ) ) )
              || (    ( 0 != vol )
                   && ( EF_DIR_ATTRIB_BIT_VOLUME_ID != ( ~EF_DIR_ATTRIB_BIT_ARCHIVE & u8Attrib ) ) ) ) )
    {
      ord = 0xFF;
    }
    /* Else, if An LFN entry is found */
    else if ( EF_DIR_ATTRIB_BITS_LFN == u8Attrib )
    {
      /* Is it start of an LFN sequence? */
      if ( EF_DIR_LFN_LAST & u8EntryType )
      {
        sum = pxDir->pu8Dir[EF_DIR_LFN_CHECKSUM];
        u8EntryType &= (ef_u08_t)~EF_DIR_LFN_LAST;
        ord = u8EntryType;
        pxDir->u32BlkOffset = pxDir->u32Offset;
      }
      ucs2_t *    pxLFNBuffer;            /* LFN working buffer */
      (void) EF_LFN_BUFFER_GET( pxDir->xObject.pxFS, pxLFNBuffer );
      /* Check LFN validity and capture it */
      if (    ( u8EntryType == ord )
           && ( sum == pxDir->pu8Dir[ EF_DIR_LFN_CHECKSUM ] )
           && ( EF_RET_OK == eEFPrvLFNPick( pxLFNBuffer, pxDir->pu8Dir ) ) )
      {
        ord = ord - 0x01;
      }
      else
      {
        ord = 0xFF;
      }
    }
    else
    {          /* An SFN entry is found */
      ef_u08_t u8CheckSum = 0;
      /* Is there a valid LFN? */
      if (    ( 0 != ord )
           || (    ( EF_RET_OK == eEFPrvSFNChecksumGet(  pxDir->pu8Dir,
                                                          &u8CheckSum ) )
                && ( sum != u8CheckSum ) ) )
      {
        /* It has no LFN. */
        pxDir->u32BlkOffset = 0xFFFFFFFF;
      }
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
    pxDir->xSector = 0;    /* Terminate the read operation on error or EOT */
  }
#endif /* ( 0 != EF_CONF_VFAT ) */

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
