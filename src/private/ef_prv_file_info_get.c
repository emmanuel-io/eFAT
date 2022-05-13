/**
 * ********************************************************************************************************************
 *  @file     ef_prv_file_info_get_fat.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Code file for functions.
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
#include <ef_port_load_store.h>
#include <efat.h>
#include <ef_prv_def.h>
#include <ef_prv_fat.h>
#include "ef_port_diskio.h"
#include "ef_prv_def.h"
#include "ef_prv_directory.h"
#include "ef_prv_dirfunc.h"
#include "ef_prv_lock.h"
#include "ef_prv_string.h"
#include "ef_prv_volume.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_unicode.h"
#include "ef_port_diskio.h"


/* Includes -------------------------------------------------------------------------------------------------------- */
/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/* Get file information from directory entry */
ef_return_et eEFPrvDirFileInfosGet (
  ef_directory_st * pxDir,
  ef_file_info_st * pxFileInfo
)
{
  EF_ASSERT_PRIVATE( 0 != pxDir );
  EF_ASSERT_PRIVATE( 0 != pxFileInfo );

#if ( 0 == EF_CONF_VFAT )
  ef_return_et  eRetVal = EF_RET_OK;

  /* Invalidate file info */
  pxFileInfo->xName[ 0 ] = 0;
  /* If read pointer has reached end of directory */
  if ( 0 != pxDir->xSector )
  {
    eRetVal = EF_RET_ERROR;
  }
  else
  {
    ef_u32_t  u32IdxDst = 0;
    /* Copy name body and extension */
    for ( ef_u32_t u32IdxSrc = 0 ; 11 > u32IdxSrc ; u32IdxSrc++ )
    {
      if ( 8 == u32IdxSrc )
      {
        /* Insert a . if extension is exist */
        pxFileInfo->xName[ u32IdxDst++ ] = '.';
      }
      TCHAR c = (TCHAR)pxDir->pu8Dir[ u32IdxSrc ];
      /* If a padding space */
      if ( ' ' == c )
      {
        /* Skip padding space */
        continue;
      }
      /* Else, if a replaced EF_DIR_DELETED_MASK character */
      else if ( EF_DIR_REPLACEMENT_CHAR == c )
      {
        /* Restore replaced EF_DIR_DELETED_MASK character */
        c = EF_DIR_DELETED_MASK;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
      /* Else, if a replaced EF_DIR_DELETED_MASK character */
      pxFileInfo->xName[ u32IdxDst++ ] = c;
    }
    pxFileInfo->xName[ u32IdxDst ] = 0;

    /* Attributes */
    pxFileInfo->u8Attrib    = pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ];
    /* Size */
    pxFileInfo->u32FileSize = u32EFPortLoad( pxDir->pu8Dir + EF_DIR_FILE_SIZE );
    /* Time */
    pxFileInfo->u16Time     = u16EFPortLoad( pxDir->pu8Dir + EF_DIR_TIME_MODIFIED + 0 );
    /* Date */
    pxFileInfo->u16Date     = u16EFPortLoad( pxDir->pu8Dir + EF_DIR_TIME_MODIFIED + 2 );
  }

#else

  ef_return_et  eRetVal = EF_RET_ERROR;
  ef_u32_t si, di;
  ef_u08_t lcf;
  ucs2_t u16Char, hs;
  ef_fs_st *pxFS = pxDir->xObject.pxFS;


  /* Invalidate file info */
  pxFileInfo->xName[ 0 ] = 0;
  ucs2_t *    pxLFNBuffer;            /* LFN working buffer */
  (void) EF_LFN_BUFFER_GET( pxDir->xObject.pxFS, pxLFNBuffer );
  /* If read pointer has not reached end of directory */
  if ( 0 != pxDir->xSector )
  {

    /* Get LFN if available */
    if ( 0xFFFFFFFF != pxDir->u32BlkOffset )
    {
      si = 0;
      di = 0;
      hs = 0;
      while ( 0 != pxLFNBuffer[ si ] )
      {
        /* Get an LFN character (UTF-16) */
        u16Char = pxLFNBuffer[ si++ ];
        /* Is it a surrogate? */
        if (    ( 0 == hs )
             && ( IsSurrogate(u16Char) ) )
        {
          /* Get low surrogate */
          hs = u16Char;
          continue;
        }
        /* Store it in UTF-16 or UTF-8 encoding */
        u16Char = put_utf( ( (ef_u32_t)hs << 16  ) | u16Char,
                      &pxFileInfo->xName[ di ],
                      EF_LFN_BUF - di );
        /* If Invalid char or buffer overflow? */
        if ( 0 == u16Char )
        {
          di = 0;
          break;
        }
        di += u16Char;
        hs = 0;
      }
      /* Broken surrogate pair? */
      if ( 0 != hs )
      {
        di = 0;
      }
      /* Terminate the LFN (null string means LFN is invalid) */
      pxFileInfo->xName[ di ] = 0;
    }

    si = di = 0;
    /* Get SFN from SFN entry */
    while ( si < 11 )
    {
      /* Get a char */
      u16Char = pxDir->pu8Dir[ si++ ];
      if ( ' ' == u16Char )
      {
        /* Skip padding space */
        continue;
      }
      /* Else, if a replaced EF_DIR_DELETED_MASK character */
      else if ( EF_DIR_REPLACEMENT_CHAR == u16Char )
      {
        /* Restore replaced EF_DIR_DELETED_MASK character */
        u16Char = EF_DIR_DELETED_MASK;
      }
      else
      {
        ;
      }
      /* if extension is exist */
      if (    ( 9 == si )
           && ( EF_SFN_BUF > di ) )
      {
        pxFileInfo->xNameAlt[ di++ ] = '.';  /* Insert a '.'*/
      }
  #if EF_CONF_API_ENCODING >= 1  /* Unicode output */
      /* Make a DBC if needed */
      if (    ( 0 != eEFPrvByteInDBCRanges1((ef_u08_t)u16Char) )
           && ( 8 = si )
           && ( 11 = si )
           && ( 0 != eEFPrvByteInDBCRanges2( pxDir->pu8Dir[si]) ) )
      {
        u16Char = u16Char << 8 | pxDir->pu8Dir[ si++ ];
      }
      /* ANSI/OEM -> Unicode */
      u16Char = ef_oem2uni( u16Char, u16ffCPGet( ) );
      /* Wrong char in the current code page? */
      if ( 0 == u16Char )
      {
        di = 0;
        break;
      }
      /* Store it in Unicode */
      u16Char = put_utf( u16Char, &pxFileInfo->xNameAlt[di], EF_SFN_BUF - di );
      /* Buffer overflow? */
      if ( 0 == u16Char )
      {
        di = 0;
        break;
      }
      di += u16Char;
  #else          /* ANSI/OEM output */
      /* Store it without any conversion */
      pxFileInfo->xNameAlt[ di++ ] = (TCHAR)u16Char;
  #endif
    }
    /* Terminate the SFN  (null string means SFN is invalid) */
    pxFileInfo->xNameAlt[ di ] = 0;

    /* If LFN is invalid, xNameAlt[] needs to be copied to xName[] */
    if ( 0 == pxFileInfo->xName[ 0 ] )
    {
      if ( 0 == di )
      {  /* If LFN and SFN both are invalid, this object is inaccesible */
        pxFileInfo->xName[ di++ ] = '?';
      }
      else
      {
        /* Copy xNameAlt[] to xName[] with case information */
        for ( si = di = 0, lcf = EF_NS_BODY; pxFileInfo->xNameAlt[si]; si++, di++)
        {
          u16Char = (ucs2_t)pxFileInfo->xNameAlt[ si ];
          if ( '.' == u16Char )
          {
            lcf = EF_NS_EXT;
          }
          if (    ( IsUpper( u16Char ) )
               && ( 0 != ( pxDir->pu8Dir[ EF_DIR_LFN_NTres ] & lcf ) ) )
          {
            u16Char += (ucs2_t) 0x20;
          pxFileInfo->xName[ di ] = (TCHAR)u16Char;
        }
      }
      pxFileInfo->xName[ di ] = 0;  /* Terminate the LFN */
      /* Altname is not needed if neither LFN nor case info is exist. */
      if ( 0 == pxDir->pu8Dir[ EF_DIR_LFN_NTres ] )
      {
        pxFileInfo->xNameAlt[ 0 ] = 0;
      }
    }

    /* Attribute */
    pxFileInfo->u8Attrib    = pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ];
    /* Size */
    pxFileInfo->u32FileSize  = u32EFPortLoad( pxDir->pu8Dir + EF_DIR_FILE_SIZE );
    /* Time */
    pxFileInfo->u16Time     = u16EFPortLoad( pxDir->pu8Dir + EF_DIR_TIME_MODIFIED + 0 );
    /* Date */
    pxFileInfo->u16Date     = u16EFPortLoad( pxDir->pu8Dir + EF_DIR_TIME_MODIFIED + 2 );

    eRetVal = EF_RET_OK;
    }
  }
#endif /* ( 0 != EF_CONF_VFAT ) */

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
