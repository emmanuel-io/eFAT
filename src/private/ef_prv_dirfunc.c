/**
 * ********************************************************************************************************************
 *  @file     ef_prv_dirfunc_fat.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Directory handling.
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
#include <ef_prv_fat.h>
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

/* Read an object from the directory */
ef_return_et eEFPrvDirRead (
  ef_directory_st * pxDir,
  ef_bool_t       * pbEmpty
)
{
  EF_ASSERT_PRIVATE( 0 != pxDir );
  EF_ASSERT_PRIVATE( 0 != pbEmpty );

#if ( 0 == EF_CONF_VFAT )

  ef_return_et    eRetVal = EF_RET_OK;
  ef_bool_t       bEmpty = EF_BOOL_FALSE;
  ef_fs_st      * pxFS    = pxDir->xObject.pxFS;
  ef_u08_t        u8Attrib;
  ef_u08_t        u8Byte;

  while ( 0 != pxDir->xSector )
  {
    if ( EF_RET_OK != eEFPrvFSWindowLoad( pxFS, pxDir->xSector ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      break;
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
    /* Test for the entry type */
    u8Byte = pxDir->pu8Dir[ EF_DIR_NAME_START ];
    /* Reached to end of the directory */
    if ( 0 == u8Byte )
    {
      bEmpty = EF_BOOL_TRUE;
      break;
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
    /* Get attribute */
    u8Attrib = pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ] & EF_DIR_ATTRIB_BITS_DEFINED;
    pxDir->xObject.u8Attrib = u8Attrib;
    /* Is it a valid entry? */
//    if (    ( EF_DIR_DELETED_MASK != u8Byte )
//         && ( '.' != u8Byte )
//         && ( EF_DIR_ATTRIB_BITS_LFN != u8Attrib )
//         && ( EF_DIR_ATTRIB_BIT_VOLUME_ID != ( ~EF_DIR_ATTRIB_BIT_ARCHIVE & u8Attrib ) ) )
    if ( EF_DIR_DELETED_MASK == u8Byte )
    {
      EF_CODE_COVERAGE( );
    }
    else if ( '.' == u8Byte )
    {
      EF_CODE_COVERAGE( );
    }
    else if ( EF_DIR_ATTRIB_BITS_LFN == u8Attrib )
    {
      EF_CODE_COVERAGE( );
    }
    else if ( EF_DIR_ATTRIB_BIT_VOLUME_ID == ( ~EF_DIR_ATTRIB_BIT_ARCHIVE & u8Attrib ) )
    {
      EF_CODE_COVERAGE( );
    }
    else
    {
      break;
    }
    ef_bool_t     bStretched = EF_BOOL_FALSE;
    ef_bool_t     bMoved = EF_BOOL_FALSE;
    /* Next entry */
    if ( EF_RET_OK != eEFPrvDirectoryIndexNext( pxDir, EF_BOOL_FALSE, &bStretched, &bMoved ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      break;
    }
    else if ( EF_BOOL_TRUE == bMoved )
    {
      bEmpty = EF_BOOL_TRUE;
      break;
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
  }

  if ( EF_RET_OK != eRetVal )
  {
    /* Terminate the read operation on error or EOT */
    pxDir->xSector = 0;
  }
  else
  {
    EF_CODE_COVERAGE( );
  }
  *pbEmpty = bEmpty;


#else
  ef_return_et eRetVal = EF_RET_NO_FILE;
  ef_fs_st *  pxFS      = pxDir->xObject.pxFS;
  ef_u08_t             u8Attrib;
  ef_u08_t             ord = 0xFF;
  ef_u08_t             sum = 0xFF;

  while ( pxDir->xSector )
  {
    eRetVal = eEFPrvFSWindowLoad( pxFS, pxDir->xSector );
    if ( EF_RET_OK != eRetVal )
    {
      break;
    }
    ef_u08_t b = pxDir->pu8Dir[ EF_DIR_NAME_START ];  /* Test for the entry type */
    if ( 0 == b )
    {
      eRetVal = EF_RET_NO_FILE;
      break; /* Reached to end of the directory */
    }
    u8Attrib = pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ] & EF_DIR_ATTRIB_BITS_DEFINED;
    pxDir->xObject.u8Attrib = u8Attrib;
    /* Get attribute */
    /* Is it a valid entry? */
    if (    ( EF_DIR_DELETED_MASK == b )
         || ( '.' == b )
//         || ((int)((u8Attrib & ~EF_DIR_ATTRIB_BIT_ARCHIVE) == EF_DIR_ATTRIB_BIT_VOLUME_ID) != vol ) )
         || (    (    ( 0 == vol )
                   && ( EF_DIR_ATTRIB_BIT_VOLUME_ID == ( ~EF_DIR_ATTRIB_BIT_ARCHIVE & u8Attrib ) ) )
              || (    ( 0 != vol )
                   && ( EF_DIR_ATTRIB_BIT_VOLUME_ID != ( ~EF_DIR_ATTRIB_BIT_ARCHIVE & u8Attrib ) ) ) ) )
    {
      ord = 0xFF;
    }
    /* Else, if An LFN entry is found */
    else if ( EF_DIR_ATTRIB_BITS_LFN == u8Attrib )
    {
      /* Is it start of an LFN sequence? */
      if ( EF_DIR_LFN_LAST & b )
      {
        sum = pxDir->pu8Dir[EF_DIR_LFN_CHECKSUM];
        b &= (ef_u08_t)~EF_DIR_LFN_LAST;
        ord = b;
        pxDir->u32BlkOffset = pxDir->u32Offset;
      }
      ucs2_t *    pxLFNBuffer;            /* LFN working buffer */
      (void) EF_LFN_BUFFER_GET( pxFS, pxLFNBuffer );
      /* Check LFN validity and capture it */
      if (    ( b == ord )
           && ( sum == pxDir->pu8Dir[ EF_DIR_LFN_CHECKSUM ] )
           && ( EF_RET_OK == eEFPrvLFNPick( pxLFNBuffer, pxDir->pu8Dir ) ) )
      {
        ord = ord - 0x01;
      }
      else
      {
        ord = 0xFF;
      }
    }
    else
    {          /* An SFN entry is found */
      ef_u08_t u8CheckSum = 0;
      /* Is there a valid LFN? */
      if (    ( 0 != ord )
           || (    ( EF_RET_OK == eEFPrvSFNChecksumGet(  pxDir->pu8Dir,
                                                          &u8CheckSum ) )
                && ( sum != u8CheckSum ) ) )
      {
        /* It has no LFN. */
        pxDir->u32BlkOffset = 0xFFFFFFFF;
      }
      break;
    }
    /* Next entry */
    eRetVal = eEFPrvDirectoryIndexNext( pxDir, EF_BOOL_FALSE );
    if ( EF_RET_OK != eRetVal )
    {
      break;
    }
  }

  if ( EF_RET_OK != eRetVal )
  {
    pxDir->xSector = 0;    /* Terminate the read operation on error or EOT */
  }
#endif /* ( 0 != EF_CONF_VFAT ) */
  return eRetVal;
}

/* Directory handling - Find an object in the directory */
ef_return_et eEFPrvDirFind (
  ef_directory_st * pxDir,
  ef_bool_t       * pbFound
)
{
  EF_ASSERT_PRIVATE( 0 != pxDir );
  EF_ASSERT_PRIVATE( 0 != pbFound );

#if ( 0 == EF_CONF_VFAT )

  ef_return_et  eRetVal = EF_RET_OK;
  ef_bool_t     bFound = EF_BOOL_FALSE;

  /* Rewind directory object */
  if ( EF_RET_OK != eEFPrvDirectoryIndexSet( pxDir, 0 ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    ef_fs_st * pxFS = pxDir->xObject.pxFS;

    do
    {
      if ( EF_RET_OK != eEFPrvFSWindowLoad( pxFS, pxDir->xSector ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
        break;
      }
      /* If we Reached to end of table */
      else if ( 0 == pxDir->pu8Dir[ EF_DIR_NAME_START ] )
      {
//        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_NO_FILE );
        break;
      }
      else
      {
        /* Non LFN configuration */
        pxDir->xObject.u8Attrib = EF_DIR_ATTRIB_BITS_DEFINED & pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ];
        /* If it is a NOT valid entry */
        if ( 0 != ( EF_DIR_ATTRIB_BIT_VOLUME_ID & pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ] ) )
        {
          EF_CODE_COVERAGE( );
        }
        else if ( EF_RET_OK != eEFPortMemCompare( pxDir->pu8Dir, pxDir->u8Name, 11 ) )
        {
          EF_CODE_COVERAGE( );
        }
        else
        {
          bFound = EF_BOOL_TRUE;
          break;
        }
        ef_bool_t     bStretched = EF_BOOL_FALSE;
        ef_bool_t     bMoved = EF_BOOL_FALSE;
        /* Next entry */
        if ( EF_RET_OK != eEFPrvDirectoryIndexNext( pxDir, EF_BOOL_FALSE, &bStretched, &bMoved ) )
        {
          (void) EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
        }
        else if ( EF_BOOL_FALSE != bMoved )
        {
          break;
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
      }
    }
    while ( EF_RET_OK == eRetVal );
  }

  *pbFound = bFound;

#else
  /* Rewind directory object */
  eRetVal = eEFPrvDirectoryIndexSet( pxDir, 0 );
  if ( EF_RET_OK == eRetVal )
  {
    ef_fs_st  * pxFS = pxDir->xObject.pxFS;
    ef_u08_t c;
    /* Reset LFN sequence */
    ef_u08_t ord = 0xFF;
    ef_u08_t sum = 0xFF;
    pxDir->u32BlkOffset = 0xFFFFFFFF;
    do
    {
      eRetVal = eEFPrvFSWindowLoad( pxFS, pxDir->xSector );
      if ( EF_RET_OK != eRetVal )
      {
        break;
      }
      c = pxDir->pu8Dir[ EF_DIR_NAME_START ];
      if ( 0 == c )
      {
        eRetVal = EF_RET_NO_FILE;
        break;
      }  /* Reached to end of table */
      /* LFN configuration */
      ef_u08_t u8Attrib = EF_DIR_ATTRIB_BITS_DEFINED & pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ];
      pxDir->xObject.u8Attrib = u8Attrib;
      /* If this entry without valid data */
      if (    ( EF_DIR_DELETED_MASK == c )
           || (    ( 0 != ( EF_DIR_ATTRIB_BIT_VOLUME_ID & u8Attrib ) )
                && ( EF_DIR_ATTRIB_BITS_LFN != u8Attrib ) ) )
      {
        /* Reset LFN sequence */
        ord = 0xFF;
        pxDir->u32BlkOffset = 0xFFFFFFFF;
      }
      else
      {
        if ( EF_DIR_ATTRIB_BITS_LFN == u8Attrib )
        { /* An LFN entry is found */
          /* If it is the start of LFN sequence? */
          if ( 0 == ( EF_NS_NOLFN & pxDir->u8Name[ EF_NSFLAG ] ) )
          {
            if ( 0 != ( EF_DIR_LFN_LAST & c ) )
            {
              sum = pxDir->pu8Dir[ EF_DIR_LFN_CHECKSUM ];
              c &= (ef_u08_t)~EF_DIR_LFN_LAST;
              /* LFN start order */
              ord = c;
              /* Start offset of LFN */
              pxDir->u32BlkOffset = pxDir->u32Offset;
            }
            ucs2_t *    pxLFNBuffer;            /* LFN working buffer */
            (void) EF_LFN_BUFFER_GET( pxDir->xObject.pxFS, pxLFNBuffer );
            /* Check validity of the LFN entry and compare it with given name */
            if (    ( c == ord )
                 && ( sum == pxDir->pu8Dir[ EF_DIR_LFN_CHECKSUM ] )
                 && ( EF_RET_OK == eEFPrvLFNCompare(  pxLFNBuffer,
                                                      pxDir->pu8Dir ) ) )
            {
              ord -= (ef_u08_t) 1;
            }
            else
            {
              ord = 0xFF;
            }
          }
        } /* An LFN entry is found */
        else
        { /* An SFN entry is found */
          ef_u08_t u8CheckSum = 0;
          /* LFN matched? */
          if (    ( 0 == ord )
               || (    ( EF_RET_OK == eEFPrvSFNChecksumGet(  pxDir->pu8Dir,
                                                              &u8CheckSum ) )
                    && ( sum != u8CheckSum ) ) )
          {
            break;
          }
          /* SFN matched? */
          /* If it is a NOT valid entry */
          else if (   ( 0 == ( EF_NS_LOSS & pxDir->u8Name[ EF_NSFLAG ] ) )
                   && ( EF_RET_OK == eEFPortMemCompare( pxDir->pu8Dir, pxDir->u8Name, 11 ) ) )
          {
            break;
          }
          else
          {
            /* Reset LFN sequence */
            ord = 0xFF;
            pxDir->u32BlkOffset = 0xFFFFFFFF;
          }
        } /* An SFN entry is found */
      }
      /* Next entry */
      eRetVal = eEFPrvDirectoryIndexNext( pxDir, EF_BOOL_FALSE );
    } while ( EF_RET_OK == eRetVal );
  }
#endif /* ( 0 != EF_CONF_VFAT ) */

  return eRetVal;
}

/* Register an object to the directory */
ef_return_et eEFPrvDirRegister (
  ef_directory_st * pxDir
)
{
  EF_ASSERT_PRIVATE( 0 != pxDir );

#if ( 0 == EF_CONF_VFAT )

  ef_return_et eRetVal = EF_RET_OK;

  ef_fs_st  * pxFS = pxDir->xObject.pxFS;

  /* Allocate an entry for SFN */
  if ( EF_RET_OK != eEFPrvDirectoryAllocate( pxDir, 1 ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
  }
  /* Set SFN entry */
  else if ( EF_RET_OK != eEFPrvFSWindowLoad( pxFS, pxDir->xSector ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
  }
  /* Clean the entry */
  else if ( EF_RET_OK != eEFPortMemZero( pxDir->pu8Dir, EF_DIR_ENTRY_SIZE ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  /* Put SFN */
  else if ( EF_RET_OK != eEFPortMemCopy( pxDir->u8Name, pxDir->pu8Dir + EF_DIR_NAME_START, 11 ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  else
  {
    pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
  }

#else
  ef_return_et  eRetVal = EF_RET_INT_ERR;
  ef_fs_st    * pxFS = pxDir->xObject.pxFS;

  ef_u32_t n;
  ef_u32_t nlen;
  ef_u32_t nent;
  ef_u08_t sn[ 12 ];
  ef_u08_t sum;

  /* Check name validity */
  if ( 0 != ( ( EF_NS_DOT | EF_NS_NONAME ) & pxDir->u8Name[ EF_NSFLAG ] ) )
  {
    return EF_RET_INVALID_NAME;
  }
  ucs2_t *    pxLFNBuffer;            /* LFN working buffer */
  (void) EF_LFN_BUFFER_GET( pxDir->xObject.pxFS, pxLFNBuffer );
  /* Get lfn length */
  for ( nlen = 0 ; 0 != pxLFNBuffer[ nlen ] ; nlen++ ) ;

  /* On the FAT/FAT32 volume */
  eEFPortMemCopy( pxDir->u8Name, sn, 12);
  /* When LFN is out of 8.3 format, generate a numbered name */
  if ( 0 != ( EF_NS_LOSS & sn[ EF_NSFLAG ] ) )
  {
    /* Find only SFN */
    pxDir->u8Name[ EF_NSFLAG ] = EF_NS_NOLFN;
    for ( n = 1 ; n < 100 ; n++ )
    {
      /* Generate a numbered name */
      (void) eEFPrvLFNCreateSFN( pxDir->u8Name, sn, pxLFNBuffer, n );
      /* Check if the name collides with existing SFN */
      eRetVal = eEFPrvDirFind( pxDir );
      if ( EF_RET_OK != eRetVal )
      {
        break;
      }
    }
    /* Abort if too many collisions */
    if ( 100 == n )
    {
      return EF_RET_DENIED;
    }
    /* Abort if the result is other than 'not collided' */
    if (eRetVal != EF_RET_NO_FILE)
    {
      return eRetVal;
    }
    pxDir->u8Name[ EF_NSFLAG ] = sn[ EF_NSFLAG ];
  }

  /* Create an SFN with/without LFNs. */
  if ( 0 != ( EF_NS_LFN & sn[ EF_NSFLAG ] ) )
  {
    /* Number of entries to allocate */
    nent = ( ( nlen + 12 ) / 13 ) + 1;
  }
  else
  {
    /* Number of entries to allocate */
    nent = 1;
  }
  /* Allocate entries */
  eRetVal = eEFPrvDirectoryAllocate( pxDir, nent );
  /* Set LFN entry if needed */
  if (    ( EF_RET_OK == eRetVal )
       && ( 0 != (--nent ) ) )
  {
    eRetVal = eEFPrvDirectoryIndexSet( pxDir,
                                      pxDir->u32Offset
                                    - ( nent * EF_DIR_ENTRY_SIZE ) );
    if ( EF_RET_OK == eRetVal )
    {
      /* Checksum value of the SFN tied to the LFN */
      (void) eEFPrvSFNChecksumGet( pxDir->u8Name , &sum );
      /* Store LFN entries in bottom first */
      do {
        eRetVal = eEFPrvFSWindowLoad( pxFS, pxDir->xSector );
        if ( EF_RET_OK != eRetVal )
        {
          break;
        }
        (void) eEFPrvLFNPut( pxLFNBuffer, pxDir->pu8Dir, (ef_u08_t)nent, sum );
        pxFS->u8WinFlags = 1;
        /* Next entry */
        eRetVal = eEFPrvDirectoryIndexNext( pxDir, EF_BOOL_FALSE );
      }
      while (    ( EF_RET_OK == eRetVal )
              && ( 0 != (--nent ) ) );
    }
  }

  /* Set SFN entry */
  if ( EF_RET_OK == eRetVal )
  {
    eRetVal = eEFPrvFSWindowLoad( pxFS, pxDir->xSector );
    if ( EF_RET_OK == eRetVal )
    {
      /* Clean the entry */
      eEFPortMemZero( pxDir->pu8Dir, EF_DIR_ENTRY_SIZE );
      /* Put SFN */
      eEFPortMemCopy( pxDir->u8Name, pxDir->pu8Dir + EF_DIR_NAME_START, 11 );
      /* Put NT u8StatusFlags */
      pxDir->pu8Dir[EF_DIR_LFN_NTres] = (EF_NS_BODY | EF_NS_EXT) & pxDir->u8Name[EF_NSFLAG];
      pxFS->u8WinFlags = 1;
    }
  }
#endif /* ( 0 != EF_CONF_VFAT ) */

  return eRetVal;
}

/* Remove an object from the directory */
ef_return_et eEFPrvDirRemove (
  ef_directory_st * pxDir
)
{
  EF_ASSERT_PRIVATE( 0 != pxDir );

#if ( 0 == EF_CONF_VFAT )

  ef_return_et eRetVal = EF_RET_OK;

  ef_fs_st  * pxFS = pxDir->xObject.pxFS;

  if ( EF_RET_OK != eEFPrvFSWindowLoad( pxFS, pxDir->xSector ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  else
  {
    /* Mark the entry 'deleted'.*/
    pxDir->pu8Dir[ EF_DIR_NAME_START ] = EF_DIR_DELETED_MASK;
    pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
  }

#else
  ef_return_et  eRetVal = EF_RET_INT_ERR;
  ef_fs_st    * pxFS = pxDir->xObject.pxFS;

  ef_u32_t     last = pxDir->u32Offset;

  if ( 0xFFFFFFFF == pxDir->u32BlkOffset )
  {
    /* Goto top of the entry block if LFN is exist */
    eRetVal = EF_RET_OK;
  }
  else
  {
    /* Goto top of the entry block if LFN is exist */
    eRetVal = eEFPrvDirectoryIndexSet( pxDir, pxDir->u32BlkOffset );
  }
  if ( EF_RET_OK == eRetVal )
  {
    do
    {
      eRetVal = eEFPrvFSWindowLoad( pxFS, pxDir->xSector );
      if ( EF_RET_OK != eRetVal )
      {
        break;
      }
      /* Mark the entry 'deleted'. */
      pxDir->pu8Dir[ EF_DIR_NAME_START ] = EF_DIR_DELETED_MASK;
      pxFS->u8WinFlags = 1;
      /* If reached last entry */
      if ( pxDir->u32Offset >= last )
      {
        /* then all entries of the object has been deleted. */
        break;
      }
      /* Next entry */
      eRetVal = eEFPrvDirectoryIndexNext( pxDir, EF_BOOL_FALSE );
    } while ( EF_RET_OK == eRetVal );
    if ( EF_RET_NO_FILE == eRetVal )
    {
      eRetVal = EF_RET_INT_ERR;
    }
  }
#endif /* ( 0 != EF_CONF_VFAT ) */

  return eRetVal;
}
/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

