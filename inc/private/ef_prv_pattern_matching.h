/**
 * ********************************************************************************************************************
 *  @file     ef_prv_pattern_matching.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private Header file.
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
#ifndef EFAT_PRIVATE_PATTERN_MATCHING_H
#define EFAT_PRIVATE_PATTERN_MATCHING_H

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
/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

/**
 *  @brief  Pattern matching testing
 *
 *  @param  pxPattern   Matching pattern
 *  @param  pxString    String to be tested
 *  @param  skip        Number of pre-skip chars (number of ?s)
 *  @param  inf         Infinite search (* specified)
 *
 *  @return Operation result
 *  @retval EF_RET_OK       Success (match)
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvPatternMatching (
  const TCHAR * pxPattern,
  const TCHAR * pxString,
  int           skip,
  int           inf
);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_PATTERN_MATCHING_H */
/* END OF FILE ***************************************************************************************************** */

