/**
 * ********************************************************************************************************************
 *  @file     ef_prv_gpt.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private GPT support function.
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
#ifndef EFAT_PRIVATE_GPT_H
#define EFAT_PRIVATE_GPT_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/** @defgroup groupFUNCTION_GPT  GUID Partion Table Support
 *  This is all what is needed to use GPT
 *  @{
 */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include <efat.h>      /* Declarations of eFAT API */
#include "ef_port_diskio.h"    /* Declarations of device I/O functions */
#include "ef_prv_def.h"      /* Declarations of eFAT API */
/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */
/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
/*--------------------------------------------------------------------------
 *
 *  @brief   GPT support function
 *------------------------------------------------------------------------*/
/**
 *  @brief  Calculate CRC32 in byte-by-byte
 *
 *  @param  u32CRC  Current CRC value
 *  @param  u8Byte  A byte to be processed
 *
 *  @return Next CRC value
 */
ef_u32_t u32ffCRC32 (
  ef_u32_t u32CRC,
  ef_u08_t u8Byte
);

/**
 *  @brief  Check validity of GPT header
 *
 *  @param  pu8GPTHeader  Pointer to the GPT header
 *
 *  @return Test result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvGPTHeaderTest (
  const ef_u08_t * pu8GPTHeader
);


/** @} */ /* end of groupFUNCTION_GPT */

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_GPT_H */
/* END OF FILE ***************************************************************************************************** */


