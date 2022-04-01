/**
 * ********************************************************************************************************************
 *  @file     ef_prv_dirfunc_vfat.c
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
#include <ef_port_load_store.h>
#include <ef_port_memory.h>
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

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */
/* Read an object from the directory */
ef_return_et eEFPrvDirReadVFAT (
  ef_directory_st * pxDir,
  int               vol
)
{
  ef_return_et eRetVal = EF_RET_NO_FILE;
#if ( 0 != EF_CONF_VFAT )
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
ef_return_et eEFPrvDirFindVFAT (
  ef_directory_st * pxDir
)
{
  ef_return_et eRetVal = EF_RET_INT_ERR;

#if ( 0 != EF_CONF_VFAT )
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
ef_return_et eEFPrvDirRegisterVFAT (
  ef_directory_st * pxDir
)
{
  ef_return_et  eRetVal = EF_RET_INT_ERR;
#if ( 0 != EF_CONF_VFAT )
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
ef_return_et eEFPrvDirRemoveVFAT (
  ef_directory_st * pxDir
)
{
  ef_return_et  eRetVal = EF_RET_INT_ERR;
#if ( 0 != EF_CONF_VFAT )
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
