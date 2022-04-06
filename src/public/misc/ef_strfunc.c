/**
 * ********************************************************************************************************************
 *  @file     ef_strfunc.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    String function
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
#include "ef_prv_lock.h"
#include "ef_prv_string.h"
#include "ef_prv_volume.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_unicode.h"

#if (    ( 0 != EF_CONF_VFAT ) \
      && ( 0 != EF_CONF_STRF_ENCODING ) \
      && ( 0 >= EF_CONF_STRF_ENCODING ) \
      && ( 3 >= EF_CONF_STRF_ENCODING ) )
#error Wrong EF_CONF_STRF_ENCODING setting
#endif

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */

/**
 *  Putchar output buffer and work area
 */
typedef struct {
  ef_file_st *pxFile; /* Ptr to the writing file */
  int      idx;       /* Write index of buf[] (-1:error), number of encoding units written */
  int      nchr;      /* Write index of buf[] (-1:error), number of encoding units written */
#if ( 0 != EF_CONF_VFAT ) && EF_CONF_STRF_ENCODING == 1
  ucs2_t hs;
#elif ( 0 != EF_CONF_VFAT ) && EF_CONF_STRF_ENCODING == 2
  ef_u08_t bs[4];
  ef_u32_t    wi;
  ef_u32_t    ct;
#endif
  ef_u08_t buf[ 64 ];  /* Write buffer */
} putbuff;

/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */


/*-----------------------------------------------------------------------*/
/* Put a Character to the File (sub-functions)                           */
/*-----------------------------------------------------------------------*/

/* Buffered write with code conversion */
static  void putc_bfd ( putbuff* pb, TCHAR c );

/* Flush remaining characters in the buffer */
static  int putc_flush ( putbuff* pb );

/* Initialize write buffer */
static  void putc_init ( putbuff* pb, ef_file_st* pxFile );

/* Local functions --------------------------------------------------------- */

/* Buffered write with code conversion */
static void putc_bfd (
    putbuff* pb,
    TCHAR c
)
{
  ef_u32_t n;
  int       i;
  int       nc;
#if ( 0 != EF_CONF_VFAT ) && EF_CONF_STRF_ENCODING
  ucs2_t hs;
  ucs2_t u16Char;
#if EF_CONF_STRF_ENCODING == 2
  ef_u32_t dc;
  TCHAR *tp;
#endif
#endif

  if (    ( 0 != EF_CONF_CONVERT_LF_CRLF )
       && ( '\n' == c ) )
  {
    /* LF -> CRLF conversion */
    putc_bfd(pb, '\r');
  }

  /* Write index of pb->buf[] */
  i = pb->idx;
  if (i < 0) return;
  nc = pb->nchr;      /* Write unit counter */

#if ( 0 != EF_CONF_VFAT ) && EF_CONF_STRF_ENCODING
#if EF_CONF_STRF_ENCODING == 1    /* UTF-16 input */
  if (IsSurrogateH(c)) {  /* High surrogate? */
    pb->hs = c; return;  /* Save it for next */
  }
  hs = pb->hs; pb->hs = 0;
  if (hs != 0) {      /* There is a leading high surrogate */
    if (!IsSurrogateL(c)) hs = 0;  /* Discard high surrogate if not a surrogate pair */
  } else {
    if (IsSurrogateL(c)) return;  /* Discard stray low surrogate */
  }
  u16Char = c;
#elif EF_CONF_STRF_ENCODING == 2  /* UTF-8 input */
  for (;;) {
    if (pb->ct == 0) {  /* Out of multi-byte sequence? */
      pb->bs[pb->wi = 0] = (ef_u08_t)c;  /* Save 1st byte */
      if ((ef_u08_t)c < 0x80) break;          /* Single byte? */
      if (((ef_u08_t)c & 0xE0) == 0xC0) pb->ct = 1;  /* 2-byte sequence? */
      if (((ef_u08_t)c & 0xF0) == 0xE0) pb->ct = 2;  /* 3-byte sequence? */
      if (((ef_u08_t)c & 0xF1) == 0xF0) pb->ct = 3;  /* 4-byte sequence? */
      return;
    } else {        /* In the multi-byte sequence */
      if (((ef_u08_t)c & 0xC0) != 0x80) {  /* Broken sequence? */
        pb->ct = 0; continue;
      }
      pb->bs[++pb->wi] = (ef_u08_t)c;  /* Save the trailing byte */
      if (--pb->ct == 0) break;  /* End of multi-byte sequence? */
      return;
    }
  }
  tp = (TCHAR*)pb->bs;
  dc = u32xCharToUTF16(&tp);  /* UTF-8 ==> UTF-16 */
  if (dc == 0xFFFFFFFF) return;  /* Wrong code? */
  u16Char = (ucs2_t)dc;
  hs = (ucs2_t)(dc >> 16);
#elif EF_CONF_STRF_ENCODING == 3  /* UTF-32 input */
  if (IsSurrogate(c) || c >= 0x110000) return;  /* Discard invalid code */
  if (c >= 0x10000) {    /* Out of BMP? */
    hs = (ucs2_t)(0xD800 | ((c >> 10) - 0x40));   /* Make high surrogate */
    u16Char = 0xDC00 | (c & 0x3FF);          /* Make low surrogate */
  } else {
    hs = 0;
    u16Char = (ucs2_t)c;
  }
#endif
  /* A code point in UTF-16 is available in hs and u16Char */

#if EF_DEF_FILE_IO_UTF16LE == 1    /* Write a code point in UTF-16LE */
  if (hs != 0) {  /* Surrogate pair? */
    vEFPortStoreu16(&pb->buf[i], hs);
    i += 2;
    nc++;
  }
  vEFPortStoreu16(&pb->buf[i], u16Char);
  i += 2;
#elif EF_DEF_FILE_IO_UTF16BE == 2  /* Write a code point in UTF-16BE */
  if (hs != 0) {  /* Surrogate pair? */
    pb->buf[i++] = (ef_u08_t)(hs >> 8);
    pb->buf[i++] = (ef_u08_t)hs;
    nc++;
  }
  pb->buf[i++] = (ef_u08_t)(u16Char >> 8);
  pb->buf[i++] = (ef_u08_t)u16Char;
#elif EF_DEF_FILE_IO_UTF8 == 3  /* Write a code point in UTF-8 */
  if (hs != 0) {  /* 4-byte sequence? */
    nc += 3;
    hs = (hs & 0x3FF) + 0x40;
    pb->buf[i++] = (ef_u08_t)(0xF0 | hs >> 8);
    pb->buf[i++] = (ef_u08_t)(0x80 | (hs >> 2 & 0x3F));
    pb->buf[i++] = (ef_u08_t)(0x80 | (hs & 3) << 4 | (u16Char >> 6 & 0x0F));
    pb->buf[i++] = (ef_u08_t)(0x80 | (u16Char & 0x3F));
  } else {
    if (u16Char < 0x80) {  /* Single byte? */
      pb->buf[i++] = (ef_u08_t)u16Char;
    } else {
      if (u16Char < 0x800) {  /* 2-byte sequence? */
        nc += 1;
        pb->buf[i++] = (ef_u08_t)(0xC0 | u16Char >> 6);
      } else {      /* 3-byte sequence */
        nc += 2;
        pb->buf[i++] = (ef_u08_t)(0xE0 | u16Char >> 12);
        pb->buf[i++] = (ef_u08_t)(0x80 | (u16Char >> 6 & 0x3F));
      }
      pb->buf[i++] = (ef_u08_t)(0x80 | (u16Char & 0x3F));
    }
  }
#else            /* Write a code point in ANSI/OEM */
  if (hs != 0) return;
  u16Char = ef_uni2oem( u16Char, u16ffCPGet( ) );  /* UTF-16 ==> ANSI/OEM */
  (void) eEFPrvUnicode2OEM( u16Char, &u16Char, u16ffCPGet( ) );  /* UTF-16 ==> ANSI/OEM */
  if (u16Char == 0) return;
  if (u16Char >= 0x100) {
    pb->buf[i++] = (ef_u08_t)(u16Char >> 8); nc++;
  }
  pb->buf[i++] = (ef_u08_t)u16Char;
#endif

#else                  /* ANSI/OEM input (without re-encoding) */
  pb->buf[i++] = (ef_u08_t)c;
#endif

  if ( i >= (int)(sizeof pb->buf) - 4 )
  {
    /* Write buffered characters to the file */
    eEF_fwrite( pb->pxFile, pb->buf, (ef_u32_t)i, &n );
    i = (n == (ef_u32_t)i) ? 0 : -1;
  }
  pb->idx = i;
  pb->nchr = nc + 1;
}


/* Flush remaining characters in the buffer */
static  int putc_flush (
  putbuff * pb
)
{
  ef_u32_t nw;

/* Flush buffered characters to the file */
  if (    ( 0 <= pb->idx )
       && ( EF_RET_OK == eEF_fwrite( pb->pxFile, pb->buf, (ef_u32_t)pb->idx, &nw ) )
       && ( nw == ( ef_u32_t)pb->idx ) )
    {
      return pb->nchr;
    }
  return EF_EOF;
}

/* Initialize write buffer */
static  void putc_init (
  putbuff     * pb,
  ef_file_st  * pxFile
)
{
  eEFPortMemZero( pb, sizeof (putbuff) );
  pb->pxFile = pxFile;
}


/* Public functions ------------------------------------------------------------------------------------------------ */

/* Get a String from the File */
TCHAR* eEF_gets (
  TCHAR       * buff,   /* Pointer to the buffer to store read string */
  int           len,    /* Size of string buffer (items) */
  ef_file_st  * pxFile  /* Pointer to the file object */
)
{
  int       nc = 0;
  TCHAR *   p = buff;
  ef_u08_t   s[4];
  ef_u32_t      rc;
  ef_u32_t  dc;
#if ( 0 != EF_CONF_VFAT ) && EF_CONF_STRF_ENCODING && EF_CONF_STRF_ENCODING <= 2
  ucs2_t u16Char;
#endif
#if ( 0 != EF_CONF_VFAT ) && EF_CONF_STRF_ENCODING && EF_CONF_STRF_ENCODING == 3
  ef_u32_t ct;
#endif

  /* With code conversion (Unicode API) */
#if ( 0 != EF_CONF_VFAT ) && ( 0 != EF_CONF_STRF_ENCODING )
  /* Make a room for the character and terminator  */
  if (EF_CONF_STRF_ENCODING == 1)
  {
    len -= (EF_CONF_STRF_ENCODING == 0) ? 1 : 2;
  }
  if (EF_CONF_STRF_ENCODING == 2)
  {
    len -= (EF_CONF_STRF_ENCODING == 0) ? 3 : 4;
  }
  if (EF_CONF_STRF_ENCODING == 3)
  {
    len -= 1;
  }
  while ( nc < len )
  {
#if ( EF_DEF_FILE_IO_OEM == EF_CONF_STRF_ENCODING )
    /* Read a character in ANSI/OEM */
    f_read( pxFile, s, 1, &rc );
    /* Get a code unit */
    if ( rc != 1 )
      break;      /* EF_EOF? */
    u16Char = s[0];
    if (effCharInDBCRangesByte1((ef_u08_t)u16Char)) {  /* DBC 1st byte? */
      f_read(pxFile, s, 1, &rc);  /* Get DBC 2nd byte */
      if (rc != 1 || !effCharInDBCRangesByte2(s[0])) continue;  /* Wrong code? */
      u16Char = u16Char << 8 | s[0];
    }
//    dc = ef_oem2uni( u16Char, u16ffCPGet( ) );  /* OEM --> */
    eEFPrvOEM2Unicode( u16Char, &dc, u16ffCPGet( ) )
    if (dc == 0)
      continue;
#elif (    ( EF_DEF_FILE_IO_UTF16LE == EF_CONF_STRF_ENCODING )
        || ( EF_DEF_FILE_IO_UTF16BE == EF_CONF_STRF_ENCODING ) )
    /* Read a character in UTF-16LE/BE */
    f_read(pxFile, s, 2, &rc);    /* Get a code unit */
    if (rc != 2)
      break;      /* EF_EOF? */
    dc = (EF_CONF_STRF_ENCODING == 1) ? u16EFPortLoad(s) : s[0] << 8 | s[1];
    /* Broken surrogate pair? */
    if (IsSurrogateL(dc))
      continue;
    /* High surrogate? */
    if (IsSurrogateH(dc))
    {    /* High surrogate? */
      f_read(pxFile, s, 2, &rc);  /* Get low surrogate */
      if (rc != 2)
        break;    /* EF_EOF? */
      u16Char = (EF_CONF_STRF_ENCODING == 1) ? u16EFPortLoad(s) : s[0] << 8 | s[1];
      if (!IsSurrogateL(u16Char))
        continue;  /* Broken surrogate pair? */
      dc = ((dc & 0x3FF) + 0x40) << 10 | (u16Char & 0x3FF);  /* Merge surrogate pair */
    }
#elif (    ( EF_DEF_FILE_IO_UTF8 == EF_CONF_STRF_ENCODING )
  /* Read a character in UTF-8 */
    f_read(pxFile, s, 1, &rc);    /* Get a code unit */
    if (rc != 1) break;      /* EF_EOF? */
    dc = s[0];
    if (dc >= 0x80) {      /* Multi-byte sequence? */
      ct = 0;
      if ((dc & 0xE0) == 0xC0) { dc &= 0x1F; ct = 1; }  /* 2-byte sequence? */
      if ((dc & 0xF0) == 0xE0) { dc &= 0x0F; ct = 2; }  /* 3-byte sequence? */
      if ((dc & 0xF8) == 0xF0) { dc &= 0x07; ct = 3; }  /* 4-byte sequence? */
      if (ct == 0) continue;
      f_read(pxFile, s, ct, &rc);    /* Get trailing bytes */
      if (rc != ct) break;
      rc = 0;
      do {  /* Merge the byte sequence */
        if ((s[rc] & 0xC0) != 0x80) break;
        dc = dc << 6 | (s[rc] & 0x3F);
      } while (++rc < ct);
      if (rc != ct || dc < 0x80 || IsSurrogate(dc) || dc >= 0x110000) continue;  /* Wrong encoding? */
    }
#else
#error pipo
#endif
    /* A code point is avaialble in dc to be output */

    if (    ( 0 != EF_CONF_CONVERT_LF_CRLF )
         && ( dc == '\r' ) )
      /* Strip \r off if needed */
      continue;
    /* Output it in UTF-16/32 encoding */
#if (    ( EF_DEF_API_UTF16 == EF_CONF_API_ENCODING )
      || ( EF_DEF_API_UTF32 == EF_CONF_API_ENCODING ) )
    /* Out of BMP at UTF-16? */
    if (    ( EF_DEF_API_UTF16 == EF_CONF_API_ENCODING )
         && ( dc >= 0x10000 ) )
    {
      /* Make and output high surrogate */
      *p++ = (TCHAR)(0xD800 | ((dc >> 10) - 0x40));
      nc++;
      /* Make low surrogate */
      dc = 0xDC00 | (dc & 0x3FF);
    }
    *p++ = (TCHAR)dc;
    nc++;
    /* End of line? */
    if (dc == '\n')
      break;
    /* Output it in UTF-8 encoding */
#elif ( EF_DEF_API_UTF8 == EF_CONF_API_ENCODING )
    /* Single byte? */
    if (dc < 0x80)
    {
      *p++ = (TCHAR)dc;
      nc++;
      if (dc == '\n')
        break;  /* End of line? */
    }
    else
    {
      /* 2-byte sequence? */
      if (dc < 0x800)
      {
        *p++ = (TCHAR)(0xC0 | (dc >> 6 & 0x1F));
        *p++ = (TCHAR)(0x80 | (dc >> 0 & 0x3F));
        nc += 2;
      }
      else
      {
        /* 3-byte sequence? */
        if (dc < 0x10000) {
          *p++ = (TCHAR)(0xE0 | (dc >> 12 & 0x0F));
          *p++ = (TCHAR)(0x80 | (dc >> 6 & 0x3F));
          *p++ = (TCHAR)(0x80 | (dc >> 0 & 0x3F));
          nc += 3;
        }
        else
        {      /* 4-byte sequence? */
          *p++ = (TCHAR)(0xF0 | (dc >> 18 & 0x07));
          *p++ = (TCHAR)(0x80 | (dc >> 12 & 0x3F));
          *p++ = (TCHAR)(0x80 | (dc >> 6 & 0x3F));
          *p++ = (TCHAR)(0x80 | (dc >> 0 & 0x3F));
          nc += 4;
        }
      }
    }
#endif
  }

#else
  /* Byte-by-byte read without any conversion (ANSI/OEM API) */
  len -= 1;  /* Make a room for the terminator */
  while ( nc < len )
  {
    /* Get a byte */
    eEF_fread( pxFile, s, 1, &rc );
    /* EF_EOF? */
    if (rc != 1)
      break;
    dc = s[ 0 ];
    if (    ( 0 != EF_CONF_CONVERT_LF_CRLF )
         && ( '\r' == dc ) )
    {
      continue;
    }
    *p++ = (TCHAR)dc;
    nc++;
    if ( dc == '\n' )
      break;
  }
#endif

  /* Terminate the string */
  *p = 0;
  /* When no data read due to EF_EOF or error, return with error. */
  return nc ? buff : 0;
}




#include <stdarg.h>
/*-----------------------------------------------------------------------*/
/* Put a Character to the File (sub-functions)                           */
/*-----------------------------------------------------------------------*/

///* Putchar output buffer and work area */
//
//typedef struct {
//  ef_file_st *pxFile;    /* Ptr to the writing file */
//  int idx, nchr;  /* Write index of buf[] (-1:error), number of encoding units written */
//#if EF_CONF_VFAT && EF_CONF_STRF_ENCODING == 1
//  ucs2_t hs;
//#elif EF_CONF_VFAT && EF_CONF_STRF_ENCODING == 2
//  ef_u08_t bs[4];
//  ef_u32_t wi, ct;
//#endif
//  ef_u08_t buf[64];  /* Write buffer */
//} putbuff;
//



int eEF_putc (
  TCHAR         c,      /* A character to be output */
  ef_file_st  * pxFile  /* Pointer to the file object */
)
{
  putbuff pb;
  int iRetVal = -1;

  putc_init( &pb, pxFile );
  /* Put the character */
  putc_bfd( &pb, c );
  return putc_flush( &pb );
}




/*-----------------------------------------------------------------------*/
/* Put a String to the File                                              */
/*-----------------------------------------------------------------------*/

int eEF_puts (
  const TCHAR * str,    /* Pointer to the string to be output */
  ef_file_st  * pxFile  /* Pointer to the file object */
)
{
  putbuff pb;


  putc_init( &pb, pxFile );
  while ( 0 != (*str) )
  {
    /* Put the string */
    putc_bfd( &pb, *str++ );
  }
  return putc_flush( &pb );
}

/*-----------------------------------------------------------------------*/
/* Put a Formatted String to the File                                    */
/*-----------------------------------------------------------------------*/

int eEF_printf (
  ef_file_st  * pxFile,   /* Pointer to the file object */
  const TCHAR * u8Format, /* Pointer to the format string */
  ...                     /* Optional arguments... */
)
{
  va_list arp;
  putbuff pb;
  ef_u08_t f;
  ef_u08_t r;
  ef_u32_t i;
  ef_u32_t j;
  ef_u32_t w;
  ef_u32_t v;
  TCHAR c;
  TCHAR d;
  TCHAR str[32];
  TCHAR *p;


  putc_init(&pb, pxFile);

  va_start(arp, u8Format);

  for (;;)
  {
    c = *u8Format++;
    if ( 0 == c )
    {
      break;      /* End of string */
    }
    if ( '%' != c )
    {        /* Non escape character */
      putc_bfd(&pb, c);
      continue;
    }
    w = f = 0;
    c = *u8Format++;
    if ( '0' == c )
    {        /* Flag: '0' padding */
      f = 1;
      c = *u8Format++;
    }
    else
    {
      if ( '-' == c )
      {      /* Flag: left justified */
        f = 2;
        c = *u8Format++;
      }
    }
    if ( '*' == c )
    {        /* Minimum width by argument */
      w = va_arg(arp, int);
      c = *u8Format++;
    }
    else
    {
      while ( 0 != IsDigit(c) )
      {  /* Minimum width */
        w = w * 10 + c - '0';
        c = *u8Format++;
      }
    }
    if ( ( 'l' == c ) || ( 'L' == c ) )
    {  /* Type prefix: Size is long int */
      f |= 4;
      c = *u8Format++;
    }
    if ( 0 == c )
    {
      break;
    }
    d = c;
    if ( 0 != IsLower(d) )
    {
      d -= 0x20;
    }
    switch ( d )
    {        /* Atgument type is... */
    case 'S' :          /* String */
      p = va_arg( arp, TCHAR* );
      for ( j = 0 ; 0!= p[ j ]; j++ )
      {
        ;
      }
      if ( 0 == ( 2 & f ) )
      {            /* Right padded */
        while ( j++ < w )
        {
          putc_bfd( &pb, ' ' );
        }
      }
      while ( 0 != *p )
      {
        putc_bfd( &pb, *p++ );    /* String body */
      }
      while ( j++ < w )
      {
        putc_bfd( &pb, ' ' );  /* Left padded */
      }
      continue;

    case 'C' :          /* Character */
      putc_bfd( &pb, (TCHAR)va_arg( arp, int ) );
      continue;

    case 'B' :          /* Unsigned binary */
      r = 2;
      break;

    case 'O' :          /* Unsigned octal */
      r = 8;
      break;

    case 'D' :          /* Signed decimal */
    case 'U' :          /* Unsigned decimal */
      r = 10;
      break;

    case 'X' :          /* Unsigned hexdecimal */
      r = 16;
      break;

    default:          /* Unknown type (pass-through) */
      putc_bfd( &pb, c );
      continue;
    }

    /* Get an argument and put it in numeral */
    //v = (f & 4) ? (ef_u32_t)va_arg(arp, long) : ((d == 'D') ? (ef_u32_t)(long)va_arg(arp, int) : (ef_u32_t)va_arg(arp, unsigned int));
    if ( 0 != ( f & 4 ) )
    {
      v = (ef_u32_t)va_arg(arp, long);
    }
    else
    {
      //v = ((d == 'D') ? (ef_u32_t)(long)va_arg(arp, int) : (ef_u32_t)va_arg(arp, unsigned int));
      if ( d == 'D' )
      {
        v = (ef_u32_t)(long)va_arg(arp, int);
      }
      else
      {
        v = (ef_u32_t)va_arg(arp, unsigned int);
      }
    }
    if (     ( 'D' == d )
          && ( 0 != ( 0x80000000 & v ) ) )
    {
      v  = 0 - v;
      f |= 8;
    }
    i = 0;
    do {
      d = (TCHAR)(v % r);
      v /= r;
      if ( 9 < d )
      {
        if ( 'x' == c )
        {
          d += 0x27;
        }
        else
        {
          d += 0x07;
        }
      }
      str[ i++ ] = d + '0';
    } while ( ( 0 != v ) && ( i < ( sizeof(str) / sizeof (*str) ) ) );
    if ( 0 != ( 8 & f ) )
    {
      str[ i++ ] = '-';
    }
    j = i;
    if ( 0 !=  ( 1 & f ) )
    {
      d = '0';
    }
    else
    {
      d = ' ';
    }
    if ( 0 == ( 2 & f ) )
    {
      while ( j++ < w )
      {
        putc_bfd( &pb, d );  /* Right pad */
      }
    }
    do {
      putc_bfd( &pb, str[ --i ] );      /* Number body */
    } while ( 0 != i );
    while ( j++ < w )
    {
      putc_bfd( &pb, d );    /* Left pad */
    }
  }

  va_end(arp);

  return putc_flush( &pb );
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
