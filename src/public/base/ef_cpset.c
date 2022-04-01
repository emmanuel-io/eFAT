/**
 * ********************************************************************************************************************
 *  @file     ef_cpset.c
 *  @ingroup  group_eFAT_Public
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Set Active Codepage for the ANSI/OEM encoding
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

#include <efat.h>
#include "ef_prv_unicode.h"

/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_CPSet (
  ef_u16_t  u16CodePage
)
{
  /* If we found the code page in the table of supported ones */
  return eEFPrvCPSet( u16CodePage );
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

