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
static const char badchr[ ] = "+.,;=[]/\\\"*:<>\?|\x7F";
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_label_set (
  const TCHAR * pxLabel
)
{
  EF_ASSERT_PUBLIC( 0 != pxLabel );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st    * pxFS;
  ef_u08_t      u8Buffer[ 22 ];

  /* Get logical drive */
  if ( EF_RET_OK != eEFPrvVolumeMountCheck( &pxLabel, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else if ( EF_RET_OK != eEFPortMemSet( u8Buffer, (ef_u08_t) ' ', 11 ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    ef_u32_t  u32Index = 0;
    ucs2_t    u16Char;

    if ( EF_DEF_API_OEM != EF_CONF_API_ENCODING )
    { /* UNICODE input */
      while ( (ef_u32_t)*pxLabel >= ' ' )
      {  /* Create volume pxLabel */
        ef_u32_t u32Char;
        eEFPrvu32xCharToUnicode( &pxLabel, &u32Char );
        if ( u32Char >= 0x10000 )
        {
          /* Reject invalid characters for volume label */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
          break;
        }
        else if ( EF_RET_OK != eEFPrvUnicodeToUpper( u32Char, &u32Char ) )
        {
          /* Reject invalid characters for volume label */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
          break;
        }
        else if ( EF_RET_OK != eEFPrvUnicode2OEM( u32Char, &u32Char, u16ffCPGet( ) ) )  /* UTF-16 ==> ANSI/OEM */
        {
          /* Reject invalid characters for volume label */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
          break;
        }
        else if ( EF_RET_OK == eEFPrvStringFindChar( badchr + 0, (char) u32Char ) )
        {
          /* Reject invalid characters for volume pxLabel */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
          break;
        }
        else if ( u32Index >= (ef_u32_t)((u32Char >= 0x100) ? 10 : 11) )
        {
          /* Reject invalid characters for volume pxLabel */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
          break;
        }
        else if ( 0x100 <= u16Char )
        {
          u8Buffer[ u32Index++ ] = (ef_u08_t)( u16Char >> 8 );
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
        u8Buffer[ u32Index++ ] = (ef_u08_t)u16Char;
      } /*while */
    } /* UNICODE input */
    else
    { /* ANSI/OEM input */
      while ( (ef_u32_t)*pxLabel >= ' ' )
      {  /* Create volume pxLabel */
        u16Char = (ef_u08_t)*pxLabel++;
        /* If byte NOT in double byte code range */
        if ( EF_RET_OK != eEFPrvByteInDBCRanges1( (ef_u08_t) u16Char ) )
        {
          /* skip */
          EF_CODE_COVERAGE( );
        }
        /* Else, if next byte NOT in double byte code range */
        else if ( EF_RET_OK == eEFPrvByteInDBCRanges2( (ef_u08_t)*pxLabel ) )
        {
          /* code error */
          u16Char = 0;
        }
        /* Else, we have a doubly byte character */
        else
        {
          u16Char <<= 8;
          u16Char  |= (ef_u08_t) *pxLabel++;
        }

        if ( IsLower(u16Char) )
        {
          /* To upper ASCII characters */
          u16Char -= 0x20;
        }
        /* To upper extended characters */
        else if ( EF_RET_OK != eEFPrvu16ToUpperExtendedCharacter( u16Char, &u16Char ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
        }
        else if ( 0 == u16Char )
        {
          /* Reject invalid characters for volume label */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
          (void) eEFPrvFSUnlock( pxFS, eRetVal );
          return eRetVal;
        }
        else if ( EF_RET_OK == eEFPrvStringFindChar( badchr + 0, (char) u16Char ) )
        {
          /* Reject invalid characters for volume label */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
          (void) eEFPrvFSUnlock( pxFS, eRetVal );
          return eRetVal;
        }
        else if ( u32Index >= (ef_u32_t)((u16Char >= 0x100) ? 10 : 11) )
        {
          /* Reject invalid characters for volume pxLabel */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
          (void) eEFPrvFSUnlock( pxFS, eRetVal );
          return eRetVal;
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
        if ( 0x100 <= u16Char )
        {
          u8Buffer[ u32Index++ ] = (ef_u08_t)( u16Char >> 8 );
        }
        u8Buffer[ u32Index++ ] = (ef_u08_t)u16Char;
      } /*while */
    } /* ANSI/OEM input */

    if ( EF_RET_OK != eRetVal )
    {
      EF_CODE_COVERAGE( );
    }
    else if ( EF_DIR_DELETED_MASK == u8Buffer[ 0 ] )
    {
      /* Reject illegal name (heading EF_DIR_DELETED_MASK) */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    else
    {
      while ( ( 0 != u32Index ) && ( ' ' == u8Buffer[ u32Index - 1 ] ) )
      {
        /* Snip trailing spaces */
        u32Index--;
      }

      ef_directory_st   xDir;
      /* Set volume pxLabel */
      xDir.xObject.pxFS = pxFS;
      /* Open root directory */
      xDir.xObject.u32ClstStart = 0;

      if ( EF_RET_OK != eEFPrvDirectoryIndexSet( &xDir, 0 ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      else
      {
        /* Get volume Label entry */
        eRetVal = DIR_READ_LABEL( &xDir );
        /* If an entry was found */
        if ( EF_RET_OK == eRetVal )
        {
          if ( 0 != u32Index )
          {
            /* Change the volume pxLabel */
            eEFPortMemCopy( u8Buffer, xDir.pu8Dir, 11 );
          }
          else
          {
            /* Remove the volume pxLabel */
            xDir.pu8Dir[ EF_DIR_NAME_START ] = EF_DIR_DELETED_MASK;
          }
          pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
          if ( EF_RET_OK != eEFPrvFSSync( pxFS ) )
          {
            eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
          }
          else
          {
            EF_CODE_COVERAGE( );
          }
        }
        /* Else, if No volume Label entry */
        else if ( EF_RET_NO_FILE == eRetVal)
        {
          eRetVal = EF_RET_OK;
          /* Create a volume Label entry */
          if ( 0 == u32Index )
          {
            EF_CODE_COVERAGE( );
          }
          /* Else, if allocate an entry failed */
          else if ( EF_RET_OK !=  eEFPrvDirectoryAllocate( &xDir, 1 ) )
          {
            eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
          }
          /* Else, if the entry cleaning failed */
          else if ( EF_RET_OK !=  eEFPortMemZero( xDir.pu8Dir, EF_DIR_ENTRY_SIZE ) )
          {
            eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
          }
          else
          {
            /* Create volume Label entry */
            xDir.pu8Dir[ EF_DIR_ATTRIBUTES ] = EF_DIR_ATTRIB_BIT_VOLUME_ID;
            (void) eEFPortMemCopy( u8Buffer, xDir.pu8Dir, 11 );
            pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
            if ( EF_RET_OK != eEFPrvFSSync( pxFS ) )
            {
              eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
            }
            else
            {
              EF_CODE_COVERAGE( );
            }
          }
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
      }
    }
  }

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}
/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

