/**
 * ********************************************************************************************************************
 *  @file     ef_prv_pattern_matching.c
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

#include <efat.h>
#include "ef_prv_string.h"
#include "ef_prv_unicode.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */

/**
 *  @brief  Get a character and advances ptr
 *
 *  @param  ppxString   Pointer to pointer to the ANSI/OEM or Unicode string
 *
 *  @return Operation result
 *  @retval EF_RET_OK       Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
static ef_u32_t u32StringCharGet (
  const TCHAR** ppxString
);

/* Local functions ------------------------------------------------------------------------------------------------- */

/* Pattern matching */
static ef_u32_t u32StringCharGet (
  const TCHAR** ppxString
)
{
  EF_ASSERT_PRIVATE( 0 != ppxString );

  ef_u32_t chr;

  if (    ( 0 != EF_CONF_VFAT )
       && ( EF_DEF_API_OEM != EF_CONF_API_ENCODING ) )
  { /* Unicode input */
    chr = u32xCharToUTF16( ppxString );
    if ( 0xFFFFFFFF == chr )
    {
      /* Wrong UTF encoding is recognized as end of the string */
      chr = 0;
    }
    chr = ef_wtoupper( chr );
  } /* Unicode input */
  else
  { /* ANSI/OEM input */
    /* Get a byte and increment string pointer */
    chr = (ef_u08_t)*(*ppxString)++;
    /* If character is a lower ASCII */
    if ( IsLower( chr ) )
    {
      /* To upper ASCII char */
      chr -= 0x20;
    }
    /* To upper SBCS extended char */
    chr = u32ffToUpperExtendedCharacter( chr );
    /* If character is in the range of DBCS first byte */
    if ( EF_RET_OK == eEFPrvCharInDBCRangesByte1( (ef_u08_t) chr ) )
    {
      /* If character is in the range of DBCS second byte */
      if ( EF_RET_OK == eEFPrvCharInDBCRangesByte2( (ef_u08_t) **ppxString ) )
      {
        /* Add DBC 2nd byte to output */
        chr = chr << 8 | (ef_u08_t)*(*ppxString)++;
      }
      else
      {
        /* Wrong encoding is recognized as end of the string */
        chr = 0;
      }
    }
  } /* ANSI/OEM input */

  return chr;
}

/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEFPrvPatternMatching (
  const TCHAR * pxPattern,
  const TCHAR * pxString,
  int           skip,
  int           inf
)
{
  EF_ASSERT_PRIVATE( 0 != pxPattern );
  EF_ASSERT_PRIVATE( 0 != pxString );

  ef_return_et  eRetVal = EF_RET_ERROR;

  /* Pre-skip name chars */
  while ( 0 != skip-- )
  {
    /* Branch mismatched if less name chars */
    if ( 0 == u32StringCharGet( &pxString ) )
    {
      /* Branch mismatched if less name chars */
      return EF_RET_ERROR;
    }
  }
  if ( ( 0 == *pxPattern ) && ( 0 !=inf ) )
  {
    return EF_RET_OK;  /* (short circuit) */
  }

  ef_u32_t pc;
  ef_u32_t nc;
  int nm;
  int nx;

  do
  {
    /* Top of pattern and name to match */
    const TCHAR * pp = pxPattern;
    /* Top of pattern and name to match */
    const TCHAR * np = pxString;
    for ( ; ; )
    {
      /* Wildcard? */
      if (    ( '?' == *pp )
           || ( '*' == *pp ) )
      {
        nm = 0;
        nx = 0;
        /* Analyze the wildcard block */
        do
        {
          if ( '?' == *pp++ )
          {
            nm++;
          }
          else
          {
            nx = 1;
          }
        } while ( ( '?' == *pp ) || ( '*' == *pp ) );
        if ( EF_RET_OK == eEFPrvPatternMatching( pp, np, nm, nx ) )
        {
          return EF_RET_OK;  /* Test new branch (recurs upto number of wildcard blocks in the pattern) */
        }
        nc = *np;
        break;  /* Branch mismatched */
      }
      /* Get a pattern char */
      pc = u32StringCharGet( &pp );
      /* Get a name char */
      nc = u32StringCharGet( &np );
      /* Branch mismatched? */
      if ( pc != nc )
      {
        break;
      }
      /* Branch matched? (matched at end of both strings) */
      if ( 0 == pc )
      {
        /* Branch matched? (matched at end of both strings) */
        return EF_RET_OK;
      }
    }
    /* pxString++ */
    (void) u32StringCharGet( &pxString );
  /* Retry until end of name if infinite search is specified */
  } while ( ( 0 != inf ) && ( 0 != nc ) );
  eRetVal = EF_RET_ERROR;

  return eRetVal;
}
/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
