/**
 * ********************************************************************************************************************
 *  @file     ef_port_diskio.h
 *  @ingroup  group_eFAT_Portable
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Header file for Low level disk interface.
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
#ifndef EFAT_PORTABLE_DISK_IO_DEFINED
#define EFAT_PORTABLE_DISK_IO_DEFINED
#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include "efat.h"
/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Public functions prototypes---------------------------------------------- */

/**
 *  @brief  SD Card IO Drive Functions
 */
extern ef_drive_functions_st xffDriveFunctionsSDIO;

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PORTABLE_DISK_IO_DEFINED */
/* END OF FILE ***************************************************************************************************** */
