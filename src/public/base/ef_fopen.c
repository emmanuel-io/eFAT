/**
 * ********************************************************************************************************************
 *  @file     ef_fopen.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Open or Create a File
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
#include "ef_prv_drive.h"
#include "ef_prv_directory.h"
#include "ef_prv_dirfunc.h"
#include "ef_prv_fs_window.h"
#include "ef_prv_lock.h"
#include "ef_prv_string.h"
#include "ef_prv_volume.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_path_follow.h"
#include "ef_prv_unicode.h"
#include "ef_prv_validate.h"
#include "ef_prv_volume_nb.h"
#include "ef_prv_volume.h"
#include <ef_prv_volume_mount.h>
#include <ef_port_load_store.h>
#include <ef_port_memory.h>

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */

/**
 *  @brief  Truncate a File on opening
 *
 *  @param  pxFile  Pointer to the blank file object
 *  @param  pxFS    Pointer to the file system object
 *  @param  pxDir   Pointer to directory object to update
 *
 *  @return Function completion
 *  @retval EF_RET_OK                   Succeeded
 *  @retval EF_RET_DISK_ERR             A hard error occurred in the low level disk I/O layer
 *  @retval EF_RET_INT_ERR              Assertion failed
 *  @retval EF_RET_NOT_READY            The physical drive cannot work
 *  @retval EF_RET_NO_FILE              Could not find the file
 *  @retval EF_RET_NO_PATH              Could not find the pxPath
 *  @retval EF_RET_INVALID_NAME         The pxPath name format is invalid
 *  @retval EF_RET_DENIED               Access denied due to prohibited access or directory full
 *  @retval EF_RET_EXIST                Access denied due to prohibited access
 *  @retval EF_RET_INVALID_OBJECT       The file/directory object is invalid
 *  @retval EF_RET_WRITE_PROTECTED      The physical drive is write protected
 *  @retval EF_RET_INVALID_DRIVE        The logical drive number is invalid
 *  @retval EF_RET_NOT_ENABLED          The volume has no work area
 *  @retval EF_RET_NO_FILESYSTEM        There is no valid FAT volume
 *  @retval EF_RET_MKFS_ABORTED         The f_mkfs() aborted due to any problem
 *  @retval EF_RET_TIMEOUT              Could not get a grant to access the volume within defined period
 *  @retval EF_RET_LOCKED               The operation is rejected according to the file sharing policy
 *  @retval EF_RET_NOT_ENOUGH_CORE      LFN working buffer could not be allocated
 *  @retval EF_RET_TOO_MANY_OPEN_FILES  Number of open files > EF_CONF_FILE_LOCK
 *  @retval EF_RET_INVALID_PARAMETER    Given parameter is invalid
 */
ef_return_et eEF_file_truncate (
  ef_file_st      * pxFile,
  ef_fs_st        * pxFS,
  ef_directory_st * pxDir
);

/* Local functions ------------------------------------------------------------------------------------------------- */

ef_return_et eEF_file_truncate (
  ef_file_st      * pxFile,
  ef_fs_st        * pxFS,
  ef_directory_st * pxDir
)
{
  EF_ASSERT_PRIVATE( 0 != pxFile );
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_ERROR;

  ef_u32_t  u32Cluster;
  ef_lba_t  xSector;

  /* Set directory entry initial state */
  /* Get current cluster chain */
  //u32Cluster = eEFPrvDirectoryClusterGet( pxFS, pxDir->pu8Dir );
  if ( EF_RET_OK != eEFPrvDirectoryClusterGet(  pxFS,
                                                pxDir->pu8Dir,
                                                &u32Cluster ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  /* Set created time */
  vEFPortStoreu32( pxDir->pu8Dir + EF_DIR_TIME_CREATED, EF_FATTIME_GET( ) );
  /* Reset attribute */
  pxDir->pu8Dir[ EF_DIR_ATTRIBUTES ] = EF_DIR_ATTRIB_BIT_ARCHIVE;
  /* Reset file allocation info */
  (void) eEFPrvDirectoryClusterSet( pxFS, pxDir->pu8Dir, 0 );
  vEFPortStoreu32( pxDir->pu8Dir + EF_DIR_FILE_SIZE, 0 );
  pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
  /* Remove the cluster chain if exist */
  if ( 0 != u32Cluster )
  {
    xSector = pxFS->xWindowSector;
    if ( EF_RET_OK == eEFPrvFATChainRemove( &(pxDir->xObject), u32Cluster, 0 ) )
    {
      eRetVal = eEFPrvFSWindowLoad( pxFS, xSector );
      if ( EF_RET_OK != eRetVal ) { (void) EF_RETURN_CODE_HANDLER( eRetVal );}
      /* Reuse the cluster hole */
      pxFS->u32ClstLast = u32Cluster - 1;
    }
  }

    return eRetVal;
}

/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEF_fopen (
  EF_FILE     * pxFile,
  const TCHAR * pxPath,
  ef_u08_t      u8Mode
)
{
  EF_ASSERT_PUBLIC( 0 != pxFile );
  EF_ASSERT_PUBLIC( 0 != pxPath );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st *    pxFS;
  ef_u08_t      u8temp = u8Mode & (   EF_FILE_OPEN_EXISTING
                                    | EF_FILE_OPEN_ANYWAY
                                    | EF_FILE_OPEN_NEW );

  /* If parameters check on file opening mode */
  if (    ( 0 == ( u8temp & (   EF_FILE_OPEN_EXISTING
                              | EF_FILE_OPEN_ANYWAY
                              | EF_FILE_OPEN_NEW ) ) )
            && ( 0 == ( ~EF_FILE_OPEN_MASK & u8Mode ) ) )
  {
    /* Parameters problem */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_PARAMETER );
  }
  else if ( EF_RET_OK != eEFPrvVolumeMountCheck( &pxPath, &pxFS ) )
  {
    /* Parameters problem */
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
  }
  else
  {
    ef_directory_st     xDir;

    /* Define buffers for name */
    EF_LFN_BUFFER_DEFINE

    xDir.xObject.pxFS = pxFS;

    /* Link buffers for name to the instance of the FS object */
    if ( EF_RET_OK != EF_LFN_BUFFER_SET( pxFS ) )
    {
      /* LFN Buffer setting error */
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_LFN_BUFFER_ERROR );
    }
    else
    {

      ef_return_et  eResult;

      /* Follow the file path */
      if ( EF_RET_OK != eEFPrvPathFollow( pxPath, &xDir, &eResult ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      /* Directory found for this path */
      else if ( EF_RET_OK == eResult )
      {
        /* If it is Origin directory itself */
        if ( 0 != ( EF_NS_NONAME & xDir.u8Name[ EF_NSFLAG ] ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_NAME );
        }
        /* Else if it is a directory */
        else if ( 0 != ( xDir.xObject.u8Attrib & EF_DIR_ATTRIB_BIT_DIRECTORY ) )
        {
          /* We try to open a file and a directory exist ! */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DENIED );
        }
        /* Else if File opening only if it doesn't exist */
        else if ( 0 != ( u8Mode & EF_FILE_OPEN_NEW ) )
        {
          /* We try to open only a new file and a directory exist ! */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_EXIST );
        }
        /* Else if     Write mode
                   AND File is not flagged R/O in filesystem */
        else if (    ( 0 != ( u8Mode & EF_FILE_OPEN_WRITE ) )
                  && ( 0 != ( xDir.xObject.u8Attrib & EF_DIR_ATTRIB_BIT_READONLY ) ) )
        {
          /* We try to open an existing Read-Only file in write mode ! */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DENIED );
        }
        /* Else, if opening mode is existing or anyway */
        else if ( ( 0 != ( u8Mode & (   EF_FILE_OPEN_EXISTING
                                      | EF_FILE_OPEN_ANYWAY ) ) ) )
        {
          /* Check if the file cannot be used */
          /* 1: Write access mode / 0: Read access mode */
          eRetVal = eEFPrvLockCheck( &xDir, (int) ( u8Mode & EF_FILE_OPEN_WRITE ) );
          if ( EF_RET_OK != eRetVal ) { (void) EF_RETURN_CODE_HANDLER( eRetVal );}
        }
        else
        {
          /* File is opened without restrictions */
          EF_CODE_COVERAGE( );
        }
      }
      /* Directory found for this path END */

      /* Else, if     File doesn't exist
       *          AND opening mode is anyway or new only */
      else if (    ( EF_RET_NO_FILE == eResult )
                && ( 0 != ( u8Mode & (   EF_FILE_OPEN_ANYWAY
                                       | EF_FILE_OPEN_NEW ) ) ) )
      { /* Create or Open a file */

        /* Consider everything ok bby default */
        eRetVal = EF_RET_OK;
        /* If there is NOT file lock available */
        if ( EF_RET_OK != eEFPrvLockEnq( ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_TOO_MANY_OPEN_FILES );
        }
        /* Else, Try to create a new entry */
        else if ( EF_RET_OK != eEFPrvDirRegister( &xDir ) )
        {
          /* Parameters problem */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
        }
        /* Else */
        else
        {
          /* File is created */
          u8Mode |= EF_FILE_MODIFIED;
        }
      }

      else
      {
        /* Parameters problem */
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INVALID_PARAMETER );
      }

      /* NOW DO all the checks on the opened or created directory */
      /* NOW WE HAVE A FILE,  */

      /* If something went wrong */
      if ( EF_RET_OK != eRetVal )
      {
        EF_CODE_COVERAGE( );
      }
      /* Else, if we DO NOT need to truncate the file */
      else if ( 0 == ( u8Mode & EF_FILE_OPEN_TRUNCATE ) )
      {
        EF_CODE_COVERAGE( );
      }
      /* Else, if truncating the file failed (FAT filesystem) */
      else if ( EF_RET_OK != eEF_file_truncate( pxFile,pxFS, &xDir ) )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
      }
      else
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
        EF_CODE_COVERAGE( );
      }

      if ( EF_RET_OK != eRetVal )
      {
        EF_CODE_COVERAGE( );
      }
      else
      {
        /* Set file change u8StatusFlags if created or overwritten */
        if ( 0 != ( u8Mode & EF_FILE_MODIFIED ) )
        {
          u8Mode |= EF_FILE_MODIFIED;
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
        /* Pointer to the directory entry */
        pxFile->xDirSector  = pxFS->xWindowSector;
        pxFile->pu8DirPtr   = xDir.pu8Dir;
        /* Lock the file for this session in access 0: read,1: write */
        if ( EF_RET_OK != eEFPrvLockInc( &xDir, (int) ( u8Mode & EF_FILE_OPEN_WRITE ), &(pxFile->xObject.u32LockId) ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
      }

      if ( EF_RET_OK != eRetVal )
      {
        EF_CODE_COVERAGE( );
      }
      else
      {
        ef_u32_t  u32Cluster;
        ef_u32_t  u32ClusterSize;

        /* Get object allocation info */
        //pxFile->xObject.u32ClstStart  = eEFPrvDirectoryClusterGet( pxFS, xDir.pu8Dir );
        if ( EF_RET_OK != eEFPrvDirectoryClusterGet( pxFS, xDir.pu8Dir, &(pxFile->xObject.u32ClstStart) ) )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
        pxFile->u32Size = u32EFPortLoad( xDir.pu8Dir + EF_DIR_FILE_SIZE );
        /* Validate the file object */
        pxFile->xObject.pxFS = pxFS;
        pxFile->xObject.u16MountId = pxFS->u16MountId;
        /* Set file access u8Mode */
        pxFile->u8StatusFlags = u8Mode;
        /* Clear error u8StatusFlags */
        pxFile->u8ErrorCode = 0;
        /* Invalidate current data sector */
        pxFile->xSector = 0;
        /* Set file pointer top of the file */
        pxFile->u32FileOffset = 0;
        /* Clear sector buffer */
        eEFPortMemZero( pxFile->u8Window, sizeof(pxFile->u8Window) );

        /* If     EF_FILE_OPEN_APPEND is specified
         *    AND File size is not null */
        if (    ( 0 != (u8Mode & EF_FILE_OPEN_APPEND) )
             && ( pxFile->u32Size > 0 ) )
        {
          /* Seek to end of file if EF_FILE_OPEN_APPEND is specified */
          /* Offset to seek */
          pxFile->u32FileOffset = pxFile->u32Size;
          /* Cluster size in byte */
          u32ClusterSize = (ef_u32_t)pxFS->u8ClstSize * EF_SECTOR_SIZE( pxFS );
          /* Follow the cluster chain */
          u32Cluster = pxFile->xObject.u32ClstStart;
          ef_u32_t u32Offset = pxFile->u32Size;
          while ( u32Offset > u32ClusterSize )
          {
            if ( EF_RET_OK != eEFPrvFATGet( pxFS, u32Cluster, &u32Cluster ) )
            {
              eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
              break;
            }
            else
            {
              u32Offset -= u32ClusterSize;
            }
          }
          pxFile->u32Clst = u32Cluster;
          /* Fill sector buffer if not on the sector boundary */
          if (    ( eRetVal == EF_RET_OK )
               && ( u32Offset % EF_SECTOR_SIZE( pxFS ) ) )
          {
            ef_lba_t xSector;
            if ( EF_RET_OK != eEFPrvFATClusterToSector( pxFS, u32Cluster, &xSector ) )
            {
              eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
            }
            else
            {
              pxFile->xSector = xSector + (ef_u32_t)(u32Offset / EF_SECTOR_SIZE( pxFS ));
              if ( EF_RET_OK != eEFPrvDriveRead( pxFS->u8PhysDrv, pxFile->u8Window, pxFile->xSector, 1 ) )
              {
                eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_DISK_ERR );
              }
              else
              {
                EF_CODE_COVERAGE( );
              }
            }
          }
          else
          {
            EF_CODE_COVERAGE( );
          }
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
      }

      EF_LFN_BUFFER_FREE();
    }

    if ( EF_RET_OK != eRetVal )
    {
      /* Invalidate file object on error */
      pxFile->xObject.pxFS = 0;
    }
    else
    {
      EF_CODE_COVERAGE( );
    }

    (void) eEFPrvFSUnlock( pxFS, eRetVal );

  }

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */

