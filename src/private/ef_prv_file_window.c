/**
 * ********************************************************************************************************************
 *  @file     ef_prv_file_window.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    File window management.
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
#include "ef_prv_drive.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEFPrvFileWindowDirtyWriteBack (
  ef_file_st  * pxFile,
  ef_fs_st    * pxFS
)
{
  EF_ASSERT_PRIVATE( 0 != pxFile );
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_OK;

  /* Write-back dirty sector cache */
  if ( 0 == ( EF_FILE_WIN_DIRTY & pxFile->u8StatusFlags ) )
  {
    EF_CODE_COVERAGE( );
  }
  else if ( EF_RET_OK != eEFPrvDriveWrite(  pxFS->u8PhysDrv,
                                                  pxFile->u8Window,
                                                  pxFile->xSector,
                                                  1 )  )
  {
    pxFile->u8ErrorCode = (ef_u08_t) EF_RET_DISK_ERR;
    eRetVal = EF_RET_ERROR;
  }
  else
  {
    pxFile->u8StatusFlags &= (ef_u08_t)~EF_FILE_WIN_DIRTY;
  }

  return eRetVal;
}


ef_return_et eEFPrvFileWindowUpdate (
  ef_file_st  * pxFile,
  ef_fs_st    * pxFS,
  ef_lba_t      xSector
)
{
  EF_ASSERT_PRIVATE( 0 != pxFile );
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_OK;

  /* If Data sector is still the one in the window */
  if ( pxFile->xSector == xSector )
  {
    /* Do nothing */
    EF_CODE_COVERAGE( );
  }
  /* Else, if Write-back dirty sector cache if needed failed */
  else if ( EF_RET_OK != eEFPrvFileWindowDirtyWriteBack ( pxFile, pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
  }
  /* Else, if Reload sector cache failed */
  else if ( EF_RET_OK != eEFPrvDriveRead( pxFS->u8PhysDrv, pxFile->u8Window, xSector, 1 ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
  }
  else
  {
    /* Now the sector in the window is where the FileOffset belong */
    pxFile->xSector = xSector;
  }

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

