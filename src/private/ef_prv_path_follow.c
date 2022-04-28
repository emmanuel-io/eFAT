/**
 * ********************************************************************************************************************
 *  @file     ef_prv_path_follow.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Code file for functions.
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
#include "ef_prv_create_name.h"
#include "ef_prv_directory.h"
#include "ef_prv_dirfunc.h"
#include "ef_prv_directory.h"
#include "ef_prv_lock.h"
#include "ef_prv_string.h"
#include "ef_prv_volume.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_unicode.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */
/* Follow a file path */
ef_return_et eEFPrvPathFollow (
  const TCHAR     * pxPath,
  ef_directory_st * pxDir,
  ef_return_et    * peResult
)
{
  EF_ASSERT_PRIVATE( 0 != pxPath );
  EF_ASSERT_PRIVATE( 0 != pxDir );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st    * pxFS = pxDir->xObject.pxFS;


  /* Without heading separator */
  if (    ( 0 != EF_CONF_RELATIVE_PATH )
       && ( '/' != *pxPath )
       && ( '\\' != *pxPath ) )
  {
    /* Start from current directory */
    pxDir->xObject.u32ClstStart = pxFS->u32DirClstCurrent;
  }
  else
  {
    /* With heading separator */
    while ( ( '/' == *pxPath ) || ( '\\' == *pxPath ) )
    {
      /* Strip heading separator */
      pxPath++;
    }
    /* Start from root directory */
    pxDir->xObject.u32ClstStart = 0;
  }

  /* Null path name is the origin directory itself */
  if ( (ef_u32_t)*pxPath < ' ' )
  {
    pxDir->u8Name[ EF_NSFLAG ] = EF_NS_NONAME;
    eRetVal = eEFPrvDirectoryIndexSet( pxDir, 0 );
    if ( EF_RET_OK != eRetVal ) { (void) EF_RETURN_CODE_HANDLER( eRetVal );}
  }
  else
  {
    /* Follow path */
    for ( ; ; )
    {
      /* Get a segment name of the pxPath */
      eRetVal = eEFPrvNameCreate( pxDir, &pxPath );
      if ( EF_RET_OK != eRetVal )
      {
        (void) EF_RETURN_CODE_HANDLER( eRetVal );
        break;
      }
      ef_bool_t     bFound = EF_BOOL_FALSE;
      /* Find an object with the segment name */
      eRetVal = eEFPrvDirFind( pxDir, &bFound );
      if ( EF_RET_NO_FILE == eRetVal )
      {
        *peResult = eRetVal;
        EF_CODE_COVERAGE( );
      }
      else if ( EF_RET_OK == eRetVal )
      {
        *peResult = eRetVal;
        EF_CODE_COVERAGE( );
      }
      else
      {
        (void) EF_RETURN_CODE_HANDLER( eRetVal );
      }
      ef_u08_t  ns = pxDir->u8Name[ EF_NSFLAG ];
      /* Failed to find the object */
      if ( EF_RET_NO_FILE == eRetVal )
      {
        /* If dot entry is not exist, stay there */
        if (    ( 0 != EF_CONF_RELATIVE_PATH )
             && ( 0 != ( ns & EF_NS_DOT ) ) )
        {
          /* If not last segment */
          if ( 0 == ( ns & EF_NS_LAST ) )
          {
            /* Continue to follow */
            continue;
          }
          else
          {
            EF_CODE_COVERAGE( );
          }
          pxDir->u8Name[EF_NSFLAG] = EF_NS_NONAME;
          eRetVal = EF_RET_OK;
        }
        else
        { /* Could not find the object */
          /* If not last segment */
          if ( 0 == ( ns & EF_NS_LAST ) )
          {
            /* Adjust error code */
            eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_PATH );
          }
          else
          {
            EF_CODE_COVERAGE( );
          }
        }
        break;
      } /* Object is not found */
      else if ( EF_RET_OK != eRetVal )
      {
        break;
      }
      /* Else, if it is last segment */
      else if ( 0 != ( ns & EF_NS_LAST ) )
      {
        /* Function completed. */
        break;
      }
      /* Else, if it is not a sub-directory */
      else if ( 0 == ( pxDir->xObject.u8Attrib & EF_DIR_ATTRIB_BIT_DIRECTORY ) )
      {
        /* It is not a sub-directory and cannot follow */
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_PATH );
        break;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
      /* Open next directory */
//      pxDir->xObject.u32ClstStart = eEFPrvDirectoryClusterGet( pxFS,
//                                                              pxFS->pu8Window
//                                                              + pxDir->u32Offset % EF_SECTOR_SIZE( pxFS ) );
       (void) eEFPrvDirectoryClusterGet(  pxFS,
                                          pxFS->pu8Window + pxDir->u32Offset % EF_SECTOR_SIZE( pxFS ),
                                          &(pxDir->xObject.u32ClstStart) );
    } /* Follow path */
  }

  return eRetVal;
} /* eEFPrvPathFollow */


/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

