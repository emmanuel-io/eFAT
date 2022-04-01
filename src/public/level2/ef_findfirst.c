/**
 * ********************************************************************************************************************
 *  @file     ef_findfirst.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Find First File
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

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_findfirst (
  EF_DIR * pxDir,
  ef_file_info_st * pxFileInfo,
  const TCHAR     * pxPath,
  const TCHAR     * pxPattern
)
{
  EF_ASSERT_PUBLIC( 0 != pxDir );
  EF_ASSERT_PUBLIC( 0 != pxFileInfo );
  EF_ASSERT_PUBLIC( 0 != pxPath );
  EF_ASSERT_PUBLIC( 0 != pxPattern );

  /* Save pointer to pattern string */
  pxDir->pxPattern = pxPattern;
  /* Open the target directory */
  ef_return_et eRetVal = eEF_diropen( pxDir, pxPath );
  if ( EF_RET_OK == eRetVal )
  {
    /* Find the first item */
    eRetVal = eEF_findnext( pxDir, pxFileInfo );
  }
  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
