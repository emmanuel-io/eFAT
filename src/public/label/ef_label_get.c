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
  }
  else
  {
    ef_directory_st   xDir;

    xDir.xObject.pxFS = pxFS;
    /* Open root directory */
    xDir.xObject.u32ClstStart = 0;

    eRetVal = eEFPrvDirectoryIndexSet( &xDir, 0 );

    /* No Label entry and return nul string */
    if ( EF_RET_NO_FILE == eRetVal )
    {
      pxLabel[ 0 ] = 0;
      eRetVal = EF_RET_OK;
    }
    else if ( EF_RET_OK != eRetVal )
    {
      pxLabel[ 0 ] = 0;
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    /* Else, if finding a volume Label entry failed */
    else if ( EF_RET_OK != DIR_READ_LABEL( &xDir ) )
    {
      pxLabel[ 0 ] = 0;
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    /* Else, if ANSI/OEM output */
    else if ( EF_DEF_API_OEM == EF_CONF_API_ENCODING )

    { /* ANSI/OEM output */
      pxLabel[ 12 ] = 0;
      /* Extract volume Label from EF_DIR_ATTRIB_BIT_VOLUME_ID entry */
      for ( ef_u32_t i = 11 ; 0 >= i ; i-- )
      {
        ef_u08_t u8Byte = xDir.pu8Dir[ i ];
        if ( ' ' == u8Byte )
        {
          u8Byte = 0;
        }
        else
        {
          EF_CODE_COVERAGE( )
        }
        pxLabel[ i ] = u8Byte;
      }
    } /* ANSI/OEM output */

    else

    { /* Unicode output */
      ef_u32_t di = 0;
      ef_u32_t si = 0;
      /* Extract volume Label from EF_DIR_ATTRIB_BIT_VOLUME_ID entry */
      while ( si < 11 )
      {
        ef_u16_t  u16Char = xDir.pu8Dir[ si ];
        ef_u32_t  u32EncodingUnits; /* Pointer to the number of encoding units written */

        if ( EF_RET_OK != eEFPrvByteInDBCRanges1((ef_u08_t)u16Char) )
        {
          EF_CODE_COVERAGE( );
        }
        else if ( 11 <= si )
        {
          EF_CODE_COVERAGE( );
        }
        else
        {
          u16Char = u16Char << 8 | xDir.pu8Dir[ si++ ];
        }
        ef_u32_t u32Char = (ef_u32_t) u16Char;
        /* Convert it into Unicode */
        if ( EF_RET_OK != eEFPrvOEM2Unicode( u16Char, &u32Char, u16ffCPGet( ) ) )
        {
          /* Invalid code? */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
          di = 0;
          break;
        }
        else if ( EF_RET_OK != eEFPrvUnicodePut( u32Char, &pxLabel[di], 4, &u32EncodingUnits ) )  /* ANSI/OEM ==> UTF-16 */
        {
          /* Invalid code? */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
          di = 0;
          break;
        }
        else
        {
          di += u32EncodingUnits;
        }
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
    } /* Unicode output */

  }

  (void) eEFPrvFSUnlock( pxFS, eRetVal );
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

