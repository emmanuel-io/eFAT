/**
 * ********************************************************************************************************************
 *  @file     ef_label_get.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Get Volume Label
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
#include "ef_prv_dir_label.h"
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
#include "ef_prv_def_bpb_fat.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_label_get (
  const TCHAR * pxPath,
  TCHAR       * pxLabel
)
{
  EF_ASSERT_PUBLIC( 0 != pxPath );
  EF_ASSERT_PUBLIC( 0 != pxLabel );

  ef_return_et    eRetVal = EF_RET_OK;
  ef_fs_st      * pxFS;

  /* Get logical drive */
  if ( EF_RET_OK != eEFPrvVolumeMountCheck( &pxPath, &pxFS ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    EF_CODE_COVERAGE( );
  }
  else
  {
    ef_directory_st   xDir;

    xDir.xObject.pxFS = pxFS;
    /* Open root directory */
    xDir.xObject.u32ClstStart = 0;

    eRetVal = eEFPrvDirectoryIndexSet( &xDir, 0 );
    if ( EF_RET_OK == eRetVal )
    {
      ef_u32_t di = 0;
      /* Find a volume Label entry */
      eRetVal = DIR_READ_LABEL( &xDir );
      if ( EF_RET_OK == eRetVal )
      {
        ef_u32_t si = 0;
        /* Extract volume pxLabel from EF_DIR_ATTRIB_BIT_VOLUME_ID entry */
        while ( si < 11 )
        {
          ucs2_t  u16Char = xDir.pu8Dir[ si++ ];
          /* If Unicode output */
          if ( EF_DEF_API_OEM != EF_CONF_API_ENCODING )
          { /* Unicode output */
            /* If it is a DBC? */
            if (    ( EF_RET_OK == eEFPrvCharInDBCRangesByte1((ef_u08_t)u16Char) )
                 && ( si < 11 ) )
            {
              u16Char = u16Char << 8 | xDir.pu8Dir[ si++ ];
            }
            /* Convert it into Unicode */
            u16Char = ef_oem2uni( u16Char, u16ffCPGet( ) );
            /* Put it in Unicode */
            if ( 0 != u16Char )
            {
              u16Char = put_utf( u16Char, &pxLabel[di], 4 );
            }
            if ( 0 == u16Char )
            {
              di = 0;
              break;
            }
            di += u16Char;
          } /* Unicode output */
          else
          { /* ANSI/OEM output */
            pxLabel[ di++ ] = (TCHAR) u16Char;
          } /* ANSI/OEM output */
        }
        /* Truncate trailing spaces */
        do
        {
          pxLabel[ di ] = 0;
          if ( 0 == di )
          {
            break;
          }
        } while ( ' ' == pxLabel[--di] );
      }
    }
    /* No Label entry and return nul string */
    else if ( EF_RET_NO_FILE == eRetVal )
    {
      pxLabel[ 0 ] = 0;
      eRetVal = EF_RET_OK;
    }
    else
    {
      pxLabel[ 0 ] = 0;
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
  }

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

