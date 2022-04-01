/**
 * ********************************************************************************************************************
 *  @file     ef_prv_drive.c
 *  @ingroup  group_eFAT_Private
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Code file for Low level disk interface.
 *
 *            If a working storage control module is available, it should be
 *            attached to the eFAT via a glue function rather than modifying it.
 *            This is an example of glue functions to attach various existing
 *            storage control modules to the eFAT module with a defined API.
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
#include "ef_prv_def.h"
#include "ef_port_diskio.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */

#if ( ( EF_CONF_DRIVERS_NB < 1 ) || ( EF_CONF_DRIVERS_NB > 128 ) )
#error Wrong EF_CONF_DRIVERS_NB setting
#endif

/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */

/**
 *  Filesystem objects (logical drives)
 */
static ef_u08_t u8FarFsDrivesNb = 0;

/**
 *  Filesystem objects (logical drives)
 */
static ef_drive_functions_st xFarFsDrives[ EF_CONF_DRIVERS_NB ] = { { 0, 0, 0, 0, 0 } };

/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/* Initialize a Drive */
ef_return_et  eEFPrvDriveInitialize (
  ef_u08_t u8PhyDrvNb
)
{
  return xFarFsDrives[ u8PhyDrvNb ].pxInitialize( );
}

/* Get Drive Status */
ef_return_et  eEFPrvDriveStatus (
  ef_u08_t u8PhyDrvNb
)
{
  return xFarFsDrives[ u8PhyDrvNb ].pxStatus( );
}

/* Read Sector(s)*/
ef_return_et  eEFPrvDriveRead (
  ef_u08_t    u8PhyDrvNb,
  ef_u08_t  * pu8Buffer,
  ef_lba_t    xSector,
  ef_u32_t    u32Count
)
{
//  EF_ASSERT_PRIVATE( EF_CONF_DRIVERS_NB <= u8PhyDrvNb );
  EF_ASSERT_PRIVATE( 0 != pu8Buffer );

  return xFarFsDrives[ u8PhyDrvNb ].pxRead( pu8Buffer, xSector, u32Count );
}

/* Write Sector(s) */
ef_return_et  eEFPrvDriveWrite (
  ef_u08_t          u8PhyDrvNb,
  const ef_u08_t  * pu8Buffer,
  ef_lba_t          xSector,
  ef_u32_t          u32Count
)
{
//  EF_ASSERT_PRIVATE( EF_CONF_DRIVERS_NB <= u8PhyDrvNb );
  EF_ASSERT_PRIVATE( 0 != pu8Buffer );

  return xFarFsDrives[ u8PhyDrvNb ].pxWrite( pu8Buffer, xSector, u32Count );
}

/* Miscellaneous Functions */
ef_return_et  eEFPrvDriveIOCtrl (
  ef_u08_t    u8PhyDrvNb,
  ef_u08_t    u8Cmd,
  void      * pvBuffer
)
{
//  EF_ASSERT_PRIVATE( EF_CONF_DRIVERS_NB <= u8PhyDrvNb );

  /* Assertion of non null buffer delegated to driver function
   * as it might be null if not needed
   */
  //  EF_ASSERT_PRIVATE( 0 != pvBuffer );

  return xFarFsDrives[ u8PhyDrvNb ].pxCtrl( u8Cmd, pvBuffer );
}

/* Register a Drive */
ef_return_et eEFPrvDriveRegister (
  ef_drive_functions_st * pxDriveFunctions
)
{
  EF_ASSERT_PRIVATE( 0 != pxDriveFunctions );

  ef_return_et eRetVal = EF_RET_OK;

  if ( EF_CONF_DRIVERS_NB > u8FarFsDrivesNb )
  {
    /* Register function to Initialize Drive */
    xFarFsDrives[ u8FarFsDrivesNb ].pxInitialize  = pxDriveFunctions->pxInitialize;
    /* Register function to Get Disk Status */
    xFarFsDrives[ u8FarFsDrivesNb ].pxStatus      = pxDriveFunctions->pxStatus;
    /* Register function to Read Sector(s) */
    xFarFsDrives[ u8FarFsDrivesNb ].pxRead        = pxDriveFunctions->pxRead;
    /* Register function to Write Sector(s) */
    xFarFsDrives[ u8FarFsDrivesNb ].pxWrite       = pxDriveFunctions->pxWrite;
    /* Register function to I/O control operation */
    xFarFsDrives[ u8FarFsDrivesNb ].pxCtrl        = pxDriveFunctions->pxCtrl;
  }
  else
  {
    eRetVal = EF_RET_INVALID_PARAMETER;
  }

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
