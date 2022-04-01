/**
 * ********************************************************************************************************************
 *  @file     ef_prv_validate.h
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
#ifndef EFAT_PRIVATE_VALIDATE_H
#define EFAT_PRIVATE_VALIDATE_H

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
/* Remark: Variables defined here without initial value shall be guaranteed
/  zero/null at start-up. If not, the linker option or start-up routine is
/  not compliance with C standard. */

/**
 *  @brief   Check if the file/directory object is valid or not
 *
 *  @param  pxObject  Pointer to the ef_object_st, the 1st member in the ef_file_st/ef_directory_st object, to check validity
 *  @param  ppxFS     Pointer to pointer to the owner filesystem object to return
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_INVALID_OBJECT       The file/directory object is invalid
 *  @retval EF_RET_TIMEOUT              Could not get a grant to access the volume within defined period
 */
ef_return_et eEFPrvValidateObject (
    ef_object_st *  pxObject,
    ef_fs_st     ** ppxFS
);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_VALIDATE_H */
/* END OF FILE ***************************************************************************************************** */

