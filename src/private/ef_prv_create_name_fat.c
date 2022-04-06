/**
 * ********************************************************************************************************************
 *  @file     ef_prv_create_name_fat.c
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

/* Pick a top segment and create the object name in directory form */
ef_return_et eEFPrvNameCreateFAT (
  ef_directory_st * pxDir,
  const TCHAR**     ppxPath
)
{
  EF_ASSERT_PRIVATE( 0 != pxDir );
  EF_ASSERT_PRIVATE( 0 != ppxPath );

  ef_return_et  eRetVal  = EF_RET_INVALID_NAME;;
  ef_u32_t      si       = 0;

  /* Create file name in directory form */
  const char  * pxPath = *ppxPath;
  ef_u08_t    * sfn    = pxDir->u8Name;

  /* Fill the short file name with spaces */
  eEFPortMemSet( sfn, (ef_u08_t) ' ', 11 );

  /* If     we use relative path
   *    AND this a dot entry */
  if (    ( 0 != EF_CONF_RELATIVE_PATH )
       && ( '.' == pxPath[ si ] ) )
  { /* Relative path and dot entry */
    ef_u32_t  i  = 0;
    ef_u08_t  c;
#if 1
    for ( ; ; )
    {
      c = (ef_u08_t)pxPath[ si++ ];
      if (    ( '.' != c )
           || ( 3 <= si ) )
      {
        break;
      }
      sfn[ i++ ]  = c;
    } /* for ( ; ; ) */
#else
    for ( si = 0 ; 3 > si ; si++  )
    {
      c = (ef_u08_t)pxPath[ si ];
      if ( '.' != c )
      {
        si++;
        break;
      }
      sfn[ i++ ]  = c;
    } /* for ( ; ; ) */
#endif
    /* If     not forward slash separator
     *    AND not backward slash separator
     *    AND not an end of line character */
    if (    ( '/' != c )
         && ( '\\' != c )
         && ( ' ' < c ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    else
    {
      /* Return pointer to the next segment */
      *ppxPath = pxPath + si;
      /* If end of the path */
      if ( ' ' >= c )
      {
        /* Set last segment Status Flag */
        sfn[ EF_NSFLAG ] = EF_NS_LAST | EF_NS_DOT;
      }
      else
      {
        sfn[ EF_NSFLAG ] = EF_NS_DOT;
      }
      eRetVal = EF_RET_OK;
    }
  } /* Relative path and dot entry */
  else
  { /* NOT Relative path and dot entry */
    eRetVal = EF_RET_OK;

    ef_u32_t ni = 8;
    ef_u32_t i  = 0;
    ef_u08_t c;
    ef_u08_t d;

    for ( ; ; )
    {
      /* Get a byte */
      c = (ef_u08_t)pxPath[ si++ ];
      /* If end of the path name */
      if ( ' ' >= c )
      {
        /* Break */
        break;
      }
      /* Break if a separator is found */
      if (    ( '/' == c )
           || ( '\\' == c ) )
      {
        /* Skip duplicated separator if exist */
        while (    ( '/' == pxPath[ si ] )
                || ( '\\' == pxPath[ si ] ) )
        {
          si++;
        }
        break;
      }
      /* End of body or field overflow? */
      if (    ( '.' == c )
           || ( i >= ni ) )
      {
        /* Field overflow or invalid dot? */
        if (    ( 11 == ni )
             || ( '.' != c ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
          break;
        }
        /* Enter file extension field */
        i = 8;
        ni = 11;
        continue;
      }

      /* Is SBC extended character? => To upper SBC extended character */
//      c = u8ffToUpperExtendedCharacter( c );
      (void) eEFPrvu8ToUpperExtendedCharacter(c, &c);
      /* Check if it is a DBC 1st byte */
      if ( EF_RET_OK == eEFPrvCharInDBCRangesByte1( c ) )
      { /* DBC */
        /* Get 2nd byte */
        d = (ef_u08_t)pxPath[ si++ ];
        if (    ( EF_RET_OK != eEFPrvCharInDBCRangesByte2( d ) )
             || ( i >= ni - 1 ) )
        {
          /* Reject invalid DBC */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
          break;
        }
        sfn[ i++ ] = c;
        sfn[ i++ ] = d;
      } /* DBC */
      else
      { /* SBC */
        if ( EF_RET_OK == eEFPrvStringFindChar( "\"*+,:;<=>\?[]|\x7F", (char) c ) )
        {
          /* Reject illegal chrs for SFN */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
          break;
        }
        else
        {
          /* If c is lower case */
          if ( IsLower(c) )
          {
            /* Convert to upper case */
            c = c + (ef_u08_t) ((ef_u08_t) 'A' - (ef_u08_t) 'a'); //0x20;
          }
          /* Add Character to ShortFileName */
          sfn[ i++ ] = c;
        }
      } /* SBC */
    } /* for ( ; ; ) */

    if ( EF_RET_OK == eRetVal )
    {
      /* Return pointer to the next segment */
      *ppxPath = pxPath + si;
      /* Reject nul string */
      if ( 0 == i )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
      }
      else
      {
        /* If the first character collides with EF_DIR_DELETED_MASK */
        if ( EF_DIR_DELETED_MASK == sfn[ 0 ] )
        {
          /* Replace it with EF_DIR_REPLACEMENT_CHAR */
          sfn[ 0 ] = EF_DIR_REPLACEMENT_CHAR;
        }
        /* If end of the path */
        if ( ' ' >= c )
        {
          /* Set last segment Flags */
          sfn[ EF_NSFLAG ] = EF_NS_LAST;
        }
        else
        {
          sfn[ EF_NSFLAG ] = 0;
        }
        eRetVal = EF_RET_OK;
      }
    }
  } /* NOT Relative path and dot entry */

  return eRetVal;
}
/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
