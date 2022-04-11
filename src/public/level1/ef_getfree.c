/**
 * ********************************************************************************************************************
 *  @file     ef_getfree.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Get Number of Free Clusters
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
#include <ef_prv_volume_mount.h>
#include "ef_port_diskio.h"
#include "ef_prv_def.h"
#include "ef_prv_directory.h"
#include "ef_prv_dirfunc.h"
#include "ef_prv_fs_window.h"
#include "ef_prv_lock.h"
#include "ef_prv_string.h"
#include "ef_prv_volume.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_unicode.h"
#include "ef_prv_validate.h"
#include "ef_prv_volume_nb.h"
#include "ef_prv_volume.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_getfree (
  const TCHAR * pxPath,
  ef_u32_t    * pu32ClusterNb
)
{
  EF_ASSERT_PUBLIC( 0 != pxPath );
  EF_ASSERT_PUBLIC( 0 != pu32ClusterNb );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st    * pxFS;

  /* Get logical drive, Return ptr to the pxFS object */
  if ( EF_RET_OK != eEFPrvVolumeMountCheck( &pxPath, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    ef_u32_t  u32ClusterCounter;
    ef_u32_t  u32Status;
    ef_lba_t  xSector;
    ef_u32_t  i;

    /* If u32ClstFreeNb is valid, return it without full FAT scan */
    if ( pxFS->u32ClstFreeNb <= ( pxFS->u32FatEntriesNb - 2 ) )
    {
      *pu32ClusterNb = pxFS->u32ClstFreeNb;
    }
    else
    {
      ef_object_st  xObject;
      /* Scan FAT to obtain number of free clusters */
      u32ClusterCounter = 0;

      /* If filesystem is FAT12: Scan bit field FAT entries */
      if ( 0 != ( EF_FS_FAT12 & pxFS->u8FsType ) )
      {
        xObject.pxFS = pxFS;
        /* FAT12: Scan bit field FAT entries */
        for ( ef_u32_t u32Cluster = 2 ; u32Cluster < pxFS->u32FatEntriesNb ; u32Cluster++ )
        {
          if ( EF_RET_OK != eEFPrvFATGet( pxFS, u32Cluster, &u32Status ) )
          {
            eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
            break;
          }
          if ( 0 == u32Status )
          {
            u32ClusterCounter++;
          }
        }
      }
      /* Else FAT16/32: Scan ef_u16_t/ef_u32_t FAT entries */
      else
      {
        ef_object_st  xObject;
        ef_u32_t      clst;
        {
          clst    = pxFS->u32FatEntriesNb;  /* Number of entries */
          xSector = pxFS->xFatBase;         /* Top of the FAT */
          i = 0;          /* Offset in the sector */
          do {  /* Counts number of entries with zero in the FAT */
            if ( 0 == i )
            {
              if ( EF_RET_OK != eEFPrvFSWindowLoad( pxFS, xSector++ ) )
              {
                eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
                break;
              }
            }
            if ( 0 != ( EF_FS_FAT16 & pxFS->u8FsType ) )
            {
              if ( 0 == u16EFPortLoad(pxFS->pu8Window + i) )
              {
                u32ClusterCounter++;
              }
              i += 2;
            }
            else //if ( 0 != ( EF_FS_FAT16 & pxFS->u8FsType ) )
            {
              if ( 0 == (u32EFPortLoad(pxFS->pu8Window + i) & 0x0FFFFFFF) )
              {
                u32ClusterCounter++;
              }
              i += 4;
            }
            i %= EF_SECTOR_SIZE( pxFS );
          } while (--clst);
        }
      }
      /* Return the free clusters */
      *pu32ClusterNb = u32ClusterCounter;
      /* Now u32ClstFreeNb is valid */
      pxFS->u32ClstFreeNb = u32ClusterCounter;
      /* FAT32: FSInfo is to be updated */
      pxFS->u8FsInfoFlags |= 1;
    }
  }

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

