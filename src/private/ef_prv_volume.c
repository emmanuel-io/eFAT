/**
 * ********************************************************************************************************************
 *  @file     ef_prv_volume.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private volume access variables and functions.
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
#include "ef_port_diskio.h"
#include <ef_prv_def.h>
#include <ef_prv_def_gpt.h>
#include <ef_prv_fat.h>
#include "ef_prv_drive.h"
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
#include "ef_prv_def_bpb_fat.h"
#include "ef_prv_def_mbr.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */

/* Local function prototypes---------------------------------------------------------------------------------------- */

/**
 *  @brief  Load a sector and check if it is an FAT VBR
 *
 *  @param  pxFS    Pointer to the Filesystem object
 *  @param  xSector Sector to load and check if it is an FAT-VBR or not
 *
 *  @return The type of sector found
 *  @retval EF_SECTOR_TYPE_VBR_FAT16    Volume Boot Record FAT
 *  @retval EF_SECTOR_TYPE_BS_UNKNOWN   Boot Sector Valid but not FAT
 *  @retval EF_SECTOR_TYPE_BS_INVALID   Boot Sector Invalid
 *  @retval EF_SECTOR_TYPE_DISK_ERROR   Disk error
*/
static  ef_sector_type_et  eEFPrvFSSectorCheck (
  ef_fs_st  * pxFS,
  ef_lba_t    xSector
);

/**
 *  @brief  Find an FAT volume in GPT
 *          (It supports only generic partitioning rules, MBR, GPT and SFD)
 *
 *  @param  pxFS        Pointer to the found filesystem object
 *  @param  u8Partition Partition to find
 *                      1..128: Try in GPT partioning scheme
 *                      255:    Try to find the first one we can handle.
 *
 *  @return Returns BS status found in the hosting drive
 *  @retval EF_SECTOR_TYPE_VBR_FAT16    Volume Boot Record FAT12/16
 *  @retval EF_SECTOR_TYPE_VBR_FAT32    Volume Boot Record FAT32
 *  @retval EF_SECTOR_TYPE_BS_UNKNOWN   Boot Sector Valid but not FAT VBR
 *  @retval EF_SECTOR_TYPE_BS_INVALID   Boot Sector Invalid
 *  @retval EF_SECTOR_TYPE_DISK_ERROR   Disk error
 */

static  ef_sector_type_et eEFPrvVolumeFindInGPT (
  ef_fs_st  * pxFS,
  ef_u08_t    u8Partition
);

/**
 *  @brief  Find an FAT volume in MBR
 *          (It supports only generic partitioning rules, MBR, GPT and SFD)
 *
 *  @param  pxFS        Pointer to the found filesystem object
 *  @param  u8Partition Partition to find
 *                      1..4:   Try in MBR partioning scheme
 *                      255:    Try to find the first one we can handle.
 *
 *  @return Returns BS status found in the hosting drive
 *  @retval EF_SECTOR_TYPE_VBR_FAT16    Volume Boot Record FAT12/16
 *  @retval EF_SECTOR_TYPE_VBR_FAT32    Volume Boot Record FAT32
 *  @retval EF_SECTOR_TYPE_BS_UNKNOWN   Boot Sector Valid but not FAT VBR
 *  @retval EF_SECTOR_TYPE_BS_INVALID   Boot Sector Invalid
 *  @retval EF_SECTOR_TYPE_DISK_ERROR   Disk error
 */

static  ef_sector_type_et eEFPrvVolumeFindInMBR (
  ef_fs_st  * pxFS,
  ef_u08_t    u8Partition
);


/**
 *  @brief  Generate random value
 *
 *  @param  u32Seed     Seed value
 *  @param  pu8Buffer   Pointer to the output buffer
 *  @param  u32Length   Buffer Length
 *
 *  @return Random 32 bits Value
 */
static  ef_u32_t u32ffRandMake (
  ef_u32_t    u32Seed,
  ef_u08_t  * pu8Buffer,
  ef_u32_t    u32Length
);


/* Local functions ------------------------------------------------------------------------------------------------- */

/* Check what the sector is */
static  ef_sector_type_et  eEFPrvFSSectorCheck (
  ef_fs_st  * pxFS,
  ef_lba_t    xSector
)
{
  /* By default, Valid BS but not FAT */
  ef_sector_type_et eRetVal = EF_SECTOR_TYPE_BS_UNKNOWN;
  /* Invalidate window */
  pxFS->u8WinFlags = 0;
  pxFS->xWindowSector = (ef_lba_t)0 - 1;
  /* Load the boot sector */
  if ( EF_RET_OK != eEFPrvFSWindowLoad( pxFS, xSector ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_SECTOR_TYPE_DISK_ERROR );
  }
  /* Check boot signature (always here regardless of the sector size or whether MBR of VBR) */
  else if ( 0xAA55 != u16EFPortLoad( pxFS->pu8Window + EF_BS_OFFSET_SIGNATURE ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_SECTOR_TYPE_BS_INVALID );
  }
  /* Valid JumpBoot code? AND FAT VBR */
  else if (    ( 0 != EF_CONF_FS_CHECK_FAT16 )
            && (    ( 0xE9 == pxFS->pu8Window[ EF_BS_OFFSET_JMP_INST ] )
                 || ( 0xEB == pxFS->pu8Window[ EF_BS_OFFSET_JMP_INST ] )
                 || ( 0xE8 == pxFS->pu8Window[ EF_BS_OFFSET_JMP_INST ] ) )
            && (    ( EF_RET_OK == eEFPortMemCompare( pxFS->pu8Window + EF_BS_EBPB_FAT16_OFFSET_FS_TYPE,
                                                      "FAT", 3 ) ) ) )
  {
    eRetVal = EF_SECTOR_TYPE_VBR_FAT16;
  }
  /* Valid JumpBoot code? AND FAT32 VBR */
  else if (    ( 0 != EF_CONF_FS_CHECK_FAT32 )
            && (    ( 0xE9 == pxFS->pu8Window[ EF_BS_OFFSET_JMP_INST ] )
                 || ( 0xEB == pxFS->pu8Window[ EF_BS_OFFSET_JMP_INST ] )
                 || ( 0xE8 == pxFS->pu8Window[ EF_BS_OFFSET_JMP_INST ] ) )
            && (    ( EF_RET_OK == eEFPortMemCompare( pxFS->pu8Window + EF_BS_EBPB_FAT32_OFFSET_FS_TYPE,
                                                      "FAT32", 5 ) ) ) )
  {
    eRetVal = EF_SECTOR_TYPE_VBR_FAT32;
  }
  else if ( 0xEE == pxFS->pu8Window[  EF_MBR_OFFSET_PTE_ARRAY
                                    + EF_MBR_PTE_OFFSET_PARTITION_TYPE ] )
  {
    /* Protective MBR */
    eRetVal = EF_SECTOR_TYPE_PROTECTIVE_MBR;
  }
  else
  {
    /* Valid BS but not FAT */
    eRetVal = EF_SECTOR_TYPE_BS_UNKNOWN;
  }

  return eRetVal;
}

/* Find an FAT volume in GPT partitioning */
static  ef_sector_type_et eEFPrvVolumeFindInGPT (
  ef_fs_st  * pxFS,
  ef_u08_t    u8Partition
)
{
  /* By default, Valid BS is invalid */
  ef_sector_type_et eRetVal = EF_SECTOR_TYPE_BS_INVALID;

  /* Load GPT header sector (next to MBR) */
  if ( EF_RET_OK != eEFPrvFSWindowLoad( pxFS, 1 ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_SECTOR_TYPE_DISK_ERROR );
  }
  /* Check if GPT header is valid */
  else if ( EF_RET_OK != eEFPrvGPTHeaderTest( pxFS->pu8Window ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_SECTOR_TYPE_BS_INVALID );
  }
  else
  {
    ef_u32_t  u32EntryNb = 0;
    /* Number of partition entries in array */
    ef_u32_t  u32EntriesNb    = u32EFPortLoad( pxFS->pu8Window + EF_GPT_HEADER_OFFSET_PTE_NB );
    /* Size of single partition entry */
    ef_u32_t  u32PTESize      = u32EFPortLoad( pxFS->pu8Window + EF_GPT_HEADER_OFFSET_PTE_SIZE );
    /* Table location */
    ef_u64_t  u64PartitionLBA = u64EFPortLoad( pxFS->pu8Window + EF_GPT_HEADER_OFFSET_PTE_LBA_START );
    /* Find FAT partition */
    /* If auto search */
    if ( PARTITION_FIND_FIRST == u8Partition )
    {
      for ( ef_u32_t i = 0 ; i < u32EntriesNb ; i++ )
      {
        ef_u64_t  u64SectorLBA =   u64PartitionLBA
                                 + ( ( i * u32PTESize ) / EF_SECTOR_SIZE( pxFS ) );
        if ( EF_RET_OK != eEFPrvFSWindowLoad( pxFS, u64SectorLBA ) )
        {
          /* PT sector */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_SECTOR_TYPE_DISK_ERROR );
          break;
        }
        else
        {
          /* By default, Not found */
          eRetVal = EF_SECTOR_TYPE_BS_INVALID;
          /* Offset in the sector */
          ef_u32_t u32Offset = i * u32PTESize % EF_SECTOR_SIZE( pxFS );
          /* MS basic data partition? */
          if ( EF_RET_OK == eEFPortMemCompare(    pxFS->pu8Window + u32Offset
                                            + EF_GPT_PTE_OFFSET_TYPE_GUID,
                                           u8GUIDBasicDataPartition, 16 ) )
          {
            u32EntryNb++;
            /* Load VBR and check status */
            eRetVal =  eEFPrvFSSectorCheck( pxFS,
                                      u64EFPortLoad(   pxFS->pu8Window + u32Offset
                                               + EF_GPT_PTE_OFFSET_LBA_FIRST ) );
            /* Valid FAT volume found first) */
            if (    (    ( 0 != EF_FS_FAT16 )
                      && ( EF_SECTOR_TYPE_VBR_FAT16 == eRetVal ) )
                 || (    ( 0 != EF_FS_FAT32 )
                      && ( EF_SECTOR_TYPE_VBR_FAT32 == eRetVal ) ) )
            {
              /* Update partition number if it is a FAT VBR */
              pxFS->u8Partition = u8Partition;
              break;
            }
          }
          else
          {
            ; /* Code compliance */
          }
        }
      } /* for ( i = 0 ; i < u32EntriesNb ; i++ ) */
    } /* if ( PARTITION_FIND_FIRST == u8Partition ) */
    else if ( 0 == u8Partition )
    {
      /* Impossible in GPT Scheme */
    }
    else if ( u32EntriesNb < u8Partition )
    {
      /* We are trying to look a partition which doesn't exist. */
    }
    else
    {
      ef_u64_t  u64SectorLBA =   u64PartitionLBA
                               + (   (   ((ef_u64_t) ( u8Partition - 1 ))
                                       * ((ef_u64_t) u32PTESize) )
                                   / ((ef_u64_t) EF_SECTOR_SIZE( pxFS )) );
      if ( EF_RET_OK != eEFPrvFSWindowLoad(  pxFS, u64SectorLBA ) )
      {
        /* PT sector */
        eRetVal = EF_RETURN_CODE_HANDLER( EF_SECTOR_TYPE_DISK_ERROR );
      }
      else
      {
        /* By default, Not found */
        eRetVal = EF_SECTOR_TYPE_BS_INVALID;
        /* Offset in the sector */
        ef_u32_t u32Offset = (ef_u32_t) ( u8Partition - 1 ) * u32PTESize % EF_SECTOR_SIZE( pxFS );
        /* MS basic data partition? */
        if ( EF_RET_OK == eEFPortMemCompare(    pxFS->pu8Window + u32Offset
                                              + EF_GPT_PTE_OFFSET_TYPE_GUID,
                                             u8GUIDBasicDataPartition,
                                             16 ) )
        {
          /* Load VBR and check status */
          eRetVal =  eEFPrvFSSectorCheck(  pxFS,
                                          u64EFPortLoad(   pxFS->pu8Window + u32Offset
                                               + EF_GPT_PTE_OFFSET_LBA_FIRST ) );
          if (    (    ( 0 != EF_FS_FAT16 )
                    && ( EF_SECTOR_TYPE_VBR_FAT16 == eRetVal ) )
               || (    ( 0 != EF_FS_FAT32 )
                    && ( EF_SECTOR_TYPE_VBR_FAT32 == eRetVal ) ) )
          {
            /* Update partition number if it is a FAT VBR */
            pxFS->u8Partition = u8Partition;
          }
        }
      }
    }
  }

  return eRetVal;
}

/* Find an FAT volume in MBR partitioning */
static  ef_sector_type_et eEFPrvVolumeFindInMBR (
  ef_fs_st  * pxFS,
  ef_u08_t    u8Partition
)
{
  /* By default, Valid BS but not FAT */
  ef_sector_type_et eRetVal = EF_SECTOR_TYPE_BS_UNKNOWN;
  ef_u32_t              i;

  /* Load sector 0 and check if it is an FAT VBR as SFD */
  eRetVal =  eEFPrvFSSectorCheck( pxFS, 0 );
  /* If we want to try for No partitioning scheme */
  /* Else if we can handle multipartion and have a partition of more than 4 */
  if (    ( PARTITION_FIND_FIRST != u8Partition )
       && (    ( 0 == u8Partition )
            || ( 4 < u8Partition ) ) )
  {
    /* MBR has 4 partitions ( 1, 2, 3 & 4) */
    eRetVal = EF_SECTOR_TYPE_BS_INVALID;
  }
  /* Else we are ready to go in the MBR, if partition is forced */
  else if ( PARTITION_FIND_FIRST != u8Partition )
  {
    eRetVal = EF_SECTOR_TYPE_BS_INVALID;
    /* Get partition offsets base in the MBR */
    ef_u08_t * pu8Address =    pxFS->pu8Window
                            + EF_MBR_OFFSET_PTE_ARRAY
                            + EF_MBR_PTE_OFFSET_LBA_START
                            + ( ( u8Partition - 1 ) * EF_MBR_PTE_SIZE );
    /* Load partition offsets in the MBR */
    ef_u32_t u32MBRPt = u32EFPortLoad( pu8Address );
    /* Get next partition offset in the MBR */
    /* Check if the forced partition is FAT */
    if ( 0 != u32MBRPt )
    {
      eRetVal =  eEFPrvFSSectorCheck( pxFS, u32MBRPt );
      if (    (    ( 0 != EF_FS_FAT16 )
                && ( EF_SECTOR_TYPE_VBR_FAT16 == eRetVal ) )
           || (    ( 0 != EF_FS_FAT32 )
                && ( EF_SECTOR_TYPE_VBR_FAT32 == eRetVal ) ) )
      {
        /* Update partition number if it is a FAT VBR */
        pxFS->u8Partition = u8Partition;
      }
    }
  }
  else
  {
    ef_u32_t  u32MBRPt[ 4 ];
    /* Get partition offsets base in the MBR */
    ef_u08_t * pu8Address =    pxFS->pu8Window
                            + EF_MBR_OFFSET_PTE_ARRAY
                            + EF_MBR_PTE_OFFSET_LBA_START;
    /* Load partition offsets in the MBR */
    for ( i = 0 ; i < 4 ; i++ )
    {
      u32MBRPt[ i ] = u32EFPortLoad( pu8Address );
      /* Get next partition offset in the MBR */
      pu8Address += EF_MBR_PTE_SIZE;
    }
    /* Auto find a FAT volume */
    for ( i = 0 ; i < 4 ; i++ )
    {
      eRetVal = EF_SECTOR_TYPE_BS_INVALID;
      /* Check if the partition is FAT */
      if ( 0 != u32MBRPt[ i ] )
      {
        eRetVal =  eEFPrvFSSectorCheck( pxFS, u32MBRPt[ i ] );
        /* Valid FAT volume found first) */
        if (    (    ( 0 != EF_FS_FAT16 )
                  && ( EF_SECTOR_TYPE_VBR_FAT16 == eRetVal ) )
             || (    ( 0 != EF_FS_FAT32 )
                  && ( EF_SECTOR_TYPE_VBR_FAT32 == eRetVal ) ) )
        {
          /* Update partition number if it is a FAT VBR */
          pxFS->u8Partition = (ef_u08_t) ++i;
          break;
        }
      }
    }
  }

  return eRetVal;
}

static  ef_u32_t u32ffRandMake (
  ef_u32_t    u32Seed,
  ef_u08_t  * pu8Buffer,
  ef_u32_t    u32Length
)
{

  if ( 0 == u32Seed )
  {
    u32Seed = 1;
  }
  do {
    for ( ef_u32_t r = 0 ; r < 8 ; r++ )
    {
      if ( 0 != ( u32Seed & 1 ) )
      {
        u32Seed = u32Seed >> 1 ^ 0xA3000000;  /* Shift 8 bits the 32-bit LFSR */
      }
      else
      {
        u32Seed = u32Seed >> 1;  /* Shift 8 bits the 32-bit LFSR */
      }
    }
    *pu8Buffer++ = (ef_u08_t)u32Seed;
  } while ( 0 != (--u32Length ) );
  return u32Seed;
}

/* Public functions prototypes --------------------------------------------- */

/* Find a volume */
ef_sector_type_et eEFPrvVolumeFind (
  ef_fs_st  * pxFS,
  ef_u08_t    u8Partition
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );

  ef_sector_type_et eRetVal = EF_SECTOR_TYPE_BS_UNKNOWN;

  pxFS->u8Partition = PARTITION_FIND_FIRST;

  /* Load sector 0 and check if it is an FAT VBR as SFD */
  eRetVal =  eEFPrvFSSectorCheck( pxFS, 0 );
  /* If we want to try for No partitioning scheme */
  if ( 0 == u8Partition )
  {
    if (    (    ( 0 != EF_FS_FAT16 )
              && ( EF_SECTOR_TYPE_VBR_FAT16 == eRetVal ) )
         || (    ( 0 != EF_FS_FAT32 )
              && ( EF_SECTOR_TYPE_VBR_FAT32 == eRetVal ) ) )
    {
      /* Update partition number if it is a FAT VBR without partitioning */
      pxFS->u8Partition = 0;
    }
    ; /* Returns immediately */
  }
  /* Else if we are in autodetect mode and we found a partition in SFD scheme */
  else if (    ( PARTITION_FIND_FIRST == u8Partition )
            && (   (    ( 0 != EF_FS_FAT16 )
                     && ( EF_SECTOR_TYPE_VBR_FAT16 == eRetVal ) )
                || (    ( 0 != EF_FS_FAT32 )
                     && ( EF_SECTOR_TYPE_VBR_FAT32 == eRetVal ) ) ) )
  {
    /* Update partition number if it is a FAT VBR without partitioning */
    pxFS->u8Partition = 0;
    ; /* Returns immediately */
  }
  /* Sector 0 is not an FAT VBR or forced partition number wants a partition */
  /* Else if we can handle and have a GPT protective MBR */
  else if (    ( 0 != EF_CONF_USE_GPT )
            && ( EF_SECTOR_TYPE_PROTECTIVE_MBR == eRetVal ) )
  {
    eRetVal = eEFPrvVolumeFindInGPT( pxFS, u8Partition );
    /* Partition number updating if it is a FAT VBR is done in the function */
  }
  /* Else if we can handle a standard MBR */
  else if ( 0 != EF_CONF_USE_MBR )
  {
    eRetVal = eEFPrvVolumeFindInMBR( pxFS, u8Partition );
    /* Partition number updating if it is a FAT VBR is done in the function */
  }
  else
  {
    ; /* Returns immediately */
  }

  return eRetVal;
}

/* Create partitions on the physical drive */
ef_return_et eEFPrvPartitionCreate (
  ef_u08_t     u8PhyDrvNb,   /* Physical drive number */
  const ef_lba_t   plst[ ],   /* Partition list */
  ef_u32_t     sys,             /* System ID (for only MBR, temp setting) and bit8:GPT */
  ef_u08_t   * buf          /* Working buffer for a sector */
)
{
  ef_u32_t i, cy;
  ef_lba_t sz_drv;
  ef_u32_t sz_drv32, s_lba32, n_lba32;
  ef_u08_t *pte, hd, n_hd, sc, n_sc;

  /* Get drive size */
  if ( eEFPrvDriveIOCtrl(u8PhyDrvNb, GET_SECTOR_COUNT, &sz_drv) != EF_RET_OK)
  {
    return EF_RET_DISK_ERR;
  }

#if EF_CONF_LBA64
  /* Create partitions in GPT */
  if (sz_drv >= EF_CONF_GPT_MIN)
  {
    ef_u16_t ss;
    ef_u32_t sz_pt, pi, si, ofs;
    ef_u32_t bcc, rnd, uiDataAlign;
    ef_u64_t s_lba64, n_lba64, sz_pool, s_bpt;
    static const ef_u08_t gpt_mbr[16] = {0x00, 0x00, 0x02, 0x00, 0xEE, 0xFE, 0xFF, 0x00, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};

#if EF_CONF_SS_MAX != EF_CONF_SS_MIN
    if ( EF_RET_OK !=  eEFPrvDriveIOCtrl( u8PhyDrvNb, GET_SECTOR_SIZE, &ss ) )
    {
      return EF_RET_DISK_ERR;  /* Get sector size */
    }
    if (ss > EF_CONF_SS_MAX || ss < EF_CONF_SS_MIN || (ss & (ss - 1)))
    {
      return EF_RET_DISK_ERR;
    }
#else
    ss = EF_CONF_SS_MAX;
#endif
    rnd = EF_FATTIME_GET();      /* Random seed */
    uiDataAlign = GPT_ALIGN / ss;      /* Partition alignment [sector] */
    sz_pt = GPT_ITEMS * EF_GPT_PTE_SIZE / ss;  /* Size of PT [sector] */
    s_bpt = sz_drv - sz_pt - 1;    /* Backup PT start sector */
    s_lba64 = 2 + sz_pt;      /* First allocatable sector */
    sz_pool = s_bpt - s_lba64;    /* Size of allocatable area */
    bcc = 0xFFFFFFFF; n_lba64 = 1;
    pi = si = 0;  /* partition table index, size table index */
    do {
      if (pi * EF_GPT_PTE_SIZE % ss == 0)
      {
        /* Clean the buffer if needed */
        eEFPortMemZero( buf, ss );
      }
      /* Is the size table not termintated? */
      if ( 0 != n_lba64 )
      {
        s_lba64 = (s_lba64 + uiDataAlign - 1) & ((ef_u64_t)0 - uiDataAlign);  /* Align partition start */
        n_lba64 = plst[si++];  /* Get a partition size */
        /* Is the size in percentage? */
        if (n_lba64 <= 100)
        {
          n_lba64 = sz_pool * n_lba64 / 100;
          n_lba64 = (n_lba64 + uiDataAlign - 1) & ((ef_u64_t)0 - uiDataAlign);  /* Align partition end (only if in percentage) */
        }
        /* Clip at end of the pool */
        if (s_lba64 + n_lba64 > s_bpt)
        {
          n_lba64 = (s_lba64 < s_bpt) ? s_bpt - s_lba64 : 0;
        }
      }
      /* Add a partition? */
      if ( 0 != n_lba64 )
      {
        ofs = pi * EF_GPT_PTE_SIZE % ss;
        /* Partition GUID (Microsoft Basic Data) */
        eEFPortMemCopy( u8GUIDBasicDataPartition,
                    buf + ofs + EF_GPT_PTE_OFFSET_TYPE_GUID,
                    16 );
        rnd = u32ffRandMake( rnd, buf + ofs + EF_GPT_PTE_OFFSET_UNIQUE_GUID, 16 );    /* Unique partition GUID */
        vEFPortStoreu64( buf + ofs + EF_GPT_PTE_OFFSET_LBA_FIRST, s_lba64 );        /* Partition start LBA */
        vEFPortStoreu64( buf + ofs + EF_GPT_PTE_OFFSET_LBA_LAST, s_lba64 + n_lba64 - 1 );  /* Partition end LBA */
        s_lba64 += n_lba64;    /* Next partition LBA */
      }
      /* Write the buffer if it is filled up */
      if ( (pi + 1) * EF_GPT_PTE_SIZE % ss == 0 )
      {
        for ( i = 0 ; i < ss ; bcc = u32ffCRC32( bcc, buf[i++] ) ) ;  /* Calculate table check sum */
        /* Primary table */
        if ( EF_RET_OK !=  eEFPrvDriveWrite( u8PhyDrvNb, buf, 2 + pi * EF_GPT_PTE_SIZE / ss, 1 ) )
        {
          return EF_RET_DISK_ERR;
        }
        /* Secondary table */
        if ( EF_RET_OK !=  eEFPrvDriveWrite( u8PhyDrvNb, buf, s_bpt + pi * EF_GPT_PTE_SIZE / ss, 1 ) )
        {
          return EF_RET_DISK_ERR;
        }
      }
    } while (++pi < GPT_ITEMS);

    /* Create primary GPT header */
    eEFPortMemZero( buf, ss );
    /* Signature, version (1.0) and size (92) */
    eEFPortMemCopy( "EFI PART" "\0\0\1\0" "\x5C\0\0",
                buf + EF_GPT_HEADER_OFFSET_SIGNATURE,
                16 );
    vEFPortStoreu32( buf + EF_GPT_HEADER_OFFSET_PTE_ARRAY_CRC32, ~bcc );        /* Table check sum */
    vEFPortStoreu64( buf + EF_GPT_HEADER_OFFSET_HEADER_LBA_CURRENT, 1 );          /* LBA of this header */
    vEFPortStoreu64( buf + EF_GPT_HEADER_OFFSET_HEADER_LBA_BACKUP, sz_drv - 1 );    /* LBA of another header */
    vEFPortStoreu64( buf + EF_GPT_HEADER_OFFSET_PARTITIONS_LBA_FIRST, 2 + sz_pt );      /* LBA of first allocatable sector */
    vEFPortStoreu64( buf + EF_GPT_HEADER_OFFSET_PARTITIONS_LBA_LAST, s_bpt - 1 );      /* LBA of last allocatable sector */
    vEFPortStoreu32( buf + EF_GPT_HEADER_OFFSET_PTE_SIZE, EF_GPT_PTE_SIZE );      /* Size of a table entry */
    vEFPortStoreu32( buf + EF_GPT_HEADER_OFFSET_PTE_NB, GPT_ITEMS );      /* Number of table entries */
    vEFPortStoreu32( buf + EF_GPT_HEADER_OFFSET_PTE_LBA_START, 2 );          /* LBA of this table */
    rnd = u32ffRandMake(rnd, buf + EF_GPT_HEADER_OFFSET_DISK_GUID, 16 );  /* Disk GUID */
    for ( i = 0, bcc= 0xFFFFFFFF; i < 92; bcc = u32ffCRC32( bcc, buf[i++] ) ) ;  /* Calculate header check sum */
    vEFPortStoreu32( buf + EF_GPT_HEADER_OFFSET_HEADER_CRC32, ~bcc );          /* Header check sum */
    if ( EF_RET_OK !=  eEFPrvDriveWrite( u8PhyDrvNb, buf, 1, 1 ) )
    {
      return EF_RET_DISK_ERR;
    }

    /* Create secondary GPT header */
    vEFPortStoreu64( buf + EF_GPT_HEADER_OFFSET_HEADER_LBA_CURRENT, sz_drv - 1 );    /* LBA of this header */
    vEFPortStoreu64( buf + EF_GPT_HEADER_OFFSET_HEADER_LBA_BACKUP, 1 );          /* LBA of another header */
    vEFPortStoreu64( buf + EF_GPT_HEADER_OFFSET_PTE_LBA_START, s_bpt );        /* LBA of this table */
    vEFPortStoreu32( buf + EF_GPT_HEADER_OFFSET_HEADER_CRC32, 0 );
    for ( i = 0, bcc= 0xFFFFFFFF; i < 92; bcc = u32ffCRC32( bcc, buf[i++] ) ) ;  /* Calculate header check sum */
    vEFPortStoreu32( buf + EF_GPT_HEADER_OFFSET_HEADER_CRC32, ~bcc );          /* Header check sum */
    if ( EF_RET_OK !=  eEFPrvDriveWrite( u8PhyDrvNb, buf, sz_drv - 1, 1 ) )
    {
      return EF_RET_DISK_ERR;
    }

    /* Create protective MBR */
    eEFPortMemZero( buf, ss );
    /* Create a GPT partition */
    eEFPortMemCopy( gpt_mbr,
                buf + EF_MBR_OFFSET_PTE_ARRAY,
                16 );
    vEFPortStoreu16( buf + EF_BS_OFFSET_SIGNATURE, 0xAA55 );
    if ( EF_RET_OK !=  eEFPrvDriveWrite( u8PhyDrvNb, buf, 0, 1 ) )
    {
      return EF_RET_DISK_ERR;
    }

  } else
#endif
  {          /* Create partitions in MBR */
    sz_drv32 = (ef_u32_t)sz_drv;
    n_sc = N_SEC_TRACK;
    /* Determine drive CHS without any consideration of the drive geometry */
    for ( n_hd = 8; n_hd != 0 && sz_drv32 / n_hd / n_sc > 1024; n_hd *= 2 ) ;
    if ( n_hd == 0 )
    {
      n_hd = 255;  /* Number of heads needs to be <256 */
    }

    /* Clear MBR */
    eEFPortMemZero( buf, EF_CONF_SECTOR_SIZE );
    /* Partition table in the MBR */
    pte = buf + EF_MBR_OFFSET_PTE_ARRAY;
    for ( i = 0, s_lba32 = n_sc; i < 4 && s_lba32 != 0 && s_lba32 < sz_drv32; i++, s_lba32 += n_lba32 )
    {
      n_lba32 = (ef_u32_t)plst[i];  /* Get partition size */
      if (n_lba32 <= 100)
      {
        n_lba32 = (n_lba32 == 100) ? sz_drv32 : sz_drv32 / 100 * n_lba32;  /* Size in percentage? */
      }
      if (s_lba32 + n_lba32 > sz_drv32 || s_lba32 + n_lba32 < s_lba32)
      {
        n_lba32 = sz_drv32 - s_lba32;  /* Clip at drive size */
      }
      if ( 0 == n_lba32 )
      {
        break;  /* End of table or no sector to allocate? */
      }

      vEFPortStoreu32( pte + EF_MBR_PTE_OFFSET_LBA_START, s_lba32 );    /* Start LBA */
      vEFPortStoreu32( pte + EF_MBR_PTE_OFFSET_LBA_SIZE, n_lba32 );  /* Number of sectors */
      pte[EF_MBR_PTE_OFFSET_PARTITION_TYPE] = (ef_u08_t)sys;      /* System type */

      cy = (ef_u32_t)(s_lba32 / n_sc / n_hd);    /* Start cylinder */
      hd = (ef_u08_t)(s_lba32 / n_sc % n_hd);    /* Start head */
      sc = (ef_u08_t)(s_lba32 % n_sc + 1);    /* Start sector */
      pte[EF_MBR_PTE_OFFSET_CHS_START_HEAD] = hd;
      pte[EF_MBR_PTE_OFFSET_CHS_START_SECTOR] = (ef_u08_t)((cy >> 2 & 0xC0) | sc);
      pte[EF_MBR_PTE_OFFSET_CHS_START_CYLINDER] = (ef_u08_t)cy;

      cy = (ef_u32_t)((s_lba32 + n_lba32 - 1) / n_sc / n_hd);  /* End cylinder */
      hd = (ef_u08_t)((s_lba32 + n_lba32 - 1) / n_sc % n_hd);  /* End head */
      sc = (ef_u08_t)((s_lba32 + n_lba32 - 1) % n_sc + 1);  /* End sector */
      pte[EF_MBR_PTE_OFFSET_CHS_END_HEAD] = hd;
      pte[EF_MBR_PTE_OFFSET_CHS_END_SECTOR] = (ef_u08_t)((cy >> 2 & 0xC0) | sc);
      pte[EF_MBR_PTE_OFFSET_CHS_END_CYLINDER] = (ef_u08_t)cy;

      pte += EF_MBR_PTE_SIZE;    /* Next entry */
    }

    vEFPortStoreu16( buf + EF_BS_OFFSET_SIGNATURE, 0xAA55 );    /* MBR signature */
    if ( EF_RET_OK !=  eEFPrvDriveWrite( u8PhyDrvNb, buf, 0, 1 ) )
    {
      return EF_RET_DISK_ERR;  /* Write it to the MBR */
    }
  }

  return EF_RET_OK;
}


/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
