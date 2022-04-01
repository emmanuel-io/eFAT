/**
 * ********************************************************************************************************************
 *  @file     ef_prv_fat.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Code file for FAT functions.
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
//#include "ef_port_diskio.h"
#include "ef_prv_directory.h"
#include "ef_prv_dirfunc.h"
#include "ef_prv_fs_window.h"
#include "ef_prv_drive.h"
//#include "ef_prv_string.h"
#include "ef_prv_volume.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_unicode.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */

/**
 * End of FAT chain marker
 */
#define EF_FAT_END_OF_CHAIN  0xFFFFFFFF

/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */

/**
 *  @brief  FAT access - Find a free cluster
 *
 *  @param  pxObject    Pointer to the corresponding object
 *  @param  u32Cluster  Cluster number from where to start looking
 *  @param  pu32Cluster Pointer to the cluster value number to update
 *
 *  @return Function completion
 *  @retval EF_RET_OK       Succeeded
 *  @retval EF_RET_DISK_ERR A hard error occurred in the low level disk I/O layer
 *  @retval EF_RET_INT_ERR  Internal error
 *  @retval EF_RET_ASSERT   Assertion failed
 */
static ef_return_et eEFPrvFATClusterFindFree (
  ef_object_st  * pxObject,
  ef_u32_t        u32Cluster,
  ef_u32_t      * pu32Cluster
);

/* Local functions ------------------------------------------------------------------------------------------------- */
static ef_return_et eEFPrvFATClusterFindFree (
  ef_object_st  * pxObject,
  ef_u32_t        u32Cluster,
  ef_u32_t      * pu32Cluster
)
{
  EF_ASSERT_PRIVATE( 0 != pxObject );
  EF_ASSERT_PRIVATE( 0 != pu32Cluster );

  ef_return_et  eRetVal = EF_RET_FAT_FULL; /* By default, FAT is full */
  ef_fs_st    * pxFS = pxObject->pxFS;

  /* Check if in valid range */
  if ( EF_RET_OK !=  eEFPrvFATClusterNbCheck ( pxFS->u32FatEntriesNb, u32Cluster ) )
  {
    eRetVal = EF_RET_INT_ERR;
  }
  else
  {
    ef_u32_t  u32ClusterValue;
    ef_u32_t  u32ClusterStop = 0;
    /* If we start from the beginning of the FAT */
    if ( 2 == u32Cluster )
    {
      /* Stop at the end of the FAT */
      u32ClusterStop = 0;
    }
    /* Else, if we start from the end of the FAT */
    else if ( pxFS->u32FatEntriesNb == u32Cluster )
    {
      /* Stop just before the end of the FAT */
      u32ClusterStop = pxFS->u32FatEntriesNb - 1;
    }
    else
    {
      /* Stop at the end of the FAT */
      u32ClusterStop = u32Cluster - 1;
    }

    /* Loop through the FAT */
    while ( u32ClusterStop != u32Cluster )
    {
      /* Get the cluster status */
      eRetVal = eEFPrvFATGet( pxFS, u32Cluster, &u32ClusterValue );
      /* If an error occured */
      if ( EF_RET_OK != eRetVal )
      {
        /* Return immediately */
        break;
      }
      /* Else, if a free cluster */
      else if ( 0 == u32ClusterValue )
      {
        /* Return new cluster number or error status */
        *pu32Cluster = u32Cluster;
        eRetVal = EF_RET_OK;
        break;
      }
      else
      {
        ; /* Keep Looping */
      }
      u32Cluster++;
      if ( pxFS->u32FatEntriesNb < u32Cluster )
      {
        /* Stop just before the end of the FAT */
        u32Cluster = 2;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
    } /* Loop through the FAT */
  }
  /* If no free cluster was found or an error occured */
  if ( EF_RET_OK != eRetVal )
  {
    /* Set cluster number to 0 */
    *pu32Cluster = 0;
  }

  return eRetVal;
}

/* Public functions ------------------------------------------------------------------------------------------------ */

/* Check if cluster number is valid */
ef_return_et eEFPrvFATClusterNbCheck (
  ef_u32_t  u32FatEntriesNb,
  ef_u32_t  u32Cluster
)
{
  ef_return_et  eRetVal = EF_RET_OK;

  /* Check if in valid range */
  if ( 2 > u32Cluster )
  {
//    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_FAT_CLUSTER_UNDER );
    eRetVal = EF_RET_FAT_CLUSTER_UNDER;
  }
  else if ( u32FatEntriesNb <= u32Cluster )
  {
//    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_FAT_CLUSTER_OVER );
    eRetVal = EF_RET_FAT_CLUSTER_OVER;
  }
  else
  {
    EF_CODE_COVERAGE( );
  }

  return eRetVal;
}

#if 0
/* Check if cluster number is valid */
ef_return_et eEFPrvFATClusterTypeGet (
  ef_fs_st *  pxFS,
  ef_u32_t            u32Cluster
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_FAT_ENTRY_NEXT;

  /* If Cluster is empty */
  if ( 0x00000000 == u32Cluster )
  {
    eRetVal = EF_RET_FAT_ENTRY_FREE;
  }
  /* Else, if Cluster is end of chain (special case of allocation failure) */
  else if ( 0x00000001 == u32Cluster )
  {
    eRetVal = EF_RET_FAT_ENTRY_END_1;
  }
  else if (    ( 0 != ( EF_FS_FAT32 & pxFS->u8FsType ) )
            && ( 0x0FFFFFF7 == ( 0x0FFFFFFF & u32Cluster ) ) )
  {
    eRetVal = EF_RET_FAT_ENTRY_BAD;
  }
  else if (    ( 0 != ( EF_FS_FAT16 & pxFS->u8FsType ) )
            && ( 0x0000FFF7 == ( 0x0000FFFF & u32Cluster ) ) )
  {
    eRetVal = EF_RET_FAT_ENTRY_BAD;
  }
  else if (    ( 0 != ( EF_FS_FAT12 & pxFS->u8FsType ) )
            && ( 0x00000FF7 == ( 0x00000FFF & u32Cluster ) ) )
  {
    eRetVal = EF_RET_FAT_ENTRY_BAD;
  }
  /* Else, if Cluster is last of a chain */
  else if ( pxFS->u32FatEntriesNb > u32Cluster )
  {
    eRetVal = EF_RET_FAT_ENTRY_LAST;
  }
  /* Else, Cluster is followed by another one  */
  else
  {
    eRetVal = EF_RET_FAT_ENTRY_NEXT;
  }

  return eRetVal;
}
#endif

/* Convert cluster number to physical sector number */
ef_return_et eEFPrvFATClusterToSector (
  ef_fs_st  * pxFS,
  ef_u32_t    u32Cluster,
  ef_lba_t  * pxSector
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_return_et  eRetVal = EF_RET_OK;

  ef_lba_t xSector = 0;

  u32Cluster -= 2;    /* Cluster number is origin from 2 */
  if ( u32Cluster < ( pxFS->u32FatEntriesNb - 2 ) )
  {
    /* Start sector number of the cluster */
    xSector = pxFS->xDataBase + ( (ef_lba_t)pxFS->u8ClstSize * u32Cluster );
  }
  else
  {
    eRetVal = EF_RET_ERROR;
  }

  *pxSector = xSector;

  return eRetVal;
}


/* FAT access - Read value of a FAT entry */
ef_return_et eEFPrvFATGet (
  ef_fs_st  * pxFS,
  ef_u32_t    u32Cluster,
  ef_u32_t  * pu32Value
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );
  EF_ASSERT_PRIVATE( 0 != pu32Value );

  ef_return_et  eRetVal = EF_RET_OK;

  /* If Cluster not in valid range */
  if ( EF_RET_OK != eEFPrvFATClusterNbCheck( pxFS->u32FatEntriesNb, u32Cluster ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );  /* Internal error */
  }
  else if ( 0 != ( EF_FS_FAT32 & pxFS->u8FsType ) )
  {
    /* Load the FS window with the sector containing the FAT Cluster Number */
    if ( EF_RET_OK == eEFPrvFSWindowLoad(  pxFS,
                                           pxFS->xFatBase
                                         + ( u32Cluster / (EF_SECTOR_SIZE( pxFS ) / 4 ) ) ) )
    {
      /* Simple ef_u32_t array but mask out upper 4 bits */
      *pu32Value = 0x0FFFFFFF & u32EFPortLoad(  pxFS->pu8Window
                                        + u32Cluster * 4 % EF_SECTOR_SIZE( pxFS ) );
    }
    else
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
    }

  }
  else if ( 0 != ( EF_FS_FAT16 & pxFS->u8FsType ) )
  {
    /* Load the FS window with the sector containing the FAT Cluster Number */
    if ( EF_RET_OK == eEFPrvFSWindowLoad(  pxFS,
                                   pxFS->xFatBase
                                 + ( u32Cluster / (EF_SECTOR_SIZE( pxFS ) / 2 ) ) ) )
    {
      /* Simple ef_u16_t array */
      *pu32Value = u16EFPortLoad(pxFS->pu8Window + u32Cluster * 2 % EF_SECTOR_SIZE( pxFS ));
    }
    else
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
    }
  }
  else if ( 0 != ( EF_FS_FAT12 & pxFS->u8FsType ) )
  {
//    eRetVal = EF_RET_INT_ERR;

    ef_u32_t  u32ByteOffset = u32Cluster;
    u32ByteOffset += u32ByteOffset / 2;
    eRetVal = eEFPrvFSWindowLoad( pxFS,
                                  pxFS->xFatBase
                                  + ( u32ByteOffset / EF_SECTOR_SIZE( pxFS ) ) );
    if ( EF_RET_OK != eRetVal )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
      EF_CODE_COVERAGE( );
    }
    else
    {
      /* Get 1st byte of the entry */
      ef_u16_t  u16Value = (ef_u16_t) pxFS->pu8Window[ u32ByteOffset++ % EF_SECTOR_SIZE( pxFS ) ];
      /* Load FSWindow to access 2nd byte of the entry */
      eRetVal = eEFPrvFSWindowLoad( pxFS,
                                    pxFS->xFatBase
                                    + ( u32ByteOffset / EF_SECTOR_SIZE( pxFS ) ) );
      if ( EF_RET_OK != eRetVal )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );
        EF_CODE_COVERAGE( );
      }
      else
      {
        /* Merge 2nd byte of the entry */
        u16Value = (ef_u16_t) (u16Value | ( (ef_u16_t) pxFS->pu8Window[ u32ByteOffset % EF_SECTOR_SIZE( pxFS ) ] << 8 ));
        /* Adjust bit position */
        if ( 0 != ( 0x00000001 & u32Cluster ) )
        {
          *pu32Value = u16Value >> 4;
        }
        else
        {
          *pu32Value = 0xFFF & u16Value;
        }
      }
    }

  }
  else
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_INT_ERR );  /* Internal error */
  }

  return eRetVal;
}

/* FAT access - Change value of a FAT entry */
ef_return_et eEFPrvFATSet (
  ef_fs_st  * pxFS,
  ef_u32_t    u32Cluster,
  ef_u32_t    u32NewValue
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  /* Check if in valid range */
  ef_return_et eRetVal = EF_RET_INT_ERR;

  /* If Cluster not in valid range */
  if ( EF_RET_OK != eEFPrvFATClusterNbCheck( pxFS->u32FatEntriesNb, u32Cluster ) )
  {
    EF_CODE_COVERAGE( );
  }
  else if ( 0 != ( EF_FS_FAT32 & pxFS->u8FsType ) )
  {
    eRetVal = EF_RET_INT_ERR;

      /* Load the FS window with the sector containing the FAT Cluster Number */
      eRetVal = eEFPrvFSWindowLoad(   pxFS,
                                      pxFS->xFatBase
                                    + ( u32Cluster / ( EF_SECTOR_SIZE( pxFS ) / 4) ) );
      if ( EF_RET_OK == eRetVal )
      {
        u32NewValue =   ( u32NewValue & 0x0FFFFFFF )
                      | (   u32EFPortLoad( pxFS->pu8Window + u32Cluster * 4 % EF_SECTOR_SIZE( pxFS ) )
                          & 0xF0000000 );
        vEFPortStoreu32( pxFS->pu8Window + u32Cluster * 4 % EF_SECTOR_SIZE( pxFS ), u32NewValue );
        pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
      }
      else
      {
        eRetVal = EF_RET_INT_ERR;  /* Internal error */
      }
  }
  else if ( 0 != ( EF_FS_FAT16 & pxFS->u8FsType ) )
  {
    eRetVal = EF_RET_INT_ERR;

      /* Load the FS window with the sector containing the FAT Cluster Number */
      eRetVal = eEFPrvFSWindowLoad( pxFS,
                                      pxFS->xFatBase
                                    + ( u32Cluster / ( EF_SECTOR_SIZE( pxFS ) / 2 ) ) );
      if ( EF_RET_OK == eRetVal )
      {
        /* Simple ef_u16_t array */
        vEFPortStoreu16(  pxFS->pu8Window + u32Cluster * 2 % EF_SECTOR_SIZE( pxFS ),
                    (ef_u16_t)u32NewValue );
        pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
  }
  else if ( 0 != ( EF_FS_FAT12 & pxFS->u8FsType ) )
  {
    eRetVal = EF_RET_INT_ERR;

    /* u32ByteOffset: byte offset of the entry */
    ef_u32_t u32ByteOffset = u32Cluster;
    /* Multiply offset by 1.5 (12 bits fat entries) */
    u32ByteOffset += u32ByteOffset / 2;
    eRetVal = eEFPrvFSWindowLoad( pxFS, pxFS->xFatBase + ( u32ByteOffset / EF_SECTOR_SIZE( pxFS ) ) );
    if ( EF_RET_OK == eRetVal )
    {
      ef_u08_t * p = pxFS->pu8Window + ( u32ByteOffset++ % EF_SECTOR_SIZE( pxFS ) );
      /* Update 1st byte */
      if ( 0 != ( 0x00000001 & u32Cluster ) )
      {
        *p = (ef_u08_t) ( (*p & 0x0F) | ( (ef_u08_t)u32NewValue << 4 ) );
      }
      else
      {
        *p = (ef_u08_t) u32NewValue;
      }
      pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
      eRetVal = eEFPrvFSWindowLoad( pxFS, pxFS->xFatBase + ( u32ByteOffset / EF_SECTOR_SIZE( pxFS ) ) );
      if ( EF_RET_OK == eRetVal )
      {
        p = pxFS->pu8Window + u32ByteOffset % EF_SECTOR_SIZE( pxFS );
        /* Update 2nd byte */
        if ( 0 != ( 0x00000001 & u32Cluster ) )
        {
          *p = (ef_u08_t)(u32NewValue >> 4);
        }
        else
        {
          *p = (ef_u08_t) ((*p & 0xF0) | ((ef_u08_t)(u32NewValue >> 8) & 0x0F));
        }
        pxFS->u8WinFlags = EF_FS_WIN_DIRTY;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
    }
    else
    {
      eRetVal = EF_RET_INT_ERR;  /* Internal error */
    }

  }
  else
  {
    eRetVal = EF_RET_INT_ERR;  /* Internal error */
  }

  return eRetVal;
}

#if 0
/* FAT handling - Remove a cluster chain */
ef_return_et eEFPrvFATChainRemoveFAT (
  ef_object_st * pxObject,
  ef_u32_t          u32Cluster,
  ef_u32_t          u32ClusterPrev
)
{
  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st *pxFS = pxObject->pxFS;

  /* Check if in valid range */
  if ( EF_RET_OK != eEFPrvFATClusterNbCheck( pxFS, u32Cluster ) )
  {
    eRetVal = EF_RET_INT_ERR;
  }
  else
  {
    /* Mark the previous cluster 'EOC' on the FAT if it exists */
    if (    ( 0 != u32ClusterPrev )
         && ( 2 != pxObject->u8Status ) )
    {
      eRetVal = eEFPrvFATSet( pxFS, u32ClusterPrev, EF_FAT_END_OF_CHAIN );
    }
    if ( EF_RET_OK == eRetVal )
    {
      ef_u32_t      nxt;

      /* Remove the chain */
      do
      {
        /* Get cluster status */
        eRetVal = eEFPrvFATGet(pxObject, u32Cluster, &nxt );
        /* Empty cluster? */
        if ( 0 == nxt )
        {
          break;
        }
        /* Else if an error */
        else if ( EF_RET_OK != eRetVal )
        {
          return eRetVal;
          break;
        }
        else
        {
          /* Mark the cluster 'free' on the FAT */
          eRetVal = eEFPrvFATSet( pxFS, u32Cluster, 0 );
          if ( EF_RET_OK != eRetVal )
          {
            return eRetVal;
            break;
          }
          else
          {
            if ( pxFS->u32ClstFreeNb < ( pxFS->u32FatEntriesNb - 2 ) )
            {  /* Update FSINFO */
              pxFS->u32ClstFreeNb++;
              pxFS->u8FsInfoFlags |= 1;
            }
            /* Next cluster */
            u32Cluster = nxt;
          }
        }
      /* Repeat while not the last link */
      } while ( u32Cluster < pxFS->u32FatEntriesNb );
    }
  }
  return eRetVal;
}

/* FAT handling - Remove a cluster chain */
ef_return_et eEFPrvFATChainRemoveFATTRIM (
  ef_object_st * pxObject,
  ef_u32_t          u32Cluster,
  ef_u32_t          u32ClusterPrev
)
{
  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st *pxFS = pxObject->pxFS;

  /* Check if in valid range */
  if ( EF_RET_OK != eEFPrvFATClusterNbCheck( pxFS, u32Cluster ) )
  {
    eRetVal = EF_RET_INT_ERR;
  }
  else
  {
    /* Mark the previous cluster 'EOC' on the FAT if it exists */
    if (    ( 0 != u32ClusterPrev )
         && ( 2 != pxObject->u8Status ) )
    {
      eRetVal = eEFPrvFATSet( pxFS, u32ClusterPrev, EF_FAT_END_OF_CHAIN );
    }
    if ( EF_RET_OK != eRetVal )
    {
      ; /* Error */
    }
    else
    {
      ef_u32_t  nxt;
    #if ( 0 != EF_CONF_USE_TRIM )
      ef_u32_t  scl = u32Cluster;
      ef_u32_t  ecl = u32Cluster;
    #endif

      /* Remove the chain */
      do
      {
        /* Get cluster status */
        eRetVal = eEFPrvFATGet(pxObject, u32Cluster, &nxt );
        /* Empty cluster? */
        if ( 0 == nxt )
        {
          break;
        }
        /* Else if an error */
        else if ( EF_RET_OK != eRetVal )
        {
          return eRetVal;
        }
        /* Mark the cluster 'free' on the FAT */
        eRetVal = eEFPrvFATSet( pxFS, u32Cluster, 0 );
        if ( EF_RET_OK != eRetVal )
        {
          return eRetVal;
        }
    //    else
    //    {
        if ( pxFS->u32ClstFreeNb < ( pxFS->u32FatEntriesNb - 2 ) )
        {  /* Update FSINFO */
          pxFS->u32ClstFreeNb++;
          pxFS->u8FsInfoFlags |= 1;
        }
    #if EF_CONF_USE_TRIM
        /* Is next cluster contiguous? */
        if ( ecl + 1 == nxt )
        {  /* Is next cluster contiguous? */
          ecl = nxt;
        }
        else
        {        /* End of contiguous cluster block */
          ef_lba_t rt[ 2 ];
          ef_lba_t xSector;
          /* Start of data area to be freed */
          (void) eEFPrvFATClusterToSector( pxFS, scl, &xSector );
          rt[ 0 ] = xSector;
          /* End of data area to be freed */
          (void) eEFPrvFATClusterToSector( pxFS, ecl, &xSector );
          rt[ 1 ] = xSector + pxFS->u8ClstSize - 1;
          /* Inform storage device that the data in the block may be erased */
           eEFPrvDriveIOCtrl( pxFS->u8PhysDrv, CTRL_TRIM, rt );
          scl = nxt;
          ecl = nxt;
        }
    #endif
        /* Next cluster */
        u32Cluster = nxt;
      /* Repeat while not the last link */
      } while ( u32Cluster < pxFS->u32FatEntriesNb );

      eRetVal = EF_RET_OK;
    }
  }
  return eRetVal;
}
#endif

/* FAT handling - Remove a cluster chain */
ef_return_et eEFPrvFATChainRemove (
  ef_object_st  * pxObject,
  ef_u32_t        u32Cluster,
  ef_u32_t        u32ClusterPrev
)
{
  EF_ASSERT_PRIVATE( 0 != pxObject );

  ef_return_et  eRetVal = EF_RET_OK;
  ef_fs_st *pxFS = pxObject->pxFS;

  /* Check if in valid range */
  if ( EF_RET_OK != eEFPrvFATClusterNbCheck( pxFS->u32FatEntriesNb, u32Cluster ) )
  {
    eRetVal = EF_RET_INT_ERR;
  }
  else
  {
    /* Mark the previous cluster 'EOC' on the FAT if it exists */
    if ( 0 != u32ClusterPrev )
    {
      eRetVal = eEFPrvFATSet( pxFS, u32ClusterPrev, EF_FAT_END_OF_CHAIN );
    }
    if ( EF_RET_OK != eRetVal )
    {
      ; /* Error */
    }
    else
    {
      ef_u32_t  nxt;
    #if ( 0 != EF_CONF_USE_TRIM )
      ef_u32_t  scl = u32Cluster;
      ef_u32_t  ecl = u32Cluster;
    #endif

      /* Remove the chain */
      do
      {
        /* Get cluster status */
        eRetVal = eEFPrvFATGet( pxFS, u32Cluster, &nxt );
        /* Empty cluster? */
        if ( 0 == nxt )
        {
          break;
        }
        /* Else if an error */
        else if ( EF_RET_OK != eRetVal )
        {
          return eRetVal;
        }
        /* Mark the cluster 'free' on the FAT */
        eRetVal = eEFPrvFATSet( pxFS, u32Cluster, 0 );
        if ( EF_RET_OK != eRetVal )
        {
          return eRetVal;
        }
    //    else
    //    {
        if ( pxFS->u32ClstFreeNb < ( pxFS->u32FatEntriesNb - 2 ) )
        {  /* Update FSINFO */
          pxFS->u32ClstFreeNb++;
          pxFS->u8FsInfoFlags |= 1;
        }
    #if EF_CONF_USE_TRIM
        /* Is next cluster contiguous? */
        if ( ecl + 1 == nxt )
        {  /* Is next cluster contiguous? */
          ecl = nxt;
        }
        else
        {        /* End of contiguous cluster block */
          ef_lba_t rt[ 2 ];
          ef_lba_t xSector;
          /* Start of data area to be freed */
          (void) eEFPrvFATClusterToSector( pxFS, scl, &xSector );
          rt[ 0 ] = xSector;
          /* End of data area to be freed */
          (void) eEFPrvFATClusterToSector( pxFS, ecl, &xSector );
          rt[ 1 ] = xSector + pxFS->u8ClstSize - 1;
          /* Inform storage device that the data in the block may be erased */
          (void) eEFPrvDriveIOCtrl( pxFS->u8PhysDrv, CTRL_TRIM, rt );
          scl = nxt;
          ecl = nxt;
        }
    #endif
        /* Next cluster */
        u32Cluster = nxt;
      /* Repeat while not the last link */
      } while ( u32Cluster < pxFS->u32FatEntriesNb );

      eRetVal = EF_RET_OK;
    }
  }
  return eRetVal;
}

ef_return_et eEFPrvFATChainCreate (
  ef_object_st  * pxObject,
  ef_u32_t      * pu32Cluster
)
{
  EF_ASSERT_PRIVATE( 0 != pxObject );
  EF_ASSERT_PRIVATE( 0 != pu32Cluster );

  ef_return_et  eRetVal = EF_RET_OK; /* By default, FAT is full */
  ef_fs_st    * pxFS = pxObject->pxFS;

  /* If there are no free clusters */
  if ( 0 == pxFS->u32ClstFreeNb )
  {
    eRetVal = EF_RET_FAT_FULL;
  }
  else
  {
    /* GET THE CLUSTER FROM WHERE TO START SEARCHING BEGIN */
    /* If we create a new chain */
    /* Suggested cluster to start to find */
    ef_u32_t  u32Cluster = pxFS->u32ClstLast;
    /* If    last cluster is before the beginning of the FAT
     *    OR last cluster is after the end of the FAT
     */
    if ( EF_RET_OK != eEFPrvFATClusterNbCheck( pxFS->u32FatEntriesNb, u32Cluster ) )
    {
      /* Start searching from beginning of the FAT */
      u32Cluster = 2;
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
    /* GET THE CLUSTER FROM WHERE TO START SEARCHING END */

    /* SEARCH FOR A FREE CLUSTER BEGIN */
    ef_u32_t  u32ClusterNew = 0;
    /* If finding a free cluster starting from cluster start failed */
    if ( EF_RET_OK != eEFPrvFATClusterFindFree( pxObject, u32Cluster, &u32ClusterNew ) )
    {
      /* No Free Cluster found */
      eRetVal = EF_RET_FAT_FULL;
    }
    /* Else, if marking the new cluster as end of chain 'EOC' failed */
    else if ( EF_RET_OK != eEFPrvFATSet( pxFS, u32ClusterNew, EF_FAT_END_OF_CHAIN) )
    {
      /* No Free Cluster found */
      eRetVal = EF_RET_INT_ERR;
    }
    /* Else, all function succeeded. */
    else
    {
      /* Update FSINFO. */
      pxFS->u32ClstLast = u32ClusterNew;
      if ( pxFS->u32ClstFreeNb <= ( pxFS->u32FatEntriesNb - 2 ) )
      {
        pxFS->u32ClstFreeNb--;
      }
      pxFS->u8FsInfoFlags |= 1;
      /* Return new cluster numbers */
      *pu32Cluster = u32ClusterNew;
      /* Function completed successfully */
      eRetVal = EF_RET_OK;
    }
    /* SEARCH FOR A FREE CLUSTER END */

  } /* If there are free clusters */

  return eRetVal;
}

ef_return_et eEFPrvFATChainStretch (
  ef_object_st  * pxObject,
  ef_u32_t        u32Cluster,
  ef_u32_t      * pu32Cluster
)
{
  EF_ASSERT_PRIVATE( 0 != pxObject );
  EF_ASSERT_PRIVATE( 0 != pu32Cluster );

  ef_return_et    eRetVal = EF_RET_FAT_FULL; /* By default, FAT is full */
  ef_fs_st      * pxFS = pxObject->pxFS;

  ef_u32_t  u32ClusterStart = 0;
  ef_u32_t  u32ClusterValue = 0;

  /* GET THE CLUSTER FROM WHERE TO START SEARCHING BEGIN */
  /* Stretch a chain */
  /* If getting the cluster status failed */
  if ( EF_RET_OK != eEFPrvFATGet( pxFS, u32Cluster, &u32ClusterValue ) )
  {
    /* There is an eror */
    eRetVal = EF_RET_INT_ERR;
  }
  /* Else, if we have an invalid linked cluster */
  else if ( 2 > u32ClusterValue )
  {
    /* There is an eror */
    eRetVal = EF_RET_INT_ERR;
  }
  /* Else, if we have a valid linked cluster */
  else if ( pxFS->u32FatEntriesNb >= u32ClusterValue )
  {
    ef_u32_t  u32ClusterNew = u32ClusterValue;
    /* If getting the next cluster status failed */
    if ( EF_RET_OK != eEFPrvFATGet( pxFS, u32ClusterNew, &u32ClusterValue ) )
    {
      /* There is an eror */
      eRetVal = EF_RET_INT_ERR;
    }
    /* Else, if we have an invalid linked cluster */
    else if ( 2 > u32ClusterValue )
    {
      /* There is an eror */
      eRetVal = EF_RET_INT_ERR;
    }
    else
    {
      /* Function completed succesfully */
      eRetVal = EF_RET_OK;
      /* Return next cluster numbers */
      *pu32Cluster = u32ClusterNew;
    }
  }
  /* Else we have an End Of Chain cluster */
  /* If there are free clusters */
  if ( 0 != pxFS->u32ClstFreeNb )
  //if ( pxFS->u32FatEntriesNb < u32ClusterValue )
  {
    /* Stretching an existing chain? */
    /* Suggested cluster to start to find */
    u32ClusterStart = pxFS->u32ClstLast;
    /* If    last cluster is null
     *    OR last cluster is not known
     */
    if ( EF_RET_OK == eEFPrvFATClusterNbCheck( pxFS->u32FatEntriesNb, u32Cluster ) )
    {
      /* Start searching from beggining of the FAT */
      u32ClusterStart = 2;
    }
    /* SEARCH FOR A FREE CLUSTER BEGIN */
    ef_u32_t  u32ClusterNew = 0;
    /* If we found a free cluster starting from cluster start */
    if ( EF_RET_OK != eEFPrvFATClusterFindFree( pxObject, u32ClusterStart, &u32ClusterNew ) )
    {
      ; /* No Free Cluster found */
    }
    /* Else, if marking the new cluster as end of chain 'EOC' */
    else if ( EF_RET_OK != eEFPrvFATSet( pxFS, u32ClusterNew, EF_FAT_END_OF_CHAIN) )
    {
      ; /* No Free Cluster found */
    }
    /* Else, if linking it from the previous one if needed */
    else if ( EF_RET_OK != eEFPrvFATSet( pxFS, u32Cluster, u32ClusterNew ) )
    {
      ; /* No Free Cluster found */
    }
    /* Else, all function succeeded. */
    else
    {
      /* Update FSINFO. */
      pxFS->u32ClstLast = u32ClusterNew;
      if ( pxFS->u32ClstFreeNb <= ( pxFS->u32FatEntriesNb - 2 ) )
      {
        pxFS->u32ClstFreeNb--;
      }
      pxFS->u8FsInfoFlags |= 1;
      /* Function completed succesfully */
      eRetVal = EF_RET_OK;
      /* Return new cluster numbers */
      *pu32Cluster = u32ClusterNew;
    }
    /* SEARCH FOR A FREE CLUSTER END */
  }
  else //if ( pxFS->u32FatEntriesNb < u32ClusterValue )
  {
    /* There is an eror */
    eRetVal = EF_RET_INT_ERR;
  }

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
