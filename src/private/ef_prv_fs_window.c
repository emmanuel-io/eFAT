/**
 * ********************************************************************************************************************
 *  @file     ef_prv_fs_window.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    FS window load or Store.
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
#include "ef_prv_fat.h"
#include "ef_prv_drive.h"
#include "ef_prv_def.h"
#include "ef_prv_directory.h"
#include "ef_prv_dirfunc.h"
#include "ef_prv_lock.h"
#include "ef_prv_string.h"
#include "ef_prv_volume.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_unicode.h"
#include "ef_prv_def_bpb_fat.h"
#include <ef_port_load_store.h>
#include <ef_port_memory.h>

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/* Flush disk access window in the filesystem object */
ef_return_et eEFPrvFSWindowStore (
  ef_fs_st *  pxFS
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_OK;

  /* If the disk access window is clean */
  if ( 0 == ( EF_FS_WIN_DIRTY & pxFS->u8WinFlags ) )
  {
    EF_CODE_COVERAGE( );
  }
  /* Else, if writing the window back into the volume failed */
  else if ( EF_RET_OK !=  eEFPrvDriveWrite( pxFS->u8PhysDrv,
                                            pxFS->pu8Window,
                                            pxFS->xWindowSector,
                                            1 ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
  }
  else
  {
    /* Clear window dirty status flag */
    pxFS->u8WinFlags &= (ef_u08_t) ~EF_FS_WIN_DIRTY;

    /* If it is not the 1st FAT */
    if ( ( pxFS->xWindowSector - pxFS->xFatBase ) >= pxFS->u32FatSize )
    {
      EF_CODE_COVERAGE( );
    }
    /* Else, if a 2nd FAT is not needed */
    else if ( 2 != pxFS->u8FatsNb )
    {
      EF_CODE_COVERAGE( );
    }
    /* Else, if Reflecting it to 2nd FAT failed */
    else if ( EF_RET_OK !=  eEFPrvDriveWrite(  pxFS->u8PhysDrv,
                                               pxFS->pu8Window,
                                               pxFS->xWindowSector + pxFS->u32FatSize,
                                               1 ) )
    {
      /* Nothing because it's a backup, if it fails not a problem ! */
      EF_CODE_COVERAGE( );
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
  }

  return eRetVal;
}


/* Move + Flush disk access window in the filesystem object */
ef_return_et eEFPrvFSWindowLoad (
  ef_fs_st  * pxFS,
  ef_lba_t    xSector
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_OK;

  /* If window offset is the same */
  if ( xSector == pxFS->xWindowSector )
  {
    EF_CODE_COVERAGE( );
  }
  /* Else, if Flushing the window failed */
  else if ( EF_RET_OK !=  eEFPrvFSWindowStore( pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  /* Else, if reloading the window with new data failed */
  else if ( EF_RET_OK != eEFPrvDriveRead(  pxFS->u8PhysDrv,
                                            pxFS->pu8Window,
                                            xSector,
                                            1 ) )
  {
    /* Invalidate window if read data is not valid */
    pxFS->xWindowSector = (ef_lba_t)0 - 1;
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
  }
  else
  {
    pxFS->xWindowSector = xSector;
  }

  return eRetVal;
}

/* Synchronize filesystem and data on the storage */
ef_return_et eEFPrvFSSync (
  ef_fs_st *  pxFS
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et eRetVal = EF_RET_OK;

  if ( EF_RET_OK != eEFPrvFSWindowStore( pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
  }
  /* Else, if not FAT32 */
  else if ( 0 == ( EF_FS_FAT32 & pxFS->u8FsType ) )
  {
    EF_CODE_COVERAGE( );
  }
  /* Else, if updating FSInfo sector is not needed */
  else if ( 0x01 != pxFS->u8FsInfoFlags )
  {
    EF_CODE_COVERAGE( );
  }
  else
  {
    /* Create FSInfo structure */
    eEFPortMemZero( pxFS->pu8Window, pxFS->u32WinSize );
    vEFPortStoreu16(  pxFS->pu8Window + EF_BS_OFFSET_SIGNATURE, 0xAA55 );
    vEFPortStoreu32(  pxFS->pu8Window + EF_BS_FAT32_FSI_OFFSET_SIGNATURE_LEAD, 0x41615252 );
    vEFPortStoreu32(  pxFS->pu8Window + EF_BS_FAT32_FSI_OFFSET_SIGNATURE_NEXT, 0x61417272 );
    vEFPortStoreu32(  pxFS->pu8Window + EF_BS_FAT32_FSI_OFFSET_FREE_CLUSTERS, pxFS->u32ClstFreeNb );
    vEFPortStoreu32(  pxFS->pu8Window + EF_BS_FAT32_FSI_OFFSET_CLUSTER_LAST_ALLOC, pxFS->u32ClstLast );
    /* Write it into the FSInfo sector */
    pxFS->xWindowSector = pxFS->xVolBase + 1;
    (void) eEFPrvDriveWrite( pxFS->u8PhysDrv, pxFS->pu8Window, pxFS->xWindowSector, 1 );
    pxFS->u8FsInfoFlags = 0;
  }

  /* Make sure that no pending write process in the lower layer */
  if ( EF_RET_OK != eEFPrvDriveIOCtrl( pxFS->u8PhysDrv, CTRL_SYNC, 0 ) )
//  eRetVal = eEFPrvDriveIOCtrl( pxFS->u8PhysDrv, CTRL_SYNC, 0 );
//  if ( EF_RET_OK != eRetVal )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
  }
  else
  {
    EF_CODE_COVERAGE( );
  }


  return eRetVal;
}


/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

