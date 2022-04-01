/**
 * ********************************************************************************************************************
 *  @file     ef_prv_create_name.h
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
#ifndef EFAT_PRIVATE_CREATE_NAME_H
#define EFAT_PRIVATE_CREATE_NAME_H

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
 *  @brief  Pick a top segment and create the object name in directory form FAT version
 *
 *  @param  pxDir   Pointer to the directory object
 *  @param  ppxPath Pointer to pointer to the segment in the path string
 *
 *  @return Operation result
 *  @retval EF_RET_OK     Success
 *  @retval EF_RET_ERROR  An error occurred
 *  @retval EF_RET_ASSERT Assertion failed
 */
ef_return_et eEFPrvNameCreateFAT (
  ef_directory_st * pxDir,
  const TCHAR**     ppxPath
);

/**
 *  @brief  Pick a top segment and create the object name in directory form VFAT version
 *
 *  @param  pxDir   Pointer to the directory object
 *  @param  ppxPath Pointer to pointer to the segment in the path string
 *
 *  @return Operation result
 *  @retval EF_RET_OK     Success
 *  @retval EF_RET_ERROR  An error occurred
 *  @retval EF_RET_ASSERT Assertion failed
 */
ef_return_et eEFPrvNameCreateVFAT (
  ef_directory_st * pxDir,
  const TCHAR**     ppxPath
);

/**
 *  @brief  Pick a top segment and create the object name in directory form
 *
 *  @param  pxDir       Pointer to the directory object
 *  @param  ppxPath     Pointer to pointer to the segment in the path string
 *
 *  @return Operation result
 *  @retval EF_RET_OK       Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
#if ( 0 != EF_CONF_VFAT )
  /* LFN configuration */
  #define eEFPrvNameCreate( pxDir, ppxPath ) eEFPrvNameCreateVFAT( pxDir, ppxPath )
#else
  /* SFN Only configuration */
  #define eEFPrvNameCreate( pxDir, ppxPath ) eEFPrvNameCreateFAT( pxDir, ppxPath )
#endif

//ef_return_et eEFPrvNameCreate (
//  ef_directory_st*  pxDir,
//  const TCHAR**     pxPath
//);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_CREATE_NAME_H */
/* END OF FILE ***************************************************************************************************** */

