/**
 * ********************************************************************************************************************
 *  @file     ef_prv_fs_lock.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Filesystem locking / unlocking.
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

/* Public functions ------------------------------------------------------------------------------------------------ */

/* Request/Release grant to access the volume */
ef_return_et eEFPrvFSLock (
  ef_fs_st *  pxFS
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_OK;

  /* If    There is no file system locking mechanism
   *    OR Sync object take is ok */
  if ( 0 == EF_CONF_FS_LOCK )
  {
    EF_CODE_COVERAGE( );
  }
  else if ( EF_RET_OK == eEFPortSyncObjectTake( pxFS->xSyncObject ) )
  {
    EF_CODE_COVERAGE( );
  }
  else
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }

  return eRetVal;
}

/* Request/Release grant to access the volume */
ef_return_et eEFPrvFSUnlock (
  ef_fs_st      * pxFS,
  ef_return_et    eResult
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_OK;

  /* If there is no file system locking mechanism */
  if ( 0 == EF_CONF_FS_LOCK )
  {
    EF_CODE_COVERAGE( );
  }
  else if ( EF_RET_NOT_ENABLED == eResult )
  {
    EF_CODE_COVERAGE( );
  }
  else if ( EF_RET_INVALID_DRIVE == eResult )
  {
    EF_CODE_COVERAGE( );
  }
  else if ( EF_RET_TIMEOUT == eResult )
  {
    EF_CODE_COVERAGE( );
  }
  else if ( EF_RET_OK == eEFPortSyncObjectGive( pxFS->xSyncObject ) )
  {
    EF_CODE_COVERAGE( );
  }
  else
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }

  return eRetVal;
}

ef_return_et eEFPrvFSUnlockForce (
  ef_fs_st  * pxFS
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_OK;

  /* If there is no file system locking mechanism */
  if ( 0 == EF_CONF_FS_LOCK )
  {
    EF_CODE_COVERAGE( );
  }
  else if ( EF_RET_OK == eEFPortSyncObjectGive( pxFS->xSyncObject ) )
  {
    EF_CODE_COVERAGE( );
  }
  else
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
