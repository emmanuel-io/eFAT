/**
 * ********************************************************************************************************************
 *  @file     ef_chdrive.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Change Current Drive
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
#include <ef_prv_def.h>
#include "ef_prv_volume_nb.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_chdrive (
  const TCHAR * pxPath
)
{
  EF_ASSERT_PUBLIC( 0 != pxPath );

  ef_return_et  eRetVal = EF_RET_OK;
  int8_t        s8VolumeNb;

  /* Get current volume number */
  if ( EF_RET_OK != eEFPrvVolumeNbGet( &pxPath, &s8VolumeNb ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_DRIVE );
  }
  /* Update current volume number */
  else if ( EF_RET_OK != eEFPrvVolumeNbCurrentSet( s8VolumeNb ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_PARAMETER );
  }
  else
  {
    EF_CODE_COVERAGE( );
  }
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
