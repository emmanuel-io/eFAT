/**
 * ********************************************************************************************************************
 *  @file     ef_label_set.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Set Volume Label
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
#include <ef_prv_volume_mount.h>
#include "ef_port_diskio.h"
#include "ef_prv_def.h"
#include "ef_prv_directory.h"
#include "ef_prv_dirfunc.h"
#include "ef_prv_dir_label.h"
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

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_label_set (
  const TCHAR * pxLabel
)
{
  EF_ASSERT_PUBLIC( 0 != pxLabel );

  ef_return_et      eRetVal;
  ef_directory_st   xDir;
  ef_fs_st        * pxFS;
  ef_u08_t         dirvn[ 22 ];
  ef_u32_t         di;
  ucs2_t            u16Char;
  static const char badchr[ ] = "+.,;=[]/\\\"*:<>\?|\x7F";

  /* Get logical drive */
  eRetVal = eEFPrvVolumeMountCheck( &pxLabel, &pxFS );
  if ( EF_RET_OK != eRetVal )
  {
    (void) eEFPrvFSUnlock( pxFS, eRetVal );
    return eRetVal;
  }


  (void) eEFPortMemSet( dirvn, (ef_u08_t) ' ', 11 );
  di = 0;
  while ( (ef_u32_t)*pxLabel >= ' ' )
  {  /* Create volume pxLabel */
    if ( EF_DEF_API_OEM != EF_CONF_API_ENCODING )
    { /* UNICODE input */
      ef_u32_t u32Char = u32xCharToUTF16(&pxLabel);
      //u16Char = (u32Char < 0x10000) ? ef_uni2oem( ef_wtoupper( u32Char ), u16ffCPGet( ) ) : 0;
      if ( u32Char < 0x10000 )
      {
        ef_uni2oem( ef_wtoupper( u32Char ), u16ffCPGet( ) );
      }
      else
      {
        u16Char = 0;
      }
    } /* UNICODE input */
    else
    { /* ANSI/OEM input */
      u16Char = (ef_u08_t)*pxLabel++;
      if ( EF_RET_OK == eEFPrvCharInDBCRangesByte1( (ef_u08_t) u16Char ) )
      {
        if ( EF_RET_OK == eEFPrvCharInDBCRangesByte2( (ef_u08_t)*pxLabel ) )
        {
          u16Char <<= 8;
          u16Char  |= (ef_u08_t) *pxLabel++;
        }
        else
        {
          u16Char = 0;
        }
        //u16Char = eEFPrvCharInDBCRangesByte2((ef_u08_t)*pxLabel) ? u16Char << 8 | (ef_u08_t)*pxLabel++ : 0;
      }
      if ( IsLower(u16Char) )
      {
        /* To upper ASCII characters */
        u16Char -= 0x20;
      }
      u16Char = u16ffToUpperExtendedCharacter( u16Char );  /* To upper extended characters (SBCS cfg) */
    } /* ANSI/OEM input */
    if (    ( 0 == u16Char )
         || ( EF_RET_OK == eEFPrvStringFindChar( badchr + 0, (char) u16Char ) )
         || ( di >= (ef_u32_t)((u16Char >= 0x100) ? 10 : 11) ) )
    {
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RET_INVALID_NAME;
      (void) eEFPrvFSUnlock( pxFS, eRetVal );
      return eRetVal;
    }
    if ( 0x100 <= u16Char )
    {
      dirvn[ di++ ] = (ef_u08_t)( u16Char >> 8 );
    }
    dirvn[ di++ ] = (ef_u08_t)u16Char;
  }
  if ( EF_DIR_DELETED_MASK == dirvn[ 0 ] )
  {
    /* Reject illegal name (heading EF_DIR_DELETED_MASK) */
    eRetVal = EF_RET_INVALID_NAME;
    (void) eEFPrvFSUnlock( pxFS, eRetVal );
    return eRetVal;
  }
  while ( ( 0 != di ) && ( ' ' == dirvn[ di - 1 ] ) )
  {
    /* Snip trailing spaces */
    di--;
  }

  /* Set volume pxLabel */
  xDir.xObject.pxFS = pxFS;
  /* Open root directory */
  xDir.xObject.u32ClstStart = 0;
  eRetVal = eEFPrvDirectoryIndexSet( &xDir, 0 );

  if ( EF_RET_OK == eRetVal )
  {
    /* Get volume Label entry */
    eRetVal = DIR_READ_LABEL( &xDir );
    /* If an entry was found */
    if ( EF_RET_OK == eRetVal )
    {
      {
        if ( 0 != di )
        {
          /* Change the volume pxLabel */
          eEFPortMemCopy( dirvn, xDir.pu8Dir, 11 );
        }
        else
        {
          /* Remove the volume pxLabel */
          xDir.pu8Dir[ EF_DIR_NAME_START ] = EF_DIR_DELETED_MASK;
        }
      }
      pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
      eRetVal = eEFPrvFSSync( pxFS );
    }
    /* Else, if No volume Label entry */
    else if ( EF_RET_NO_FILE == eRetVal)
    {
      eRetVal = EF_RET_OK;
      /* Create a volume Label entry */
      if ( 0 != di )
      {
        /* Allocate an entry */
        eRetVal = eEFPrvDirectoryAllocate( &xDir, 1 );
        if ( EF_RET_OK == eRetVal )
        {
          /* Clean the entry */
          (void) eEFPortMemZero( xDir.pu8Dir, EF_DIR_ENTRY_SIZE );
          /* Create volume Label entry */
          xDir.pu8Dir[ EF_DIR_ATTRIBUTES ] = EF_DIR_ATTRIB_BIT_VOLUME_ID;
          (void) eEFPortMemCopy( dirvn, xDir.pu8Dir, 11 );
          pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
          eRetVal = eEFPrvFSSync( pxFS );
        }
      }
    }
    /* Else, an error */
    else
    {
    }
  }

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

