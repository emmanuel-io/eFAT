/**
 * ********************************************************************************************************************
 *  @file     ef_mount.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Mount/Unmount a Logical Drive
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
#include "ef_port_diskio.h"
#include "ef_prv_def.h"
#include "ef_prv_directory.h"
#include "ef_prv_dirfunc.h"
#include "ef_prv_fs_window.h"
#include "ef_prv_lock.h"
#include "ef_prv_string.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_unicode.h"
#include "ef_prv_validate.h"
#include <ef_prv_volume_mount.h>
#include "ef_prv_volume_nb.h"
#include "ef_prv_volume.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */

/**
 *  Windows for filesystem access
 */
//typedef struct fs_window_t{
//  ef_u08_t u8Window[ EF_CONF_SECTOR_SIZE ];
//} fs_window_t;

/* Local variables ------------------------------------------------------------------------------------------------- */

/**
 *  Window for filesystem access 32-Byte aligned for cache maintenance
 */
ef_u08_t xeFATWindows[ EF_CONF_VOLUMES_NB * EF_CONF_SECTOR_SIZE ] __attribute__ ((aligned (32)));
//fs_window_t xeFATWindows[ EF_CONF_VOLUMES_NB * EF_CONF_SS_MAX ];

/**
 *  Filesystem objects (logical drives)
 */
ef_fs_st xeFAT[ EF_CONF_VOLUMES_NB ];

/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_mount (
  const TCHAR * pxPath,
  ef_u08_t     u8PhysDrvNb,
  ef_u08_t     u8PartitionNb,
  ef_u08_t     u8ReadOnly

)
{
  EF_ASSERT_PUBLIC( 0 != pxPath );

  ef_return_et  eRetVal = EF_RET_INVALID_DRIVE;
  const TCHAR * rp = pxPath;
  int8_t        s8VolumeNb = -1;
  ef_fs_st    * pxFS = 0;

  if ( 0 == pxPath )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_PARAMETER );
  }
  /* Get logical drive number */
  else if ( EF_RET_OK != eEFPrvVolumeNbGet( &rp, &s8VolumeNb ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_DRIVE );
  }
  else if ( s8VolumeNb < 0 )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_PARAMETER );
  }
  /* else if Drive Letter indicates already mounted */
  else if ( EF_RET_OK != eEFPrvVolumeFSPtrGet( s8VolumeNb, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_PARAMETER );
  }
  else if ( 0 != pxFS )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_EXIST );
  }
  /* Create sync object for the new volume */
  else if ( EF_RET_OK != eEFPortSyncObjectCreate( (ef_u08_t)s8VolumeNb, &(xeFAT[ s8VolumeNb ].xSyncObject) ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  /* Lock the volume */
  else if ( EF_RET_OK != eEFPrvFSLock( &xeFAT[ s8VolumeNb ] ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_TIMEOUT );
  }
  /* Else Try mounting */
  else
  {
//    ef_u08_t * pu8pointer = 0xffff;
    xeFAT[ s8VolumeNb ].u8PhysDrv    = u8PhysDrvNb;
    xeFAT[ s8VolumeNb ].u8Partition  = u8PartitionNb;
//    xeFAT[ s8VolumeNb ].pu8Window    = (ef_u08_t *) &(xeFATWindows[ s8VolumeNb ].u8Window[ 0 ]);
//    xeFAT[ s8VolumeNb ].pu8Window    = &xeFATWindows[ s8VolumeNb ];
//    pu8pointer    = (ef_u08_t *) &(xeFATWindows[ s8VolumeNb ].u8Window[ 0 ]);
//    xeFAT[ s8VolumeNb ].pu8Window    = pu8pointer;
//    pu8pointer    = &xeFATWindows[ s8VolumeNb * EF_CONF_SS_MAX ];
    xeFAT[ s8VolumeNb ].pu8Window    = &xeFATWindows[ s8VolumeNb * EF_CONF_SECTOR_SIZE ];
//    eRetVal = eEFPrvVolumeMount( &pxPath, &pxFS, u8ReadOnly );
    eRetVal = eEFPrvVolumeMount( &xeFAT[ s8VolumeNb ], u8ReadOnly );
    /* if mounting the volume failed */
    if ( EF_RET_OK != eRetVal )
    {
      (void) eEFPrvFSUnlockForce( &xeFAT[ s8VolumeNb ] );
      /* Discard sync object of the current volume */
      if ( EF_RET_OK != eEFPortSyncObjectDelete( xeFAT[ s8VolumeNb ].xSyncObject ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
      }
      else
      {
        eRetVal = EF_RETURN_CODE_HANDLER( eRetVal );
      }
    }
    else if ( EF_RET_OK != eEFPrvVolumeFSPtrSet( s8VolumeNb, &xeFAT[ s8VolumeNb ] ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_PARAMETER );
    }
    else if ( EF_RET_OK != eEFPrvFSUnlock( &xeFAT[ s8VolumeNb ], eRetVal ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_TIMEOUT );
    }
    else
    {
      eRetVal = EF_RET_OK;
    }
  }

  return eRetVal;
}

/* Unmount a Logical Drive */
ef_return_et eEF_umount (
  const TCHAR * pxPath
)
{
  EF_ASSERT_PRIVATE( 0 != pxPath );

  ef_return_et    eRetVal = EF_RET_OK;
  const TCHAR   * rp = pxPath;
  int8_t          s8VolumeNb = -1;
  ef_fs_st      * pxFS = 0;

  if ( 0 == pxPath )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_PARAMETER );
  }
  /* Get logical drive number */
  else if ( EF_RET_OK != eEFPrvVolumeNbGet( &rp, &s8VolumeNb ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_DRIVE );
  }
  else if ( s8VolumeNb < 0 )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_PARAMETER );
  }
  /* else if Drive Letter indicates already unmounted */
  else if ( EF_RET_OK != eEFPrvVolumeFSPtrGet( s8VolumeNb, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_PARAMETER );
  }
  else if ( 0 == pxFS )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_EXIST );
  }
  /* Unlock filesystem */
  else if ( EF_RET_OK !=  eEFPrvLockClear( &xeFAT[ s8VolumeNb ] ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  /* Clear old pxFS object */
  else if ( EF_RET_OK != eEFPrvVolumeFSPtrSet( s8VolumeNb, 0 ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_PARAMETER );
  }
  /* Discard sync object of the current volume */
  else if ( EF_RET_OK != eEFPortSyncObjectDelete( pxFS->xSyncObject ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  else
  {
    EF_CODE_COVERAGE( );
  }

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

