/**
 * ********************************************************************************************************************
 *  @file     ef_prv_unicode.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private Header for unicode support.
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
#ifndef EFAT_PRIVATE_UNICODE_H
#define EFAT_PRIVATE_UNICODE_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include <efat.h>
/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/**
 *  Define the number of single byte code pages enabled
 */
#define EF_CP_SBCS_NB (   EF_CONF_CP437 \
                        + EF_CONF_CP720 \
                        + EF_CONF_CP737 \
                        + EF_CONF_CP771 \
                        + EF_CONF_CP775 \
                        + EF_CONF_CP850 \
                        + EF_CONF_CP852 \
                        + EF_CONF_CP855 \
                        + EF_CONF_CP857 \
                        + EF_CONF_CP860 \
                        + EF_CONF_CP861 \
                        + EF_CONF_CP862 \
                        + EF_CONF_CP863 \
                        + EF_CONF_CP864 \
                        + EF_CONF_CP865 \
                        + EF_CONF_CP866 \
                        + EF_CONF_CP869 )

/**
 *  Define the number of double bytes code pages enabled
 */
#define EF_CP_DBCS_NB (   EF_CONF_CP932 \
                        + EF_CONF_CP936 \
                        + EF_CONF_CP949 \
                        + EF_CONF_CP950 )

/**
 *  Number of code page supported
 */
#define EF_CODE_PAGE_SUPPORTED_NB ( EF_CP_SBCS_NB + EF_CP_DBCS_NB )

/**
 *  Number of code page supported should not be null
 */
#if ( 0 == EF_CODE_PAGE_SUPPORTED_NB )
  #error Wrong configuration, At least one code page need to be enabled (ffconf.h).
#endif

#define EF_CODE_PAGE  0

/*--------------------------------*/
/* LFN/Directory working buffer   */
/*--------------------------------*/


#include <stdarg.h>

/* Get the active code page */
ef_u16_t u16ffCPGet( void );

/* Set the active code page */
ef_return_et eEFPrvCPSet( ef_u16_t u16CP );

//ef_u08_t u8ffToUpperExtendedCharacter(
//  ef_u08_t u8Char
//);
ef_return_et eEFPrvu8ToUpperExtendedCharacter (
  ef_u08_t    u8Char,
  ef_u08_t  * pu8Char
);

//ef_u16_t u16ffToUpperExtendedCharacter(
//  ef_u16_t u16Char
//);
ef_return_et eEFPrvu16ToUpperExtendedCharacter (
  ef_u16_t    u16Char,
  ef_u16_t  * pu16Char
);

//ef_u32_t u32ffToUpperExtendedCharacter(
//  ef_u32_t u32Char
//);
ef_return_et eEFPrvu32ToUpperExtendedCharacter (
  ef_u32_t    u32Char,
  ef_u32_t  * pu32Char
);

ef_u16_t u16ffUnicodeToUpperANSIOEM(
  ef_u16_t u16Char
);
ef_return_et eEFPrvUnicodeToUpperANSIOEM (
  ef_u16_t    u16UnicodeIn,
  ef_u16_t  * pu16OEMOut
);

/**
 *  @brief  Test if the byte is in the ranges of a DBC 1st byte
 *
 *  @param  u8Byte  The byte value to test
 *
 *  @return The test result
 *  @retval EF_RET_OK     Byte is in range
 *  @retval EF_RET_ERROR  Byte is out of range
 */
ef_return_et eEFPrvCharInDBCRangesByte1 (
  ef_u08_t  u8Byte
);

/**
 *  @brief  Test if the byte is in the ranges of a DBC 2nd byte
 *
 *  @param  u8Byte  The byte value to test
 *
 *  @return The test result
 *  @retval EF_RET_OK     Byte is in range
 *  @retval EF_RET_ERROR  Byte is out of range
 */
ef_return_et eEFPrvCharInDBCRangesByte2 (
  ef_u08_t  u8Byte
);

/*--------------------------------------------------------------*/
/* Additional user defined functions                            */

/* LFN support functions */
/* Code conversion (defined in unicode.c) */

/**
 *  @brief  Convert an OEM code point to an Unicode code point
 *
 *  @param  u16OEMCodeIn  The OEM code point to convert (DBC if >=0x100)
 *  @param  u16CodePage   The unicode code point to convert
 *
 *  @return The Unicode converted code point, zero on error
 */
//ucs2_t ef_oem2uni (
//  ucs2_t    u16OEMCodeIn,
//  ef_u16_t  u16CodePage
//);
ef_return_et eEFPrvUnicode2OEM (
  ef_u32_t    u32UnicodeIn,
  ef_u32_t  * pu32OEMOut,
  ef_u16_t    u16CodePage
);

/**
 *  @brief  Convert an Unicode code point to an OEM code point
 *
 *  @param  u32UnicodeIn  The OEM code point to convert
 *  @param  u16CodePage   The unicode code point to convert
 *
 *  @return The OEM converted code point, zero on error
 */
//ucs2_t ef_uni2oem (
//  ef_u32_t  u32UnicodeIn,
//  ef_u16_t  u16CodePage
//);
ef_return_et eEFPrvUnicode2OEM (
  ef_u32_t    u32UnicodeIn,
  ef_u32_t  * pu32OEMOut,
  ef_u16_t    u16CodePage
);

/**
 *  @brief  Convert an Unicode code point to upper case
 *
 *  @param  u32UnicodeIn The unicode code point to convert
 *
 *  @return The upper case converted code point
 */
//ef_u32_t ef_wtoupper (
//  ef_u32_t  u32UnicodeIn
//);
ef_return_et eEFPrvUnicodeToUpper (
  ef_u32_t    u32UnicodeIn,
  ef_u32_t  * pu32UnicodeOut
);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_UNICODE_H */
/* END OF FILE ***************************************************************************************************** */

