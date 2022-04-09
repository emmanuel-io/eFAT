/**
 * ********************************************************************************************************************
 *  @file     ef_prv_create_name_vfat.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Code file for functions.
 *
 * ********************************************************************************************************************
 *  eFAT - embedded FAT Filesystem module
 * **************************************************************************************************************
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
#include "ef_prv_directory.h"
#include "ef_prv_dirfunc.h"
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

ef_return_et eEFPrvNameCreateVFAT (
  ef_directory_st * pxDir,
  const TCHAR**     ppxPath
)
{
  EF_ASSERT_PRIVATE( 0 != pxDir );
  EF_ASSERT_PRIVATE( 0 != ppxPath );

  ef_return_et eRetVal = EF_RET_OK;

#if ( 0 != EF_CONF_VFAT )
  ef_u08_t b;
  ef_u08_t cf;
  ucs2_t  u16Char;
  ef_u32_t    i;
  ef_u32_t    ni;
  /* Create LFN into LFN working buffer */
  const TCHAR * pxPath  = *ppxPath;
  ucs2_t      * lfn;
  ef_u32_t      di      = 0;

  (void) EF_LFN_BUFFER_GET( pxDir->xObject.pxFS, lfn );
  /*
   * Create LFN segment until a separator or end of line is found
   */
  for ( ; ; )
  {
    /* Get a character in UTF16 from current API encoding */
    ef_u32_t uc;
    (void) u32xCharToUnicodes( &pxPath, &uc );
    /* If code is invalid */
    if ( 0xFFFFFFFF == uc )
    {
      /* Invalid code or UTF decode error */
      eRetVal = EF_RET_INVALID_NAME;
      break;
    }
    /* Else if it is a high surrogate */
    else if ( 0x10000 <= uc )
    {
      /* Store high surrogate if needed */
      lfn[ di++ ] = (ucs2_t)( uc >> 16 );
    }
    else
    {
      u16Char = (ucs2_t)uc;
      /* If     forward slash separator
       *    OR  backward slash separator
       *    OR  an end of line character */
      if (    ( ' ' > u16Char )
           || ( '/' == u16Char )
           || ( '\\' == u16Char ) )
      {
        break;
      }
      /* If    name is too long
       *    OR     character is standard ASCII
       *       AND character is found in the list of illegal characters for LFN */
      if (    ( EF_LFN_UNITS_MAX <= di )
           || (    ( 0x80 > u16Char )
                && ( EF_RET_OK == eEFPrvStringFindChar( "\"*:<>\?|\x7F", (char) u16Char ) ) ) )
      {
        eRetVal = EF_RET_INVALID_NAME;
        break;
      }
      else
      {
        /* Store the Unicode character */
        lfn[ di++ ] = u16Char;
      }
    }
  } /* for ( ; ; ) */

  if ( EF_RET_OK == eRetVal )
  {
    /*
     * Set LFN segment status and return pointer to next segment
     */
    /* End of path? */
    if ( ' ' > u16Char )
    {
      /* Set last segment Status Flags */
      cf = EF_NS_LAST;
    }
    else
    {
      /* Next segment follows */
      cf = 0;
      /* Skip duplicated separators if exist */
      while (    ( '/' == *pxPath )
              || ('\\' == *pxPath ) )
      {
        pxPath++;
      }
    }
    /* Return pointer to the next segment */
    *ppxPath = pxPath;

    /* If relative path is supported
     *    AND     this segment is a single dot name
     *         OR this segment is a double dots name */
    if (    ( 0 != EF_CONF_RELATIVE_PATH )
         && (    (    ( 1 == di )
                   && ( '.' == lfn[ di - 1 ] ) )
              || (    ( 2 == di )
                   && ( '.' == lfn[ di - 1 ] )
                   && ( '.' == lfn[ di - 1 ] ) ) ) )
    {
      lfn[ di ] = 0;
      /* Create dot name for SFN entry */
      for ( i = 0 ; i < 11; i++ )
      {
        if ( i < di )
        {
          pxDir->u8Name[ i ] = '.';
        }
        else
        {
          pxDir->u8Name[ i ] = ' ';
        }
      }
      /* This is a dot entry */
      pxDir->u8Name[ i ] = cf | EF_NS_DOT;
      return EF_RET_OK;
    }

    /* Snip off trailing spaces and dots if exist */
    while ( di )
    {
      u16Char = lfn[ di - 1 ];
      if (    ( ' ' != u16Char )
           && ( '.' != u16Char ) )
      {
        break;
      }
      di--;
    }
    /* LFN is created into the working buffer */
    lfn[ di ] = 0;
    /* Reject null name */
    if ( 0 == di )
    {
      return EF_RET_INVALID_NAME;
    }

    ef_u32_t     si;
    /* Create SFN in directory form */
    /* Remove leading spaces */
    for ( si = 0 ; lfn[ si ] == ' ' ; si++ )
    {
      ;
    }
    /* Is there any leading space or dot? */
    if (    ( 0 < si )
         || ( '.' == lfn[ si ] ) )
    {
      cf |= EF_NS_LOSS | EF_NS_LFN;
    }
    /* Find last dot (di<=si: no extension) */
    while (    ( 0 < di )
            && ( '.' != lfn[di - 1] ) )
    {
      di--;
    }

    eEFPortMemSet( pxDir->u8Name, (ef_u08_t) ' ', 11 );

    i   = 0;
    b   = 0;
    ni  = 8;

    for ( ; ; )
    {
      /* Get an LFN character */
      u16Char = lfn[ si++ ];
      /* Break on end of the LFN */
      if ( 0 == u16Char )
      {
        break;
      }
      /* Remove embedded spaces and dots */
      if (    ( ' ' == u16Char )
           || (    ( '.' == u16Char )
                && ( si != di ) ) )
      {
        cf |= EF_NS_LOSS | EF_NS_LFN;
        continue;
      }

      /* End of field? */
      if (    ( i  >= ni )
           || ( si == di ) )
      {
        /* Name extension overflow? */
        if ( 11 == ni )
        {
          cf |= EF_NS_LOSS | EF_NS_LFN;
          break;
        }
        /* Name body overflow? */
        if ( si != di )
        {
          cf |= EF_NS_LOSS | EF_NS_LFN;
        }
        /* No name extension? */
        if ( si > di )
        {
          break;
        }
        /* Enter name extension */
        si  = di;
        i   = 8;
        ni  = 11;
        b <<= 2;
        continue;
      }
      /* Is this a non-ASCII character? */
      if ( 0x80 <= u16Char )
      {
        /* LFN entry needs to be created */
        cf |= EF_NS_LFN;
        /* Unicode ==> Upper convert ==> ANSI/OEM code */
        u16Char = u16ffUnicodeToUpperANSIOEM( u16Char );
      }

      /* Is this a DBC? */
      if ( u16Char >= 0x100 )
      { /* DBC */
        /* Field overflow? */
        if ( i >= ( ni - 1 ) )
        {
          cf |= EF_NS_LOSS | EF_NS_LFN;
          i   = ni;
          /* Next field */
          continue;
        }
        /* Put 1st byte */
        pxDir->u8Name[ i++ ] = (ef_u08_t)(u16Char >> 8);
      } /* DBC */
      else
      { /* SBC */
        /* Replace illegal characters for SFN if needed */
        if (    ( 0 == u16Char )
             || ( EF_RET_OK == eEFPrvStringFindChar( "+,;=[]", (char) u16Char ) ) )
        {
          /* Replace illegal characters for SFN if needed */
          u16Char = '_';
          /* Lossy conversion */
          cf |= EF_NS_LOSS | EF_NS_LFN;
        }
        /* ASCII upper case? */
        else if ( IsUpper(u16Char) )
        {
          b |= 0x02;
        }
        /* ASCII lower case? */
        else if ( IsLower(u16Char) )
        {
          b   |= 0x01;
          u16Char  -= 0x20;
        }
        else
        {
          ; /* Code compliance */
        }
      } /* SBC */
      pxDir->u8Name[ i++ ] = (ef_u08_t)u16Char;
    } /* for ( ; ; ) */

    /* If the first character collides with EF_DIR_DELETED_MASK, replace it with EF_DIR_REPLACEMENT_CHAR */
    if ( EF_DIR_DELETED_MASK == pxDir->u8Name[ 0 ] )
    {
      pxDir->u8Name[ 0 ] = EF_DIR_REPLACEMENT_CHAR;
    }

    /* If composite capitals */
    if ( 0x03 == ( 0x03 & b ) )
    {
      /* LFN entry needs to be created */
      cf |= EF_NS_LFN;
    }
    /* If no extension */
    if ( 8 == ni )
    {
      /* Shift capital flags */
      b <<= 2;
    }
    /* When LFN is in 8.3 format without extended character, NT flags are created */
    if ( 0 == ( EF_NS_LFN & cf ) )
    {
      if ( 0 != ( 0x01 & b ) )
      {
      /* NT u8StatusFlags (Extension has small capital letters only) */
        cf |= EF_NS_EXT;
      }
      if ( 0 != ( 0x04 & b ) )
      {
        /* NT u8StatusFlags (Body has small capital letters only) */
        cf |= EF_NS_BODY;
      }
    }

    /* SFN is created into pxDir->u8Name[] */
    pxDir->u8Name[ EF_NSFLAG ] = cf;

    eRetVal = EF_RET_OK;
  }
#endif /* ( 0 != EF_CONF_VFAT ) */

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
