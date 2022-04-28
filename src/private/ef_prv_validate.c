/**
 * ********************************************************************************************************************
 *  @file     ef_prv_validate.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Check if the file/directory object is valid and lock the filesystem if it is valid.
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

#include <ef_port_load_store.h>
#include <efat.h>
#include <ef_prv_def.h>
#include <ef_prv_fat.h>
#include "ef_prv_def.h"
#include "ef_prv_directory.h"
#include "ef_prv_dirfunc.h"
#include "ef_prv_lock.h"
#include "ef_prv_string.h"
#include "ef_prv_volume.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_unicode.h"
#include "ef_prv_drive.h"

/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEFPrvValidateObject (
  ef_object_st   * pxObject,
  ef_fs_st      ** ppxFS
)
{
  EF_ASSERT_PRIVATE( 0 != pxObject );
  EF_ASSERT_PRIVATE( 0 != ppxFS );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st    * pxFS    = 0;

  /* If the object file system pointer is not valid */
  if ( 0 == pxObject->pxFS )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_OBJECT );
  }
  /* Else, if the file system cannot be handled */
  else if ( 0 == ( EF_FS_FATS & pxObject->pxFS->u8FsType ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_OBJECT );
  }
  /* Else, if mount id of the object is not the same as the file system */
  else if ( pxObject->u16MountId != pxObject->pxFS->u16MountId )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_OBJECT );
  }
  /* Else, if we cannot obtain the file system object */
  else if ( EF_RET_OK != eEFPrvFSLock( pxObject->pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_TIMEOUT );
  }
  /* Else, if the physical drive is not kept initialized */
  else if ( 0 != ( EF_RET_DISK_NOINIT & eEFPrvDriveStatus( pxObject->pxFS->u8PhysDrv ) ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_OBJECT );
    (void) eEFPrvFSUnlockForce( pxObject->pxFS );
  }
  else
  {
    /* Assign corresponding file system object */
    pxFS = pxObject->pxFS;
  }
  /* Assign file system object */
  *ppxFS = pxFS;

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
