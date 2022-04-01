/**
 * ********************************************************************************************************************
 *  @file     ef_prv_volume_nb.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private volume numbering.
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
#ifndef EFAT_PRIVATE_VOLUME_NB_H
#define EFAT_PRIVATE_VOLUME_NB_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */


/* Includes -------------------------------------------------------------------------------------------------------- */
#include <efat.h>             /* Declarations of eFAT API */
#include "ef_port_diskio.h" /* Declarations of device I/O functions */
#include "ef_prv_def.h"
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
/* Remark: Variables defined here without initial value shall be guaranteed
/  zero/null at start-up. If not, the linker option or start-up routine is
/  not compliance with C standard. */


/**
 *  @brief  Remove volume letter from path name
 *
 *  @param  ppxPath Pointer to pointer to the path name
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvVolumeNbPathRemove (
  const TCHAR **  ppxPath
);

/**
 *  @brief  Get volume number from path name
 *
 *  @param  ppxPath     Pointer to pointer to the path name
 *  @param  ps8VolumeNb Pointer to the volume number to update
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvVolumeNbGet (
  const TCHAR **  ppxPath,
  int8_t      *   ps8VolumeNb
);

/* Set current volume number in relative path */
ef_return_et eEFPrvVolumeNbCurrentSet (
  int8_t s8VolumeNb
);

/* Get current volume number in relative path */
ef_return_et eEFPrvVolumeNbCurrentGet (
  int8_t  * ps8VolumeNb
);


/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_VOLUME_NB_H */
/* END OF FILE ***************************************************************************************************** */

