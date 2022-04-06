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

/* Get a Unicode code point from the TCHAR string defined in ANSI/OEM encoding */
ef_u32_t u32OEMToUTF16 (
  const TCHAR** ppxString
)
{
  ef_u32_t     uc = 0xFFFFFFFF;
  const TCHAR * p = *ppxString;

  /* ANSI/OEM input */
  ef_u08_t b;
  ucs2_t   u16Char;

  /* Get a byte */
  u16Char = (ef_u08_t)*p++;
  /* Is it a DBC 1st byte? */
  if ( EF_RET_OK == eEFPrvCharInDBCRangesByte1( (ef_u08_t)u16Char ) )
  {
    /* Get 2nd byte */
    b = (ef_u08_t)*p++;
    if ( EF_RET_OK != eEFPrvCharInDBCRangesByte2( b ) )
    {
      /* Invalid code? */
      return 0xFFFFFFFF;
    }
    /* Make a DBC */
    u16Char = (u16Char << 8) + ((ucs2_t)b);
  }
  if ( 0 != u16Char )
  {
    /* ANSI/OEM ==> Unicode */
    u16Char = ef_oem2uni( u16Char, u16ffCPGet( ) );
    if ( 0 == u16Char )
    {
      /* Invalid code? */
      return 0xFFFFFFFF;
    }
  }
  uc = u16Char;

  *ppxString = p;  /* Next read pointer */
  return uc;
}

/* Get a Unicode code point from the TCHAR string defined in UTF-16 encoding */
ef_u32_t u32UTF16ToUTF16 (
  const TCHAR** ppxString
)
{
  ef_u32_t      uc = 0xFFFFFFFF;
  const TCHAR * p = *ppxString;

  ucs2_t u16Char;

  uc = *p++;  /* Get a unit */

  if ( IsSurrogate(uc) )  /* Surrogate? */
  {
    u16Char = *p++;    /* Get low surrogate */
    if ( !IsSurrogateH(uc) || !IsSurrogateL(u16Char) )
    return 0xFFFFFFFF;  /* Wrong surrogate? */
    uc = uc << 16 | u16Char;
  }

  *ppxString = p;  /* Next read pointer */
  return uc;
}

/* Get a Unicode code point from the TCHAR string defined in UTF-8 encoding */
ef_u32_t u32UTF8ToUTF16 (
  const TCHAR** ppxString
)
{
  ef_u32_t      uc = 0xFFFFFFFF;
  const TCHAR * p = *ppxString;

  ef_u08_t b;
  int nf;

  /* Get an encoding unit */
  uc = (ef_u08_t)*p++;
  /* Multiple byte code? */
  if ( 0 != ( 0x80 & uc ) )
  {
    /* 2-byte sequence? */
    if ( 0xC0 == ( 0xE0 & uc ) )
    {
      uc &= 0x1F;
      nf  = 1;
    }
    else
    {
      /* 3-byte sequence? */
      if ( 0xE0 == ( 0xF0 & uc ) )
      {
        uc &= 0x0F;
        nf  = 2;
      }
      else
      {
        /* 4-byte sequence? */
        if ( 0xF0 == ( 0xF8 & uc ) )
        {
          uc &= 0x07;
          nf  = 3;
        }
        else
        {
          /* Wrong sequence */
          return 0xFFFFFFFF;
        }
      }
    }
    do {
    /* Get trailing bytes */
      b = (ef_u08_t)*p++;
      /* Wrong sequence? */
      if ( 0x80 != (0xC0 & b ) )
      {
        return 0xFFFFFFFF;
      }
      uc = uc << 6 | (b & 0x3F);
    } while (--nf != 0);
    /* Wrong code? */
  if (uc < 0x80 || IsSurrogate(uc) || uc >= 0x110000)
    return 0xFFFFFFFF;
    /* Make a surrogate pair if needed */
  if (uc >= 0x010000)
    uc = 0xD800DC00 | ((uc - 0x10000) << 6 & 0x3FF0000) | (uc & 0x3FF);
  }

  *ppxString = p;  /* Next read pointer */
  return uc;
}

/* Get a Unicode code point from the TCHAR string defined in UTF-32 encoding */
ef_u32_t u32UTF32ToUTF16 (
  const TCHAR** ppxString
)
{
  ef_u32_t      uc = 0xFFFFFFFF;
  const TCHAR * p = *ppxString;

  uc = (TCHAR)*p++;  /* Get a unit */
  /* Wrong code? */
  if (    IsSurrogate(uc)
       || ( 0x110000 <= uc ) )
  {
    uc = 0xFFFFFFFF;
  }
  else
  {
    if ( uc >= 0x010000 )
    {
      /* Make a surrogate pair if needed */
      uc =   0xD800DC00
           | ( ( uc - 0x10000 ) << 6 & 0x3FF0000 )
           | ( uc & 0x3FF );
    }
    /* Next read pointer */
    *ppxString = p;
  }
  return uc;
}


/* Output a TCHAR string in defined API encoding */
ef_u08_t u8UTF16ToOEM (  /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t  chr,  /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR   * buf,  /* Output buffer */
  ef_u32_t  szb  /* Size of the buffer */
)
{
  ef_u08_t u8RetVal = 0;
  /* ANSI/OEM output */
  ucs2_t u16Char;

  u16Char = ef_uni2oem( chr, u16ffCPGet( ) );
  /* Is this a DBC? */
  if ( 0x100 <= u16Char )
  {
    if ( 2 > szb )
      return 0;
    *buf++ = (char)(u16Char >> 8);  /* Store DBC 1st byte */
    *buf++ = (TCHAR)u16Char;      /* Store DBC 2nd byte */
    return 2;
  }
  if ( (0 == u16Char ) || ( 1 > szb ) )
    return 0;  /* Invalid char or buffer overflow? */
  *buf++ = (TCHAR)u16Char;          /* Store the character */
  return 1;
}

/* Output a TCHAR string in defined API encoding */
ef_u08_t u8UTF16ToUTF16 (  /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t  chr,  /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR   * buf,  /* Output buffer */
  ef_u32_t  szb  /* Size of the buffer */
)
{
  ef_u08_t u8RetVal = 0;
  /* UTF-16 output */
  ucs2_t hs;
  ucs2_t u16Char;

  hs = (ucs2_t)(chr >> 16);
  u16Char = (ucs2_t)chr;
  if (hs == 0) {  /* Single encoding unit? */
    if (szb < 1 || IsSurrogate(u16Char)) return 0;  /* Buffer overflow or wrong code? */
    *buf = u16Char;
    return 1;
  }
  if (szb < 2 || !IsSurrogateH(hs) || !IsSurrogateL(u16Char)) return 0;  /* Buffer overflow or wrong surrogate? */
  *buf++ = hs;
  *buf++ = u16Char;
  return 2;

}

/* Output a TCHAR string in defined API encoding */
ef_u08_t u8UTF16ToUTF8 (  /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t  chr,  /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR   * buf,  /* Output buffer */
  ef_u32_t  szb  /* Size of the buffer */
)
{
ef_u08_t u8RetVal = 0;

/* UTF-8 output */
  ef_u32_t hc;

  if (chr < 0x80) {  /* Single byte code? */
    if (szb < 1) return 0;  /* Buffer overflow? */
    *buf = (TCHAR)chr;
    return 1;
  }
  if (chr < 0x800) {  /* 2-byte sequence? */
    if (szb < 2) return 0;  /* Buffer overflow? */
    *buf++ = (TCHAR)(0xC0 | (chr >> 6 & 0x1F));
    *buf++ = (TCHAR)(0x80 | (chr >> 0 & 0x3F));
    return 2;
  }
  if (chr < 0x10000) {  /* 3-byte sequence? */
    if (szb < 3 || IsSurrogate(chr)) return 0;  /* Buffer overflow or wrong code? */
    *buf++ = (TCHAR)(0xE0 | (chr >> 12 & 0x0F));
    *buf++ = (TCHAR)(0x80 | (chr >> 6 & 0x3F));
    *buf++ = (TCHAR)(0x80 | (chr >> 0 & 0x3F));
    return 3;
  }
  /* 4-byte sequence */
  if (szb < 4) return 0;  /* Buffer overflow? */
  hc = ((chr & 0xFFFF0000) - 0xD8000000) >> 6;  /* Get high 10 bits */
  chr = (chr & 0xFFFF) - 0xDC00;          /* Get low 10 bits */
  if (hc >= 0x100000 || chr >= 0x400) return 0;  /* Wrong surrogate? */
  chr = (hc | chr) + 0x10000;
  *buf++ = (TCHAR)(0xF0 | (chr >> 18 & 0x07));
  *buf++ = (TCHAR)(0x80 | (chr >> 12 & 0x3F));
  *buf++ = (TCHAR)(0x80 | (chr >> 6 & 0x3F));
  *buf++ = (TCHAR)(0x80 | (chr >> 0 & 0x3F));
  return 4;
}

/* Output a TCHAR string in defined API encoding */
ef_u08_t u8UTF16ToUTF32 (  /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t  chr,  /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR   * buf,  /* Output buffer */
  ef_u32_t  szb  /* Size of the buffer */
)
{
ef_u08_t u8RetVal = 0;
/* UTF-32 output */
  ef_u32_t hc;

  /* If Buffer overflow */
  if (szb < 1)
    return 0;
  /* Out of BMP? */
  if (chr >= 0x10000)
  {
    /* Get high 10 bits */
    hc = ((chr & 0xFFFF0000) - 0xD8000000) >> 6;
    /* Get low 10 bits */
    chr = (chr & 0xFFFF) - 0xDC00;
    /* If Wrong surrogate */
    if (    ( 0x100000 <= hc )
         || ( 0x400 <= chr ) )
    {
      return 0;
    }
    else
    {
      chr = (hc | chr) + 0x10000;
    }
  }
  *buf++ = (TCHAR)chr;
  return 1;
}

/* Get a Unicode code point from the TCHAR string in defined API encoding */
ef_u32_t u32xCharToUTF16 (
  const TCHAR** ppxString
)
{
  ef_u32_t  u32Unicode = 0xFFFFFFFF;

  if ( EF_DEF_API_OEM == EF_CONF_API_ENCODING ) /* ANSI/OEM input */
  {
    u32Unicode = u32OEMToUTF16( ppxString );
  }
  else if ( EF_DEF_API_UTF16 == EF_CONF_API_ENCODING ) /* UTF-16 input */
  {
    u32Unicode = u32UTF16ToUTF16( ppxString );
  }
  else if ( EF_DEF_API_UTF8 == EF_CONF_API_ENCODING ) /* UTF-8 input */
  {
    u32Unicode = u32UTF8ToUTF16( ppxString );
  }
  else if ( EF_DEF_API_UTF32 == EF_CONF_API_ENCODING ) /* UTF-32 input */
  {
    u32Unicode = u32UTF32ToUTF16( ppxString );
  }
  else
  {
    u32Unicode = 0xFFFFFFFF; /* Error */
  }

  return u32Unicode;
}

/* Output a TCHAR string in defined API encoding */
ef_u08_t put_utf ( /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t  chr,  /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR *   buf,  /* Output buffer */
  ef_u32_t  szb   /* Size of the buffer */
)
{
  ef_u08_t u8RetVal = 0;

  if ( EF_DEF_API_OEM == EF_CONF_API_ENCODING ) /* ANSI/OEM input */
  {
    u8RetVal = u8UTF16ToOEM( chr, buf, szb );
  }
  else if ( EF_DEF_API_UTF16 == EF_CONF_API_ENCODING ) /* UTF-16 input */
  {
    u8RetVal = u8UTF16ToUTF16( chr, buf, szb );
  }
  else if ( EF_DEF_API_UTF8 == EF_CONF_API_ENCODING ) /* UTF-8 input */
  {
    u8RetVal = u8UTF16ToUTF8( chr, buf, szb );
  }
  else if ( EF_DEF_API_UTF32 == EF_CONF_API_ENCODING ) /* UTF-32 input */
  {
    u8RetVal = u8UTF16ToUTF32( chr, buf, szb );
  }
  else
  {
    u8RetVal = 0; /* Error */
  }

  return u8RetVal;
}

/* ***************************************************************************************************************** */
/* ***************************************************************************************************************** */
/* ***************************************************************************************************************** */

/* Get a Unicode code point from the TCHAR string defined in ANSI/OEM encoding */
ef_return_et eEFPrvOEMToUTF16 (
  const TCHAR** ppxString,
  ucs2_t      * pu16Char
)
{
  EF_ASSERT_PUBLIC( 0 != ppxString );
  EF_ASSERT_PUBLIC( 0 != pu16Char );

  ef_return_et  eRetVal = EF_RET_OK;
  const TCHAR * pxString = *ppxString;

  /* ANSI/OEM input */
  ucs2_t   u16Char;

  /* Get a byte */
  u16Char = (ef_u08_t)*pxString++;
  /* If byte NOT in double byte code range */
  if ( EF_RET_OK != eEFPrvCharInDBCRangesByte1( (ef_u08_t) u16Char ) )
  {
    /* skip */
    EF_CODE_COVERAGE( );
  }
  /* Else, if next byte NOT in double byte code range */
  else if ( EF_RET_OK == eEFPrvCharInDBCRangesByte2( (ef_u08_t)*pxString ) )
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
    /* ANSI/OEM ==> Unicode */
//    u16Char = ef_oem2uni( u16Char, u16ffCPGet( ) );
    /* OEM => Unicode conversions */
    /* If byte NOT in double byte code range */
//    if ( EF_RET_OK != eEFPrvOEM2Unicode( (ef_u08_t) u16Char ) )
    ef_u32_t u32Char = (ef_u32_t) u16Char;
    if ( EF_RET_OK != eEFPrvOEM2Unicode( u32Char, &u32Char, u16ffCPGet( ) ) )  /* UTF-16 ==> ANSI/OEM */
    {
      u16Char = 0;
      /* Invalid code? */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
    }
    else
    {
      *pu16Char = (ef_u16_t) u32Char;
    }
  }

  *ppxString = pxString;  /* Next read pointer */
  return eRetVal;
}

/* Get a Unicode code point from the TCHAR string defined in UTF-16 encoding */
ef_return_et eEFPrvUTF16ToUTF16 (
  const TCHAR** ppxString,
  ucs2_t      * pu16Char
)
{
  EF_ASSERT_PUBLIC( 0 != ppxString );
  EF_ASSERT_PUBLIC( 0 != pu16Char );

  ef_return_et  eRetVal = EF_RET_OK;
  const TCHAR * pxString = *ppxString;

  ucs2_t u16Char = *pxString++;  /* Get a unit */

  if ( IsSurrogate(u16Char) )  /* Surrogate? */
  {
    u16Char = *pxString++;    /* Get low surrogate */
    if ( !IsSurrogateH(u16Char) || !IsSurrogateL(u16Char) )
    return 0xFFFFFFFF;  /* Wrong surrogate? */
    u16Char = u16Char << 16 | u16Char;
  }

  *ppxString = pxString;  /* Next read pointer */
  return u16Char;
}

/* Get a Unicode code point from the TCHAR string defined in UTF-8 encoding */
ef_return_et eEFPrvUTF8ToUTF16 (
  const TCHAR** ppxString,
  ucs2_t      * pu16Char
)
{
  EF_ASSERT_PUBLIC( 0 != ppxString );
  EF_ASSERT_PUBLIC( 0 != pu16Char );

  ef_return_et  eRetVal = EF_RET_OK;
  const TCHAR * pxString = *ppxString;

  ef_u08_t b;
  int nf;

  /* Get an encoding unit */
  ef_u32_t uc = (ef_u08_t)*pxString++;
  /* Multiple byte code? */
  if ( 0 != ( 0x80 & uc ) )
  {
    /* 2-byte sequence? */
    if ( 0xC0 == ( 0xE0 & uc ) )
    {
      uc &= 0x1F;
      nf  = 1;
    }
    else
    {
      /* 3-byte sequence? */
      if ( 0xE0 == ( 0xF0 & uc ) )
      {
        uc &= 0x0F;
        nf  = 2;
      }
      else
      {
        /* 4-byte sequence? */
        if ( 0xF0 == ( 0xF8 & uc ) )
        {
          uc &= 0x07;
          nf  = 3;
        }
        else
        {
          /* Wrong sequence */
          return 0xFFFFFFFF;
        }
      }
    }
    do {
    /* Get trailing bytes */
      b = (ef_u08_t)*pxString++;
      /* Wrong sequence? */
      if ( 0x80 != (0xC0 & b ) )
      {
        return 0xFFFFFFFF;
      }
      uc = uc << 6 | (b & 0x3F);
    } while (--nf != 0);
    /* Wrong code? */
  if (uc < 0x80 || IsSurrogate(uc) || uc >= 0x110000)
    return 0xFFFFFFFF;
    /* Make a surrogate pair if needed */
  if (uc >= 0x010000)
    uc = 0xD800DC00 | ((uc - 0x10000) << 6 & 0x3FF0000) | (uc & 0x3FF);
  }

  *ppxString = pxString;  /* Next read pointer */
  return uc;
}

/* Get a Unicode code point from the TCHAR string defined in UTF-32 encoding */
ef_return_et eEFPrvUTF32ToUTF16 (
  const TCHAR** ppxString,
  ucs2_t      * pu16Char
)
{
  EF_ASSERT_PUBLIC( 0 != ppxString );
  EF_ASSERT_PUBLIC( 0 != pu16Char );

  ef_return_et  eRetVal = EF_RET_OK;
  const TCHAR * pxString = *ppxString;

  ef_u32_t uc = (TCHAR)*pxString++;  /* Get a unit */
  /* Wrong code? */
  if (    IsSurrogate(uc)
       || ( 0x110000 <= uc ) )
  {
    uc = 0xFFFFFFFF;
  }
  else
  {
    if ( uc >= 0x010000 )
    {
      /* Make a surrogate pair if needed */
      uc =   0xD800DC00
           | ( ( uc - 0x10000 ) << 6 & 0x3FF0000 )
           | ( uc & 0x3FF );
    }
    /* Next read pointer */
    *ppxString = pxString;
  }
  return uc;
}


ef_return_et eEFPrvu8UTF16ToOEM (  /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t  chr,  /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR   * buf,  /* Output buffer */
  ef_u32_t  szb  /* Size of the buffer */
)
{
  ef_u08_t u8RetVal = 0;
  /* ANSI/OEM output */
  ucs2_t u16Char;

  u16Char = ef_uni2oem( chr, u16ffCPGet( ) );
  /* Is this a DBC? */
  if ( 0x100 <= u16Char )
  {
    if ( 2 > szb )
      return 0;
    *buf++ = (char)(u16Char >> 8);  /* Store DBC 1st byte */
    *buf++ = (TCHAR)u16Char;      /* Store DBC 2nd byte */
    return 2;
  }
  if ( (0 == u16Char ) || ( 1 > szb ) )
    return 0;  /* Invalid char or buffer overflow? */
  *buf++ = (TCHAR)u16Char;          /* Store the character */
  return 1;
}

/* Output a TCHAR string in defined API encoding */
ef_return_et eEFPrvu8UTF16ToUTF16 (  /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t  chr,  /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR   * buf,  /* Output buffer */
  ef_u32_t  szb  /* Size of the buffer */
)
{
  ef_u08_t u8RetVal = 0;
  /* UTF-16 output */
  ucs2_t hs;
  ucs2_t u16Char;

  hs = (ucs2_t)(chr >> 16);
  u16Char = (ucs2_t)chr;
  if (hs == 0) {  /* Single encoding unit? */
    if (szb < 1 || IsSurrogate(u16Char)) return 0;  /* Buffer overflow or wrong code? */
    *buf = u16Char;
    return 1;
  }
  if (szb < 2 || !IsSurrogateH(hs) || !IsSurrogateL(u16Char)) return 0;  /* Buffer overflow or wrong surrogate? */
  *buf++ = hs;
  *buf++ = u16Char;
  return 2;

}

/* Output a TCHAR string in defined API encoding */
ef_return_et eEFPrvu8UTF16ToUTF8  (  /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t  chr,  /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR   * buf,  /* Output buffer */
  ef_u32_t  szb  /* Size of the buffer */
)
{
ef_u08_t u8RetVal = 0;

/* UTF-8 output */
  ef_u32_t hc;

  if (chr < 0x80) {  /* Single byte code? */
    if (szb < 1) return 0;  /* Buffer overflow? */
    *buf = (TCHAR)chr;
    return 1;
  }
  if (chr < 0x800) {  /* 2-byte sequence? */
    if (szb < 2) return 0;  /* Buffer overflow? */
    *buf++ = (TCHAR)(0xC0 | (chr >> 6 & 0x1F));
    *buf++ = (TCHAR)(0x80 | (chr >> 0 & 0x3F));
    return 2;
  }
  if (chr < 0x10000) {  /* 3-byte sequence? */
    if (szb < 3 || IsSurrogate(chr)) return 0;  /* Buffer overflow or wrong code? */
    *buf++ = (TCHAR)(0xE0 | (chr >> 12 & 0x0F));
    *buf++ = (TCHAR)(0x80 | (chr >> 6 & 0x3F));
    *buf++ = (TCHAR)(0x80 | (chr >> 0 & 0x3F));
    return 3;
  }
  /* 4-byte sequence */
  if (szb < 4) return 0;  /* Buffer overflow? */
  hc = ((chr & 0xFFFF0000) - 0xD8000000) >> 6;  /* Get high 10 bits */
  chr = (chr & 0xFFFF) - 0xDC00;          /* Get low 10 bits */
  if (hc >= 0x100000 || chr >= 0x400) return 0;  /* Wrong surrogate? */
  chr = (hc | chr) + 0x10000;
  *buf++ = (TCHAR)(0xF0 | (chr >> 18 & 0x07));
  *buf++ = (TCHAR)(0x80 | (chr >> 12 & 0x3F));
  *buf++ = (TCHAR)(0x80 | (chr >> 6 & 0x3F));
  *buf++ = (TCHAR)(0x80 | (chr >> 0 & 0x3F));
  return 4;
}

/* Output a TCHAR string in defined API encoding */
ef_return_et eEFPrvu8UTF16ToUTF32 (  /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t  chr,  /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR   * buf,  /* Output buffer */
  ef_u32_t  szb  /* Size of the buffer */
)
{
ef_u08_t u8RetVal = 0;
/* UTF-32 output */
  ef_u32_t hc;

  /* If Buffer overflow */
  if (szb < 1)
    return 0;
  /* Out of BMP? */
  if (chr >= 0x10000)
  {
    /* Get high 10 bits */
    hc = ((chr & 0xFFFF0000) - 0xD8000000) >> 6;
    /* Get low 10 bits */
    chr = (chr & 0xFFFF) - 0xDC00;
    /* If Wrong surrogate */
    if (    ( 0x100000 <= hc )
         || ( 0x400 <= chr ) )
    {
      return 0;
    }
    else
    {
      chr = (hc | chr) + 0x10000;
    }
  }
  *buf++ = (TCHAR)chr;
  return 1;
}

/* Get a Unicode code point from the TCHAR string in defined API encoding */
ef_return_et eEFPrvu32xCharToUTF16 (
  const TCHAR** ppxString
)
{
  ef_u32_t  u32Unicode = 0xFFFFFFFF;

  if ( EF_DEF_API_OEM == EF_CONF_API_ENCODING ) /* ANSI/OEM input */
  {
    u32Unicode = u32OEMToUTF16( ppxString );
  }
  else if ( EF_DEF_API_UTF16 == EF_CONF_API_ENCODING ) /* UTF-16 input */
  {
    u32Unicode = u32UTF16ToUTF16( ppxString );
  }
  else if ( EF_DEF_API_UTF8 == EF_CONF_API_ENCODING ) /* UTF-8 input */
  {
    u32Unicode = u32UTF8ToUTF16( ppxString );
  }
  else if ( EF_DEF_API_UTF32 == EF_CONF_API_ENCODING ) /* UTF-32 input */
  {
    u32Unicode = u32UTF32ToUTF16( ppxString );
  }
  else
  {
    u32Unicode = 0xFFFFFFFF; /* Error */
  }

  return u32Unicode;
}

/* Output a TCHAR string in defined API encoding */
ef_return_et eEFPrvPut_utf ( /* Returns number of encoding units written (0:buffer overflow or wrong encoding) */
  ef_u32_t  chr,  /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR *   buf,  /* Output buffer */
  ef_u32_t  szb   /* Size of the buffer */
)
{
  ef_u08_t u8RetVal = 0;

  if ( EF_DEF_API_OEM == EF_CONF_API_ENCODING ) /* ANSI/OEM input */
  {
    u8RetVal = u8UTF16ToOEM( chr, buf, szb );
  }
  else if ( EF_DEF_API_UTF16 == EF_CONF_API_ENCODING ) /* UTF-16 input */
  {
    u8RetVal = u8UTF16ToUTF16( chr, buf, szb );
  }
  else if ( EF_DEF_API_UTF8 == EF_CONF_API_ENCODING ) /* UTF-8 input */
  {
    u8RetVal = u8UTF16ToUTF8( chr, buf, szb );
  }
  else if ( EF_DEF_API_UTF32 == EF_CONF_API_ENCODING ) /* UTF-32 input */
  {
    u8RetVal = u8UTF16ToUTF32( chr, buf, szb );
  }
  else
  {
    u8RetVal = 0; /* Error */
  }

  return u8RetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
