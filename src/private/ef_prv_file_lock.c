/**
 * ********************************************************************************************************************
 *  @file     ef_prv_file_lock.c
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

#include <efat.h>
#include "ef_prv_def.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */

/**
 *  File lock control structure
 */
typedef struct ef_flock_struct {
  ef_fs_st  * pxFS;       /**< Object ID 1, volume (NULL:blank entry) */
  ef_u32_t    u32Clst;    /**< Object ID 2, containing directory (0:root) */
  ef_u32_t    u32Offset;  /**< Object ID 3, offset in the directory */
  ef_u16_t    u16Cnt;     /**< Object open counter, 0:none, 0x01..0xFF:read mode open count, 0x100:write mode */
} ef_flock_st;

/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */

#if 0 != EF_CONF_FILE_LOCK
/**
 *  Opened files object locking table
 */
static  ef_flock_st Files[ EF_CONF_FILE_LOCK ];
#else
extern  ef_flock_st Files[ 1 ];
#endif

/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/*-----------------------------------------------------------------------*/
/* File lock control functions                                           */
/*-----------------------------------------------------------------------*/
/* Check if the file can be accessed */
ef_return_et eEFPrvLockCheck (
  ef_directory_st * pxDir,
  int acc
)
{
  EF_ASSERT_PRIVATE( 0 != pxDir );

  ef_return_et  eRetVal = EF_RET_OK;

  if ( 0 == EF_CONF_FILE_LOCK )
  {
    EF_CODE_COVERAGE( );
  }
  else
  {
    eRetVal = EF_RET_ERROR;

    ef_u32_t  i;
    ef_u32_t  be = 0;

    /* Search open object table for the object */
    be = 0;
    for ( i = 0 ; i < EF_CONF_FILE_LOCK ; i++ )
    {
      /* Existing entry */
      if ( 0 != Files[ i ].pxFS )
      {
        /* Check if the object matches with an open object */
        if (    ( Files[ i ].pxFS       == pxDir->xObject.pxFS )
             && ( Files[ i ].u32Clst    == pxDir->xObject.u32ClstStart )
             && ( Files[ i ].u32Offset  == pxDir->u32Offset ) )
        {
          break;
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
      }
      else
      {      /* Blank entry */
        be = 1;
      }
    }
    if ( EF_CONF_FILE_LOCK == i )
    {
      /* The object has not been opened */
      /* Is there a blank entry for new object? */
      if (    ( 0 == be )
           && ( 2 != acc ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_TOO_MANY_OPEN_FILES );
      }
      else
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_OK );
      }
    }
    else
    {
      /* The object was opened. Reject any open against writing file and all write mode open */
      if (    ( 0 != acc )
           || ( 0x100 != Files[ i ].u16Cnt ) )
      {
        /* Reject any open against writing file and all write mode open */
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_LOCKED );
      }
      else
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_OK );
      }
    }
  }

  return eRetVal;
}

/* Check if an entry is available for a new object */
ef_return_et eEFPrvLockEnq (void)
{
  ef_return_et eRetVal = EF_RET_OK;

  /* If there is no file locking mechanism */
  if ( 0 == EF_CONF_FILE_LOCK )
  {
    EF_CODE_COVERAGE( );
  }
  else
  {
    eRetVal = EF_RET_LOCKED;
    for ( ef_u32_t i = 0 ; i < EF_CONF_FILE_LOCK ; i++ )
    {
      if ( 0 == Files[ i ].pxFS )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_OK );
        break;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
    }
  }
  return eRetVal;
}

/* Increment object open counter and returns its index (0:Internal error) */
ef_return_et eEFPrvLockInc (
  ef_directory_st * pxDir,
  int               iAccess,
  ef_u32_t        * pu32LockId
)
{
  EF_ASSERT_PRIVATE( 0 != pxDir );

  ef_return_et  eRetVal = EF_RET_OK;

  /* If there is no file locking mechanism */
  if ( 0 == EF_CONF_FILE_LOCK )
  {
    EF_CODE_COVERAGE( );
  }
  else
  {
    eRetVal = EF_RET_ERROR;
    ef_u32_t i;

    /* Find the object */
    for ( i = 0; i < EF_CONF_FILE_LOCK; i++ )
    {
      if (    ( Files[ i ].pxFS       == pxDir->xObject.pxFS )
           && ( Files[ i ].u32Clst    == pxDir->xObject.u32ClstStart )
           && ( Files[ i ].u32Offset  == pxDir->u32Offset ) )
      {
        break;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
    }

    /* Not opened. Register it as new. */
    if ( EF_CONF_FILE_LOCK == i )
    {
      for ( i = 0 ; i < EF_CONF_FILE_LOCK ; i++ )
      {
        if ( 0 == Files[ i ].pxFS )
        {
          break;
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
      }
      /* No free entry to register (int u8ErrorCode) */
      if ( EF_CONF_FILE_LOCK == i )
      {
        *pu32LockId = 0;
        return EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
      Files[ i ].pxFS       = pxDir->xObject.pxFS;
      Files[ i ].u32Clst    = pxDir->xObject.u32ClstStart;
      Files[ i ].u32Offset  = pxDir->u32Offset;
      Files[ i ].u16Cnt     = 0;
    }

    if (    ( 1 <= iAccess )
         && ( 0 != Files[ i ].u16Cnt ) )
    {
      *pu32LockId = 0;  /* Access violation (int u8ErrorCode) */
      return EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
    else
    {
      EF_CODE_COVERAGE( );
    }

    if ( 0 != iAccess )
    {
      /* Set semaphore value */
      Files[ i ].u16Cnt = 0x100;
    }
    else
    {
      /* Set semaphore value */
      Files[ i ].u16Cnt = Files[ i ].u16Cnt + 1;
    }

    *pu32LockId = i + 1;
  }
  return eRetVal;
}

/* Decrement object open counter */
ef_return_et eEFPrvLockDec (
  ef_u32_t  i
)
{
  /* Invalid index nunber */
  ef_return_et eRetVal = EF_RET_OK;

  /* If there is no file locking mechanism */
  if ( 0 == EF_CONF_FILE_LOCK )
  {
    EF_CODE_COVERAGE( );
  }
  else
  {
    eRetVal = EF_RET_INT_ERR;
    ef_u16_t n;

    /* Index number origin from 0 */
    if ( --i < EF_CONF_FILE_LOCK )
    {
      n = Files[ i ].u16Cnt;
      /* If write u8Mode open, delete the entry */
      if ( 0x100 == n )
      {
        n = 0;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
      /* Decrement read u8Mode open count */
      if ( 0 < n )
      {
        n--;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
      Files[ i ].u16Cnt = n;
      /* Delete the entry if open count gets zero */
      if ( 0 == n )
      {
        Files[ i ].pxFS = 0;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_OK );
    }
  }

  return eRetVal;
}

/* Clear lock entries of the volume */
ef_return_et eEFPrvLockClear (
  ef_fs_st *  pxFS
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_OK;

  /* If there is no file locking mechanism */
  if ( 0 == EF_CONF_FILE_LOCK )
  {
    EF_CODE_COVERAGE( );
  }
  else
  {
    eRetVal = EF_RET_INT_ERR;

    for ( ef_u32_t i = 0 ; i < EF_CONF_FILE_LOCK ; i++ )
    {
      if ( Files[ i ].pxFS == pxFS )
      {
        Files[ i ].pxFS = 0;
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_OK );
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
    }
  }

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
