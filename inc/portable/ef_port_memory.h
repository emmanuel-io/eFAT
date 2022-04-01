/**
 * ********************************************************************************************************************
 *  @file     ef_port_memory.h
 *  @ingroup  group_eFAT_Portable
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Header for portable memory fills & compare functions.
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
#ifndef EFAT_PORTABLE_MEMORY_H
#define EFAT_PORTABLE_MEMORY_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include <ef_prv_def.h>
#include "ef_port_types.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/**
 *  @brief  Compare memory block
 *
 *  @param  pvBufferA   Pointer to the data buffer A
 *  @param  pvBufferB   Pointer to the data buffer B
 *  @param  u32Count    Directory table to clear
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success (equal)
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPortMemCompare (
  const void  * pvBufferA,
  const void  * pvBufferB,
  ef_u32_t      u32Count
);

/**
 *  @brief  Copy memory byte by byte
 *
 *  @param  pvSrc       Pointer to the source data
 *  @param  pvDst       Pointer to the destination data
 *  @param  u32BytesNb  Number of bytes to transfer
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPortMemCopy (
  const void  * pvSrc,
  void        * pvDst,
  ef_u32_t      u32BytesNb
);

/**
 *  @brief  Set memory to zero
 *
 *  @param  pvDst       Pointer to the destination data
 *  @param  u32BytesNb  Number of bytes to transfer
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPortMemZero (
  void      * pvDst,
  ef_u32_t    u32BytesNb
);

/**
 *  @brief  Set memory to a known value
 *
 *  @param  pvDst       Pointer to the destination data
 *  @param  u8Value     Value to set each byte to
 *  @param  u32BytesNb  Number of bytes to transfer
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPortMemSet (
  void      * pvDst,
  ef_u08_t    u8Value,
  ef_u32_t    u32BytesNb
);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PORTABLE_MEMORY_H */
/* END OF FILE ***************************************************************************************************** */
