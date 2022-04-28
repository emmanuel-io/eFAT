/**
 * ********************************************************************************************************************
 *  @file     ef_prv_gpt.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    GPT support functions.
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
#include <ef_prv_def_gpt.h>
#include "ef_prv_def.h"
#include "ef_prv_gpt.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/*-----------------------------------------------------------------------*/
/* GPT support functions                                                 */
/*-----------------------------------------------------------------------*/

/* Calculate CRC32 in byte-by-byte */

ef_u32_t u32ffCRC32 (
  ef_u32_t u32CRC,
  ef_u08_t u8Byte
)
{
  for ( ef_u08_t b = 1 ; 0 != b ; b <<= 1 )
  {
    if ( 0 != ( u8Byte & b ) )
    {
      u32CRC ^= 1;
    }
    else
    {
      u32CRC ^= 0;
    }
    if ( 0 != ( 0x00000001 & u32CRC ) )
    {
      u32CRC = u32CRC >> 1 ^ 0xEDB88320;
    }
    else
    {
      u32CRC = u32CRC >> 1;
    }
    //u32CRC ^= ( 0 != (u8Byte & b) ) ? 1 : 0;
    //u32CRC = ( 0 != (u32CRC & 1) ) ? u32CRC >> 1 ^ 0xEDB88320 : u32CRC >> 1;
  }
  return u32CRC;
}


/* Check validity of GPT header */
ef_return_et eEFPrvGPTHeaderTest (
  const ef_u08_t * pu8GPTHeader
)
{
  EF_ASSERT_PRIVATE( 0 != pu8GPTHeader );

  ef_return_et  eRetVal = EF_RET_OK;
  /* If GPT header Signature, version (1.0) and length (92)
   * are different from what we can handle */
  if ( EF_RET_OK != eEFPortMemCompare(  pu8GPTHeader + EF_GPT_HEADER_OFFSET_SIGNATURE,
                                        "EFI PART" "\0\0\1\0" "\x5C\0\0", 16 ) )
  {
    ;  /* Error, we return */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    ef_u32_t  u32BCC = 0xFFFFFFFF;

    for ( ef_u32_t i = 0 ; i < 92 ; i++ )
    {    /* Check header BCC */
      //u32BCC = u32ffCRC32( u32BCC, i - EF_GPT_HEADER_OFFSET_HEADER_CRC32 < 4 ? 0 : pu8GPTHeader[ i ] );
      if ( 4 > ( i - EF_GPT_HEADER_OFFSET_HEADER_CRC32 ) )
      {
        u32BCC = u32ffCRC32( u32BCC, 0 );
      }
      else
      {
        u32BCC = u32ffCRC32( u32BCC, pu8GPTHeader[ i ] );
      }
    }

    /* If computed Checksum is not the same as on the disk */
    if ( ~u32BCC != u32EFPortLoad( pu8GPTHeader + EF_GPT_HEADER_OFFSET_HEADER_CRC32 ) )
    {
      ; /* Invalid */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    /* Else, if Table entry size is wrong (must be EF_GPT_PTE_SIZE bytes) */
    else if ( EF_GPT_PTE_SIZE != u32EFPortLoad( pu8GPTHeader + EF_GPT_HEADER_OFFSET_PTE_SIZE ) )
    {
      ; /* Invalid */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    /* Else, if Table size is wrong (must be 128 entries or less) */
    else if ( 128 < u32EFPortLoad( pu8GPTHeader + EF_GPT_HEADER_OFFSET_PTE_NB ) )
    {
      ; /* Invalid */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    /* Else, everything is good */
    else
    {
      EF_CODE_COVERAGE( );
    }
  }
  return eRetVal;
}


/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
