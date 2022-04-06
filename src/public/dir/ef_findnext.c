/**
 * ********************************************************************************************************************
 *  @file     ef_findnext.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Find Next File
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
#include <ef_prv_def.h>
#include "ef_prv_pattern_matching.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_findnext (
  EF_DIR * pxDir,
  ef_file_info_st * pxFileInfo
)
{
  EF_ASSERT_PUBLIC( 0 != pxDir );
  EF_ASSERT_PUBLIC( 0 != pxFileInfo );

  ef_return_et eRetVal;

  for ( ; ; )
  {
    /* Get a directory item */
    eRetVal = eEF_dirread( pxDir, pxFileInfo );
    /* If    any error
     *    OR end of directory */
    if (    ( EF_RET_OK != eRetVal )
         || ( 0 == pxFileInfo )
         || ( 0 == pxFileInfo->xName[ 0 ] ) )
    {
      break;
    }
    /* If Pattern matching failed */
    if ( EF_RET_OK != eEFPrvPatternMatching( pxDir->pxPattern, pxFileInfo->xName, 0, 0 ) )
    {
      break;
    }
    /* If     VFAT is enabled
     *    AND Finding on alternate file name is enabled
          AND Pattern matching failed */
    if (    ( 0 != EF_CONF_VFAT )
         && ( 2 == EF_CONF_USE_FIND )
         && ( EF_RET_OK != eEFPrvPatternMatching( pxDir->pxPattern, pxFileInfo->xNameAlt, 0, 0 ) ) )
    {
      break;  /* Test for alternative name if exist */
    }
  }
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
