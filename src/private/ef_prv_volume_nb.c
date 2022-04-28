/**
 * ********************************************************************************************************************
 *  @file     ef_prv_volume_nb.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private volume number management.
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
#include "ef_prv_lock.h"
#include "ef_prv_string.h"
#include "ef_prv_volume.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_unicode.h"
#include "ef_prv_volume_nb.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
#if ( EF_CONF_VOLUMES_NB < EF_CONF_VOLUMES_NB_MIN )
  #error Wrong EF_CONF_VOLUMES_NB setting, too low
#elif ( EF_CONF_VOLUMES_NB > EF_CONF_VOLUMES_NB_MAX )
  #error Wrong EF_CONF_VOLUMES_NB setting, too high
#endif

/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
#if EF_CONF_RELATIVE_PATH != 0
static int8_t s8VolumeCurrentNb;        /* Current drive */
#endif
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */
/* Set current volume number in relative path */
ef_return_et eEFPrvVolumeNbCurrentSet (
  int8_t s8VolumeNb
)
{
  ef_return_et  eRetVal = EF_RET_OK;

  if ( 0 > s8VolumeNb )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else if ( EF_CONF_VOLUMES_NB <= s8VolumeNb )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    /* Volume number */
    s8VolumeCurrentNb = s8VolumeNb;
  }
  return eRetVal;
}

/* Get current volume number in relative path */
ef_return_et eEFPrvVolumeNbCurrentGet (
  int8_t * ps8VolumeNb
)
{
  EF_ASSERT_PRIVATE( 0 != ps8VolumeNb );

  ef_return_et  eRetVal = EF_RET_OK;
  if ( ( 0 <= s8VolumeCurrentNb ) && ( EF_CONF_VOLUMES_NB > s8VolumeCurrentNb ) )
  {
    /* Volume number */
    *ps8VolumeNb = s8VolumeCurrentNb;
  }
  else
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    ; /* Code coverage */
  }
  return eRetVal;
}

/* Get logical drive number from path name */
ef_return_et eEFPrvVolumeNbPathRemove (
  const TCHAR** ppxPath
)
{
  EF_ASSERT_PRIVATE( 0 != ppxPath );
  EF_ASSERT_PRIVATE( 0 != *ppxPath );

  ef_return_et  eRetVal = EF_RET_OK;

  const TCHAR * tp = *ppxPath;
  const TCHAR * tt = *ppxPath;
  TCHAR         tc;
  TCHAR         td;

  if ( 0 != EF_CONF_VFAT )
  {
   td = ' ';
  }
  else
  {
   td = '!';
  }

  do
  {
   tc = *tt++;
  }
  while ( ( (ef_u32_t)tc >= (ef_u32_t)td ) && ( ':' != tc ) );
  /* Find a colon in the path */

  /* DOS/Windows style volume ID? */
  if ( ':' == tc )
  {
   int  i = EF_CONF_VOLUMES_NB;
   /* Is there a letter volume ID + colon? */
   if ( tp + 2 == tt )
   {
     if IsUpper( *tp )
     {
       i = (int)*tp - 'A';  /* Get the volume number */
     }
     else if IsLower( *tp )
     {
       i = (int)*tp - 'a';  /* Get the volume number */
     }
     else
     {
       ; /* Code compliance */
     }
   }
   /* If a volume ID is found, get the drive number and strip it */
   if ( EF_CONF_VOLUMES_NB > i )
   {
     /* Snip the drive prefix off */
     *ppxPath = tt;
   }
  }

  return eRetVal;
}

/* Get filesystem/volume number from path name */
ef_return_et eEFPrvVolumeNbGet (
  const TCHAR **  ppxPath,
  int8_t *        ps8VolumeNb )
{
  EF_ASSERT_PRIVATE( 0 != ppxPath );
  EF_ASSERT_PRIVATE( 0 != *ppxPath );
  EF_ASSERT_PRIVATE( 0 != ps8VolumeNb );

  ef_return_et  eRetVal = EF_RET_OK;
  int8_t        s8VolumeNb = -1;
  const TCHAR * tp = *ppxPath;
  const TCHAR * tt = *ppxPath;
  TCHAR         tc;
  TCHAR         td;

  if ( 0 != EF_CONF_VFAT )
  {
    td = ' ';
  }
  else
  {
    td = '!';
  }

  do
  {
    tc = *tt++;
  }
  while ( ( (ef_u32_t)tc >= (ef_u32_t)td ) && ( ':' != tc ) );
  /* Find a colon in the path */

  /* DOS/Windows style volume ID? */
  if ( ':' == tc )
  {
    int  i = EF_CONF_VOLUMES_NB;
    /* Is there a letter volume ID + colon? */
    if ( ( tp + 2 ) == tt )
    {
      if IsUpper( *tp )
      {
        i = (int)*tp - 'A';  /* Get the volume number */
      }
      else if IsLower( *tp )
      {
        i = (int)*tp - 'a';  /* Get the volume number */
      }
      else
      {
        ; /* Code compliance */
      }
    }
    /* If a volume ID is found, get the drive number and strip it */
    if ( i < EF_CONF_VOLUMES_NB )
    {
      s8VolumeNb  = (int8_t) i; /* Drive number */
      *ppxPath    = tt;         /* Snip the drive prefix off */
    }
  }

  /* No drive prefix is found */
  if ( 0 <= s8VolumeNb )
  {
    /* Update volume number with the one found */
    *ps8VolumeNb = s8VolumeNb;
  }
  /* Else if relative path is enabled */
  else if ( 0 != EF_CONF_RELATIVE_PATH )
  {
    /* Update volume number with the current one */
    if ( EF_RET_OK != eEFPrvVolumeNbCurrentGet( ps8VolumeNb ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
  }
  else
  {
    /* Update volume number with the Default which is 0 */
    *ps8VolumeNb = 0;
  }

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
