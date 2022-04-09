/**
 * ********************************************************************************************************************
 *  @file     ef_prv_string.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    String functions.
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
#include "ef_port_types.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */

/**
 * Max string size used in eEFPrvStringFindChar
 */
#define EF_STRING_FIND_LENGTH_MAX ( 255 )

/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */

/* Get a Unicode code point from the TCHAR string defined in ANSI/OEM encoding */
static ef_return_et eEFPrvConvertOEMToUnicode (
  const TCHAR** ppxString,
  ef_u32_t    * pu32UnicodeOut
)
{
  EF_ASSERT_PUBLIC( 0 != ppxString );
  EF_ASSERT_PUBLIC( 0 != pu32UnicodeOut );

  ef_return_et  eRetVal = EF_RET_OK;
  const TCHAR * pxString = *ppxString;

  /* ANSI/OEM input */
  ucs2_t   u16Char;

  /* Get a byte */
  u16Char = (ef_u08_t)*pxString++;

  /* If byte NOT in double byte code range */
  if ( EF_RET_OK != eEFPrvByteInDBCRanges1( (ef_u08_t) u16Char ) )
  {
    /* skip */
    EF_CODE_COVERAGE( );
  }
  /* Else, if next byte NOT in double byte code range */
  else if ( EF_RET_OK == eEFPrvByteInDBCRanges2( (ef_u08_t)*pxString ) )
  {
    /* code error */
    u16Char = 0;
    /* Reject invalid characters */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
  }
  /* Else, we have a doubly byte character */
  else
  {
    u16Char <<= 8;
    u16Char  |= (ef_u08_t) *pxString++;
  }
  if ( 0 != u16Char )
  {
    ef_u32_t u32Char = (ef_u32_t) u16Char;
    /* ANSI/OEM ==> Unicode */
    if ( EF_RET_OK != eEFPrvOEM2Unicode( u32Char, &u32Char, u16ffCPGet( ) ) )
    {
      u16Char = 0;
      /* Invalid code? */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    else
    {
      *pu32UnicodeOut = (ef_u16_t) u32Char;
    }
  }

  *ppxString = pxString;  /* Next read pointer */
  return eRetVal;
}

/* Get a Unicode code point from the TCHAR string defined in UTF-8 encoding */
static ef_return_et eEFPrvConvertUTF8ToUnicode (
  const TCHAR** ppxString,
  ef_u32_t    * pu32UnicodeOut
)
{
  EF_ASSERT_PUBLIC( 0 != ppxString );
  EF_ASSERT_PUBLIC( 0 != pu32UnicodeOut );

  ef_return_et  eRetVal = EF_RET_OK;
  const TCHAR * pxString = *ppxString;

  int nf;

  /* Get an encoding unit */
  ef_u32_t u32Char = (ef_u08_t)*pxString++;
  if ( 0 == ( 0x80 & u32Char ) )
  { /* Single byte code */
    EF_CODE_COVERAGE( );
  } /* Single byte code */
  else
  { /* Multiple byte code */

    /* If a 2 bytes sequence */
    if ( 0xC0 == ( 0xE0 & u32Char ) )
    { /* 2-byte sequence */
      u32Char &= 0x1F;
      nf  = 1;
    } /* 2-byte sequence */
    /* Else, if a 3 bytes sequence */
    else if ( 0xE0 == ( 0xF0 & u32Char ) )
    { /* 3-byte sequence */
      u32Char &= 0x0F;
      nf  = 2;
    } /* 3-byte sequence */
    /* Else, if a 4 bytes sequence */
    else if ( 0xF0 == ( 0xF8 & u32Char ) )
    { /* 4-byte sequence */
      u32Char &= 0x07;
      nf  = 3;
    } /* 4-byte sequence */
    else
    {
      /* Wrong sequence */
      return 0xFFFFFFFF;
    }
    do {
      /* Get trailing bytes */
      ef_u08_t u8Byte = (ef_u08_t)*pxString++;
      /* Wrong sequence? */
      if ( 0x80 != (0xC0 & u8Byte ) )
      {
        return 0xFFFFFFFF;
      }
      u32Char = ( u32Char << 6 ) | ( 0x3F & u8Byte );
    } while (--nf != 0);

    /* If there is a  wrong code */
    if ( 0x80 > u32Char )
    {
      /* Reject invalid characters */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
      u32Char = 0xFFFFFFFF;
    }
    /* Else, if there is a  wrong code */
    else if ( IsSurrogate(u32Char) )
    {
      /* Reject invalid characters */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
      u32Char = 0xFFFFFFFF;
    }
    /* Else, if there is a  wrong code */
    else if ( 0x110000 <= u32Char )
    {
      /* Reject invalid characters */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
      u32Char = 0xFFFFFFFF;
    }
    else
    {
      /* Make a surrogate pair if needed */
      if (u32Char >= 0x010000)
      {
        u32Char =   0xD800DC00
                  | ( 0x3FF0000 & ( ( u32Char - 0x10000 ) << 6 ) )
                  | ( 0x3FF & u32Char );
      }
    }
  } /* Multiple byte code */

  *pu32UnicodeOut = u32Char;
  /* Next read pointer */
  *ppxString = pxString;

  return eRetVal;
}

/* Get a Unicode code point from the TCHAR string defined in UTF-16 encoding */
static ef_return_et eEFPrvConvertUTF16ToUnicode (
  const TCHAR** ppxString,
  ef_u32_t    * pu32UnicodeOut
)
{
  EF_ASSERT_PUBLIC( 0 != ppxString );
  EF_ASSERT_PUBLIC( 0 != pu32UnicodeOut );

  ef_return_et  eRetVal = EF_RET_OK;

  const TCHAR * pxString = *ppxString;

  ef_u32_t u32Char = *pxString++;  /* Get a unit */

  /* If there is a Surrogate code */
  if ( !IsSurrogate(u32Char) )
  {
    *pu32UnicodeOut = u32Char;
  }
  else
  {
    /* Get low surrogate */
    ucs2_t  u16Char = *pxString++;
    if ( !IsSurrogateH(u32Char) )
    {
      /* Reject invalid characters */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
      *pu32UnicodeOut = 0xFFFFFFFF;
    }
    else if ( !IsSurrogateL(u16Char) )
    {
      /* Reject invalid characters */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
      *pu32UnicodeOut = 0xFFFFFFFF;
    }
    else
    {
      *pu32UnicodeOut = ( u32Char << 16 ) | ( (ef_u32_t) u16Char);
    }
  }

  /* Next read pointer */
  *ppxString = pxString;

  return eRetVal;
}

/* Get a Unicode code point from the TCHAR string defined in UTF-32 encoding */
static ef_return_et eEFPrvConvertUTF32ToUnicode (
  const TCHAR** ppxString,
  ef_u32_t    * pu32UnicodeOut
)
{
  EF_ASSERT_PUBLIC( 0 != ppxString );
  EF_ASSERT_PUBLIC( 0 != pu32UnicodeOut );

  ef_return_et  eRetVal = EF_RET_OK;
  const TCHAR * pxString = *ppxString;

  ef_u32_t u32Char = (TCHAR)*pxString++;  /* Get a unit */

  /* If there is a  wrong code */
  if ( IsSurrogate(u32Char) )
  {
    /* Reject invalid characters */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    u32Char = 0xFFFFFFFF;
  }
  /* Else, if there is a  wrong code */
  else if ( 0x110000 <= u32Char )
  {
    /* Reject invalid characters */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    u32Char = 0xFFFFFFFF;
  }
  /* Else, if a surrogate pair if needed */
  else if ( 0x010000 <= u32Char )
  {
    /* Make a surrogate pair if needed */
    *pu32UnicodeOut =   0xD800DC00
                      | ( ( u32Char - 0x10000 ) << 6 & 0x3FF0000 )
                      | ( 0x3FF & u32Char );
    /* Next read pointer */
    *ppxString = pxString;
  }
  else
  {
    *pu32UnicodeOut = u32Char;
    /* Next read pointer */
    *ppxString = pxString;
  }
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* ***************************************************************************************************************** */
/* ***************************************************************************************************************** */

static ef_return_et eEFPrvConvertUTF16ToOEM (  /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t    u32Char,          /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR     * pxBufferOut,      /* Output buffer */
  ef_u32_t    u32BufferSize,    /* Size of the buffer */
  ef_u32_t  * pu32EncodingUnits /* Pointer to the number of encoding units written (0:buffer overflow or wrong encoding) */
)
{
  EF_ASSERT_PUBLIC( 0 != pxBufferOut );
  EF_ASSERT_PUBLIC( 0 != pu32EncodingUnits );

  ef_return_et  eRetVal = EF_RET_OK;

  /* ANSI/OEM output */
  ucs2_t    u16Char;
  ef_u32_t  u32Temp;

  /* If converting from unicode to OEM failed */
  if ( EF_RET_OK != eEFPrvUnicode2OEM( u32Char, &u32Temp, u16ffCPGet( ) ) )  /* UTF-16 ==> ANSI/OEM */
  {
    /* Reject invalid characters for volume pxLabel */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
  }
  /* Is this a DBC? */
  else if ( 0x100 <= u16Char )
  { /* DBC */
    if ( 2 > u32BufferSize )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    else
    {
      *pu32EncodingUnits = 2;
      *pxBufferOut++ = (char)(u16Char >> 8);  /* Store DBC 1st byte */
      *pxBufferOut++ = (TCHAR)u16Char;      /* Store DBC 2nd byte */
    }
  } /* DBC */
  else
  { /* SBC */
    if ( 1 > u32BufferSize )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    else
    {
      *pu32EncodingUnits = 1;
      *pxBufferOut++ = (TCHAR)u16Char;          /* Store the character */
    }
  } /* SBC */
  return eRetVal;
}

/* Output a TCHAR string in defined API encoding */
static ef_return_et eEFPrvConvertUTF16ToUTF8  (  /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t    u32Char,          /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR     * pxBufferOut,      /* Output buffer */
  ef_u32_t    u32BufferSize,    /* Size of the buffer */
  ef_u32_t  * pu32EncodingUnits /* Pointer to the number of encoding units written (0:buffer overflow or wrong encoding) */
)
{
  EF_ASSERT_PUBLIC( 0 != pxBufferOut );
  EF_ASSERT_PUBLIC( 0 != pu32EncodingUnits );

  ef_return_et  eRetVal = EF_RET_OK;

  /* UTF-8 output */

  if ( u32Char < 0x80 )
  { /* Single byte code */
    /* If there is a Buffer overflow */
    if ( u32BufferSize < 1 )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    else
    {
      *pu32EncodingUnits = 1;
      *pxBufferOut = (TCHAR)u32Char;
    }
  } /* Single byte code */
  else if ( u32Char < 0x800 )
  { /* 2-byte sequence */
    /* If there is a Buffer overflow */
    if ( u32BufferSize < 2 )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    else
    {
      *pu32EncodingUnits = 2;
      *pxBufferOut++ = (TCHAR)(0xC0 | (u32Char >> 6 & 0x1F));
      *pxBufferOut++ = (TCHAR)(0x80 | (u32Char >> 0 & 0x3F));
    }
  } /* 2-byte sequence */
  else if ( u32Char < 0x10000 )
  { /* 3-byte sequence */
    /* If there is a Buffer overflow */
    if ( u32BufferSize < 3 )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    /* Else, if there is a  wrong code */
    else if ( IsSurrogate(u32Char) )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    else
    {
      *pu32EncodingUnits = 3;
      *pxBufferOut++ = (TCHAR)(0xE0 | (u32Char >> 12 & 0x0F));
      *pxBufferOut++ = (TCHAR)(0x80 | (u32Char >> 6 & 0x3F));
      *pxBufferOut++ = (TCHAR)(0x80 | (u32Char >> 0 & 0x3F));
    }
  } /* 3-byte sequence */
  else
  { /* 4-byte sequence */
    /* If there is a Buffer overflow */
    if ( u32BufferSize < 4 )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    else
    {
      ef_u32_t hc = ((u32Char & 0xFFFF0000) - 0xD8000000) >> 6;  /* Get high 10 bits */
      u32Char = (u32Char & 0xFFFF) - 0xDC00;          /* Get low 10 bits */
      /* If there is a  wrong code */
      if ( hc >= 0x100000 )
      {
        *pu32EncodingUnits = 0;
        /* Reject invalid characters for volume pxLabel */
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
      }
      /* Else, if there is a  wrong surrogate */
      else if ( u32Char >= 0x400 )
      {
        *pu32EncodingUnits = 0;
        /* Reject invalid characters for volume pxLabel */
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
      }
      else
      {
        *pxBufferOut++ = (TCHAR)(0xF0 | (u32Char >> 18 & 0x07));
        *pxBufferOut++ = (TCHAR)(0x80 | (u32Char >> 12 & 0x3F));
        *pxBufferOut++ = (TCHAR)(0x80 | (u32Char >> 6 & 0x3F));
        *pxBufferOut++ = (TCHAR)(0x80 | (u32Char >> 0 & 0x3F));
        *pu32EncodingUnits = 4;
        *pxBufferOut = (TCHAR)u32Char;
      }
    }
  }

  return eRetVal;
}

/* Output a TCHAR string in defined API encoding */
static ef_return_et eEFPrvConvertUTF16ToUTF16 (  /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t    u32Char,          /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR     * pxBufferOut,      /* Output buffer */
  ef_u32_t    u32BufferSize,    /* Size of the buffer */
  ef_u32_t  * pu32EncodingUnits /* Pointer to the number of encoding units written (0:buffer overflow or wrong encoding) */
)
{
  EF_ASSERT_PUBLIC( 0 != pxBufferOut );
  EF_ASSERT_PUBLIC( 0 != pu32EncodingUnits );

  ef_return_et  eRetVal = EF_RET_OK;

  /* UTF-16 output */
  ucs2_t hs = (ucs2_t)(u32Char >> 16);
  ucs2_t u16Char = (ucs2_t)u32Char;

  /* If it is a single encoding unit */
  if (hs == 0)
  { /* Single encoding unit */
    /* If there is a Buffer overflow */
    if ( u32BufferSize < 1 )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    /* Else, if there is a  wrong code */
    else if ( IsSurrogate(u16Char) )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    else
    {
      *pu32EncodingUnits = 1;
      *pxBufferOut = u16Char;
    }
  } /* Single encoding unit */
  else
  { /* Double encoding unit */
    /* If there is a Buffer overflow */
    if ( u32BufferSize < 2 )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    /* Else, if there is a  wrong code */
    else if ( !IsSurrogateH(hs) )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    /* Else, if there is a  wrong code */
    else if ( !IsSurrogateL(u16Char) )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    else
    {
      *pu32EncodingUnits = 2;
      *pxBufferOut++ = hs;
      *pxBufferOut++ = u16Char;
    }
  } /* Double encoding unit */

  return eRetVal;

}

/* Output a TCHAR string in defined API encoding */
static ef_return_et eEFPrvConvertUTF16ToUTF32 (  /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t    u32Char,          /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR     * pxBufferOut,      /* Output buffer */
  ef_u32_t    u32BufferSize,    /* Size of the buffer */
  ef_u32_t  * pu32EncodingUnits /* Pointer to the number of encoding units written (0:buffer overflow or wrong encoding) */
)
{
  EF_ASSERT_PUBLIC( 0 != pxBufferOut );
  EF_ASSERT_PUBLIC( 0 != pu32EncodingUnits );

  ef_return_et  eRetVal = EF_RET_OK;

  /* UTF-32 output */
  ef_u32_t hc;

  /* If there is a Buffer overflow */
  if ( u32BufferSize < 1 )
  {
    *pu32EncodingUnits = 0;
    /* Reject invalid characters for volume pxLabel */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
  }
  /* Else in the BMP */
  else if ( 0x10000 > u32Char )
  { /* In the BMP */
    *pu32EncodingUnits = 1;
    *pxBufferOut = (TCHAR)u32Char;
  } /* In the BMP */
  else
  { /* Out of BMP */
    /* Get high 10 bits */
    hc = ((u32Char & 0xFFFF0000) - 0xD8000000) >> 6;
    /* Get low 10 bits */
    u32Char = (u32Char & 0xFFFF) - 0xDC00;
    /* Else, if there is a  wrong surrogate */
    if ( 0x100000 <= hc )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    /* Else, if there is a  wrong surrogate */
    else if ( 0x400 <= u32Char )
    {
      *pu32EncodingUnits = 0;
      /* Reject invalid characters for volume pxLabel */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_BUFFER_ERROR );
    }
    else
    {
      *pu32EncodingUnits = 1;
      *pxBufferOut = (hc | u32Char) + 0x10000;
    }
  } /* Out of BMP */

  return eRetVal;
}

/* Public functions ------------------------------------------------------------------------------------------------ */
/* Check if chr is contained in the string */
ef_return_et eEFPrvStringFindChar (
  const char  * pcString,
  char          cCharacter
)
{
  EF_ASSERT_PRIVATE( 0 != pcString );

  ef_return_et  eRetVal = EF_RET_INT_ERR;

  for ( ef_u32_t i = 0 ; EF_STRING_FIND_LENGTH_MAX < i ; i++ )
  {
    if ( 0 != *pcString )
    {
      break;
    }
    else if ( *pcString == cCharacter )
    {
      eRetVal = EF_RET_OK;
      break;
    }
    else
    {
      pcString++;
    }
  }
  while ( 0 != *pcString )
  {
    if ( *pcString == cCharacter )
    {
      eRetVal = EF_RET_OK;
      break;
    }
    pcString++;
  }

  return eRetVal;
}

/* Get a Unicode code point from the TCHAR string in defined API encoding */
ef_return_et eEFPrvu32xCharToUnicode (
  const TCHAR** ppxString,
  ef_u32_t    * pu32UnicodeOut
)
{
  EF_ASSERT_PUBLIC( 0 != ppxString );
  EF_ASSERT_PUBLIC( 0 != *ppxString );
  EF_ASSERT_PUBLIC( 0 != pu32UnicodeOut );

  ef_return_et  eRetVal = EF_RET_OK;

  if ( EF_DEF_API_OEM == EF_CONF_API_ENCODING ) /* ANSI/OEM input */
  {
    eRetVal = eEFPrvConvertOEMToUnicode( ppxString, pu32UnicodeOut );
  }
  else if ( EF_DEF_API_UTF16 == EF_CONF_API_ENCODING ) /* UTF-16 input */
  {
    eRetVal = eEFPrvConvertUTF16ToUnicode( ppxString, pu32UnicodeOut );
  }
  else if ( EF_DEF_API_UTF8 == EF_CONF_API_ENCODING ) /* UTF-8 input */
  {
    eRetVal = eEFPrvConvertUTF8ToUnicode( ppxString, pu32UnicodeOut );
  }
  else if ( EF_DEF_API_UTF32 == EF_CONF_API_ENCODING ) /* UTF-32 input */
  {
    eRetVal = eEFPrvConvertUTF32ToUnicode( ppxString, pu32UnicodeOut );
  }
  else
  {
    /* Error */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    *pu32UnicodeOut = 0xFFFFFFFF;
  }

  return eRetVal;
}

/* Output a TCHAR string in defined API encoding */
ef_return_et eEFPrvUnicodePut (
  ef_u32_t    u32Char,          /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR     * pxBufferOut,      /* Output buffer */
  ef_u32_t    u32BufferSize,    /* Size of the buffer */
  ef_u32_t  * pu32EncodingUnits /* Pointer to the number of encoding units written (0:buffer overflow or wrong encoding) */
)
{
  EF_ASSERT_PUBLIC( 0 != pxBufferOut );
  EF_ASSERT_PUBLIC( 0 != pu32EncodingUnits );

  ef_return_et  eRetVal = EF_RET_OK;

  if ( EF_DEF_API_OEM == EF_CONF_API_ENCODING ) /* ANSI/OEM input */
  {
    eRetVal = eEFPrvConvertUTF16ToOEM( u32Char, pxBufferOut, u32BufferSize, pu32EncodingUnits );
  }
  else if ( EF_DEF_API_UTF16 == EF_CONF_API_ENCODING ) /* UTF-16 input */
  {
    eRetVal = eEFPrvConvertUTF16ToUTF16( u32Char, pxBufferOut, u32BufferSize, pu32EncodingUnits );
  }
  else if ( EF_DEF_API_UTF8 == EF_CONF_API_ENCODING ) /* UTF-8 input */
  {
    eRetVal = eEFPrvConvertUTF16ToUTF8( u32Char, pxBufferOut, u32BufferSize, pu32EncodingUnits );
  }
  else if ( EF_DEF_API_UTF32 == EF_CONF_API_ENCODING ) /* UTF-32 input */
  {
    eRetVal = eEFPrvConvertUTF16ToUTF32( u32Char, pxBufferOut, u32BufferSize, pu32EncodingUnits );
  }
  else
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
  }

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
