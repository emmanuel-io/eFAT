/**
 * ********************************************************************************************************************
 *  @file     ef_fdisk.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Create Partition Table on the Physical Drive
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
#include <efat_level3.h>
#include <ef_prv_def.h>
#include "ef_prv_drive.h"
#include "ef_prv_volume.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_fdisk (
    ef_u08_t        u8PhysDrv,
    const ef_lba_t  pxSizes[ ],
    void          * pvBuffer
)
{
  ef_u08_t      * pu8Window = (ef_u08_t*) pvBuffer;
  ef_return_et    eRetVal = EF_RET_OK;


  eRetVal =  eEFPrvDriveInitialize( u8PhysDrv );
  if ( EF_RET_DISK_NOINIT == eRetVal )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NOT_READY );
  }
  else if ( EF_RET_DISK_PROTECT == eRetVal )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_WRITE_PROTECTED );
  }
  else if ( 0 == pu8Window )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NOT_ENOUGH_CORE );
  }
  if ( EF_RET_OK != eEFPrvPartitionCreate( u8PhysDrv, pxSizes, 0x07, pu8Window ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  else
  {
    EF_CODE_COVERAGE( );
  }
//  LEAVE_MKFS(effPartitionCreate(u8PhyDrvNb, ptbl, 0x07, buf));
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

