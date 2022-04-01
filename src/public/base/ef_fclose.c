/**
 * ********************************************************************************************************************
 *  @file     ef_fclose.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    File Close
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
#include "ef_prv_lock.h"
#include "ef_prv_validate.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_fclose (
  EF_FILE * pxFile
)
{
  EF_ASSERT_PUBLIC( 0 != pxFile );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st    * pxFS;

  /* Flush cached data */
  if ( EF_RET_OK != eEF_fsync( pxFile ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  /* Lock volume */
  else if ( EF_RET_OK != eEFPrvValidateObject( &pxFile->xObject, &pxFS ) )
  {
    /* Invalidate file object */
    pxFile->xObject.pxFS = 0;
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  /* Decrement file open counter */
  else if ( EF_RET_OK != eEFPrvLockDec( pxFile->xObject.u32LockId ) )
  {
    /* Invalidate file object */
    pxFile->xObject.pxFS = 0;
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    EF_CODE_COVERAGE( );
  }

  /* Unlock volume */
  (void) eEFPrvFSUnlockForce( pxFS );

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

