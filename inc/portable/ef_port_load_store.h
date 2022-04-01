/**
 * ********************************************************************************************************************
 *  @file     ef_port_load_store.h
 *  @ingroup  group_eFAT_Portable
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Header for portable functions to Load/Store multi-byte words with unaligned access
 *            Needed for access of data in FAT structure
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
#ifndef EFAT_PORTABLE_LOAD_STORE_H
#define EFAT_PORTABLE_LOAD_STORE_H
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
/* Local function prototypes ----------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions prototypes --------------------------------------------- */

/**
 *  @brief  Load a 2-byte little-endian word
 *
 *  @param  pu8src  Pointer to the 8 bits unsigned integer where data is stored
 *
 *  @return The 16 bits unsigned integer value
 */
ef_u16_t u16EFPortLoad (
  const ef_u08_t * pu8src
);

/**
 *  @brief  Load a 4-byte little-endian word
 *
 *  @param  pu8src  Pointer to the 8 bits unsigned integer where data is stored
 *
 *  @return The 32 bits unsigned integer value
 */
ef_u32_t u32EFPortLoad (
 const ef_u08_t * pu8src
);

/**
 *  @brief  Load a 8-byte little-endian word
 *
 *  @param  pu8src  Pointer to the 8 bits unsigned integer where data is stored
 *
 *  @return The 64 bits unsigned integer value
 */
ef_u64_t u64EFPortLoad (
  const ef_u08_t * pu8src
);

/**
 *  @brief  Load a 2-byte word in little-endian
 *
 *  @param  pu8src    Pointer to the 8 bits unsigned integer where to store the data
 *  @param  pu16value Pointer to the 16 bits unsigned integer value
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_INT_ERR              Assertion failed
 */
ef_return_et eEFPortLoadu16 (
  const ef_u08_t * pu8src,
  ef_u16_t       * pu16value
);

/**
 *  @brief  Load a 4-byte word in little-endian
 *
 *  @param  pu8src    Pointer to the 8 bits unsigned integer where to store the data
 *  @param  pu32value Pointer to the 32 bits unsigned integer value
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_INT_ERR              Assertion failed
 *
 */
ef_return_et eEFPortLoadu32 (
  const ef_u08_t * pu8src,
  ef_u32_t       * pu32value
);

/**
 *  @brief  Load a 8-byte word in little-endian
 *
 *  @param  pu8src    Pointer to the 8 bits unsigned integer where to store the data
 *  @param  pu64value Pointer to the 64 bits unsigned integer value
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_INT_ERR              Assertion failed
 */
ef_return_et eEFPortLoadu64 (
  const ef_u08_t * pu8src,
  ef_u64_t       * pu64value
);

/**
 *  @brief  Store a 2-byte word in little-endian
 *
 *  @param  pu8dst    Pointer to the 8 bits unsigned integer where to store the data
 *  @param  u16value  The 16 bits unsigned integer value
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_INT_ERR              Assertion failed
 */
ef_return_et eEFPortStoreu16 (
  ef_u08_t * pu8dst,
  ef_u16_t   u16value
);

/**
 *  @brief  Store a 4-byte word in little-endian
 *
 *  @param  pu8dst    Pointer to the 8 bits unsigned integer where to store the data
 *  @param  u32value  The 32 bits unsigned integer value
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_INT_ERR              Assertion failed
 *
 */
ef_return_et eEFPortStoreu32 (
  ef_u08_t * pu8dst,
  ef_u32_t   u32value
);

/**
 *  @brief  Store a 8-byte word in little-endian
 *
 *  @param  pu8dst    Pointer to the 8 bits unsigned integer where to store the data
 *  @param  u64value  he 64 bits unsigned integer value
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_INT_ERR              Assertion failed
 */
ef_return_et eEFPortStoreu64 (
  ef_u08_t * pu8dst,
  ef_u64_t   u64value
);

/**
 *  Avoid warnings when using eEFPortStoreu16 function without checking it's return value
 */
#define vEFPortStoreu16(dst,val)  (void)eEFPortStoreu16(dst,val)
/**
 *  Avoid warnings when using eEFPortStoreu32 function without checking it's return value
 */
#define vEFPortStoreu32(dst,val)  (void)eEFPortStoreu32(dst,val)
/**
 *  Avoid warnings when using eEFPortStoreu64 function without checking it's return value
 */
#define vEFPortStoreu64(dst,val)  (void)eEFPortStoreu64(dst,val)
/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PORTABLE_LOAD_STORE_H */
/* END OF FILE ***************************************************************************************************** */

