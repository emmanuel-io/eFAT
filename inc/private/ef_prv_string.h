/**
 * ********************************************************************************************************************
 *  @file     ef_prv_string.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private Header file for string functions.
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
#ifndef EFAT_PRIVATE_STRING_H
#define EFAT_PRIVATE_STRING_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include <efat.h>
#include "ef_prv_def.h"
/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Character code support macros */

/**
 *  Test if c is an upper case character in ASCII
 */
#define IsUpper(c)      ( ( 'A' <= (c) ) && ( 'Z' >= (c) ) )

/**
 *  Test if c is an lower case character in ASCII
 */
#define IsLower(c)      ( ( 'a' <= (c) ) && ( 'z' >= (c) ) )

/**
 *  Test if c is an digital character in ASCII
 */
#define IsDigit(c)      ( ( '0' <= (c) ) && ( '9' >= (c) ) )

/**
 *  Test if c is a surrogate encoding
 */
#define IsSurrogate(c)  ( ( 0xD800 <= (c) ) && ( 0xDFFF >= (c) ) )

/**
 *  Test if c is a surrogate encoding in high range
 */
#define IsSurrogateH(c) ( ( 0xD800 <= (c) ) && ( 0xDBFF >= (c) ) )

/**
 *  Test if c is a surrogate encoding in low range
 */
#define IsSurrogateL(c) ( ( 0xDC00 <= (c) ) && ( 0xDFFF >= (c) ) )

/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */


/* Check if chr is contained in the string */
/**
 *  @brief  Check if a contains a character
 *
 *  @param  pcString    Pointer to the string where to find the character
 *  @param  cCharacter  Character to look to find in the string
 *
 *  @return Operation result
 *  @retval EF_RET_OK       Success (found)
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvStringFindChar (
  const char  * pcString,
  char          cCharacter
);

/**
 *  @brief  Returns a character in UTF-16 encoding from the TCHAR string in defined API encoding
 *
 *  @param  ppxString  Pointer to pointer to TCHAR string in configured encoding
 *
 *  @return UTF-16 encoded character (Surrogate pair if >=0x10000)
 *  @retval 0xFFFFFFFF on decode error
 */
ef_return_et eEFPrvu32xCharToUnicode (
  const TCHAR** ppxString,
  ef_u32_t    * pu32UnicodeOut
);

/**
 *  @brief  Output a TCHAR string in defined API encoding
 *
 *  @param  chr  UTF-16 encoded character (Surrogate pair if >=0x10000)
 *  @param  buf  Output buffer
 *  @param  szb  Size of the buffer
 *  @return Returns number of encoding units written
 *  @retval 0 Buffer Overflow or wrong encoding
 */
ef_return_et eEFPrvUnicodePut (
  ef_u32_t    u32Char,          /* UTF-16 encoded character (Surrogate pair if >=0x10000) */
  TCHAR     * pxBufferOut,      /* Output buffer */
  ef_u32_t    u32BufferSize,    /* Size of the buffer */
  ef_u32_t  * pu32EncodingUnits /* Pointer to the number of encoding units written (0:buffer overflow or wrong encoding) */
);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_STRING_H */
/* END OF FILE ***************************************************************************************************** */
