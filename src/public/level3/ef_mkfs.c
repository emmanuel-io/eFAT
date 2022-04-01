/**
 * ********************************************************************************************************************
 *  @file     ef_mkfs.c
 *  @ingroup  group_eFAT_Public
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Create a FAT volume
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
#include <efat_level3.h>
#include <ef_prv_def.h>
#include <ef_prv_def_bpb_fat.h>
#include <ef_prv_def_gpt.h>
#include <ef_prv_def_mbr.h>
#include <ef_prv_fat.h>
#include "ef_port_diskio.h"
#include "ef_prv_drive.h"
#include "ef_prv_directory.h"
#include "ef_prv_dirfunc.h"
#include "ef_prv_lock.h"
#include "ef_prv_string.h"
#include "ef_prv_volume.h"
#include "ef_prv_volume_nb.h"
#include "ef_prv_gpt.h"
#include "ef_prv_lfn.h"
#include "ef_prv_unicode.h"
#include "ef_port_diskio.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */

/* Limits and boundaries */
#define EF_CLUTER_NB_MAX_FAT12  ( 0x0FF5 )      /**< Max FAT12 clusters (differs from specs, but right for real DOS/Windows behavior) */
#define EF_CLUTER_NB_MAX_FAT16  ( 0xFFF5 )      /**< Max FAT16 clusters (differs from specs, but right for real DOS/Windows behavior) */
#define EF_CLUTER_NB_MAX_FAT32  ( 0x0FFFFFF5 )  /**< Max FAT32 clusters (not specified, practical limit) */

/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/**
 *  This option switches f_mkfs() function. (0:Disable or 1:Enable)
 */
#define EF_USE_MKFS ( 0 )

#if ( 0 != EF_USE_MKFS )
/*-----------------------------------------------------------------------*/
/* Create an FAT volume                                            */
/*-----------------------------------------------------------------------*/

/**
 *  Sectors per track for determination of drive CHS
 */
#define N_SEC_TRACK 63

/**
 *  Alignment of partitions in GPT [byte] (>=128KB)
 */
#define GPT_ALIGN  0x100000

/**
 *  Number of GPT table size (>=128, sector aligned)
 */
#define GPT_ITEMS  128




ef_return_et eEF_mkfs (
  const TCHAR*            pxPath,       /* Logical drive number */
  const ef_mkfs_param_st* pxParameters, /* Format options */
  void*                   work,         /* Pointer to working buffer (null: use heap memory) */
  ef_u32_t                len           /* Size of working buffer [byte] */
)
{
  static const ef_u16_t cst[] = {1, 4, 16, 64, 256, 512, 0};  /* Cluster size boundary for FAT volume (4Ks unit) */
  static const ef_u16_t cst32[] = {1, 2, 4, 8, 16, 32, 0};    /* Cluster size boundary for FAT32 volume (128Ks unit) */
  static const ef_mkfs_param_st xParametersDefault = {FM_ANY, 0, 0, 0, 0};  /* Default parameter */
  ef_u08_t fsopt, fsty, sys, *buf, *pte, u8PhyDrvNb, ipart;
  ef_u16_t ss;  /* Sector size */
  ef_u32_t sz_buf, sz_blk, n_clst, pau, nsect, n;
  ef_lba_t sz_vol, b_vol, b_fat, b_data;    /* Size of volume, Base LBA of volume, fat, data */
  ef_lba_t xSector, lba[2];
  ef_u32_t sz_rsv, sz_fat, sz_dir, sz_au;  /* Size of reserved, fat, pu8Dir, data, cluster */
  ef_u32_t u8FatsNb, u32RootDirNb, i;          /* Index, Number of FATs and Number of root pu8Dir entries */
  int vol;
  ef_u08_t ds;
  ef_return_et fr;


  /* Check mounted drive and clear work area */
  vol = eEFPrvVolumeNbPathRemove( &pxPath );          /* Get target logical drive */
  if (vol < 0)
  {
    return EF_RET_INVALID_DRIVE;
  }
  if (eFAT[vol]) eFAT[vol]->u8FsType = 0;  /* Clear the fs object if mounted */
  u8PhyDrvNb = LD2PD(vol);      /* Physical drive */
  ipart = LD2PT(vol);      /* Partition (0:create as new, 1..:get from partition table) */
  if (!pxParameters)
  {
    pxParameters = &xParametersDefault;  /* Use default parameter if it is not given */
  }

  /* Get physical drive status (sz_drv, sz_blk, ss) */
  ds =  eEFPrvDriveInitialize(u8PhyDrvNb);
  if ( 0 != ( ds & EF_RET_DISK_NOINIT ) )
  {
    return EF_RET_NOT_READY;
  }
  if ( 0 != ( ds & EF_RET_DISK_PROTECT ) )
  {
    return EF_RET_WRITE_PROTECTED;
  }
  sz_blk = pxParameters->u32DataAlign;
  if (sz_blk == 0 &&  eEFPrvDriveIOCtrl(u8PhyDrvNb, GET_BLOCK_SIZE, &sz_blk) != EF_RET_OK)
  {
    sz_blk = 1;
  }
  if (sz_blk == 0 || sz_blk > 0x8000 || (sz_blk & (sz_blk - 1)))
  {
    sz_blk = 1;
  }
#if EF_CONF_SS_MAX != EF_CONF_SS_MIN
  if ( eEFPrvDriveIOCtrl(u8PhyDrvNb, GET_SECTOR_SIZE, &ss) != EF_RET_OK)
  {
    return EF_RET_DISK_ERR;
  }
  if (ss > EF_CONF_SS_MAX || ss < EF_CONF_SS_MIN || (ss & (ss - 1)))
  {
    return EF_RET_DISK_ERR;
  }
#else
  ss = EF_CONF_SS_MAX;
#endif
  /* Options for FAT sub-type and FAT parameters */
  fsopt       = pxParameters->u8Format & (FM_ANY | FM_SFD);
  u8FatsNb    = (pxParameters->u8FatsNb >= 1 && pxParameters->u8FatsNb <= 2) ? pxParameters->u8FatsNb : 1;
  u32RootDirNb = (pxParameters->u32RootDirNb >= 1 && pxParameters->u32RootDirNb <= 32768 && (pxParameters->u32RootDirNb % (ss / EF_DIR_ENTRY_SIZE)) == 0) ? pxParameters->u32RootDirNb : 512;
  sz_au = (pxParameters->u32ClusterSize <= 0x1000000 && (pxParameters->u32ClusterSize & (pxParameters->u32ClusterSize - 1)) == 0) ? pxParameters->u32ClusterSize : 0;
  sz_au /= ss;  /* Byte --> Sector */

  /* Get working buffer */
  sz_buf = len / ss;    /* Size of working buffer [sector] */
  if (sz_buf == 0)
  {
    return EF_RET_NOT_ENOUGH_CORE;
  }
  buf = (ef_u08_t*)work;    /* Working buffer */
#if EF_CONF_VFAT == 3
  if (!buf) buf = ef_memalloc(sz_buf * ss);  /* Use heap memory for working buffer */
#endif
  if (!buf)
  {
    return EF_RET_NOT_ENOUGH_CORE;
  }

  /* Determine where the volume to be located (b_vol, sz_vol) */
  b_vol = sz_vol = 0;
  /* Is the volume associated with any specific partition? */
  if ( ( 0 != ipart ) )
  {
    /* Get partition location from the existing partition table */
    if ( eEFPrvDriveRead(u8PhyDrvNb, buf, 0, 1) != EF_RET_OK)
    {
      LEAVE_MKFS(EF_RET_DISK_ERR);  /* Load MBR */
    }
    if (u16EFPortLoad(buf + EF_BS_OFFSET_SIGNATURE) != 0xAA55)
    {
      LEAVE_MKFS(EF_RET_MKFS_ABORTED);  /* Check if MBR is valid */
    }
#if EF_CONF_LBA64
    /* GPT protective MBR? */
    if (buf[EF_MBR_OFFSET_PTE_ARRAY + EF_MBR_PTE_OFFSET_PARTITION_TYPE] == 0xEE)
    {
      ef_u32_t n_ent, ofs;
      ef_u64_t pt_lba;

      /* Get the partition location from GPT */
      if ( eEFPrvDriveRead(u8PhyDrvNb, buf, 1, 1) != EF_RET_OK)
      {
        LEAVE_MKFS(EF_RET_DISK_ERR);  /* Load GPT header sector (next to MBR) */
      }
      if ( EF_RET_OK != eEFPrvGPTHeaderTest( buf ) )
      {
        LEAVE_MKFS(EF_RET_MKFS_ABORTED);  /* Check if GPT header is valid */
      }
      n_ent = u32EFPortLoad(buf + EF_GPT_HEADER_OFFSET_PTE_NB);    /* Number of entries */
      pt_lba = u64EFPortLoad(buf + EF_GPT_HEADER_OFFSET_PTE_LBA_START);  /* Table start sector */
      ofs = i = 0;
      while (n_ent) {    /* Find MS Basic partition with order of ipart */
        if (ofs == 0 &&  eEFPrvDriveRead(u8PhyDrvNb, buf, pt_lba++, 1) != EF_RET_OK)
        {
          LEAVE_MKFS(EF_RET_DISK_ERR);  /* Get PT sector */
        }
        /* MS basic data partition? */
        if (!effMemCompare(buf + ofs + EF_GPT_PTE_OFFSET_TYPE_GUID, u8GUIDBasicDataPartition, 16) && ++i == ipart)
        {
          b_vol = u64EFPortLoad(buf + ofs + EF_GPT_PTE_OFFSET_LBA_FIRST);
          sz_vol = u64EFPortLoad(buf + ofs + EF_GPT_PTE_OFFSET_LBA_LAST) - b_vol + 1;
          break;
        }
        n_ent--; ofs = (ofs + EF_GPT_PTE_SIZE) % ss;  /* Next entry */
      }
      if (n_ent == 0)
      {
        LEAVE_MKFS(EF_RET_MKFS_ABORTED);  /* Partition not found */
      }
      fsopt |= 0x80;  /* Partitioning is in GPT */
    } else
#endif
    {  /* Get the partition location from MBR partition table */
      pte = buf + (EF_MBR_OFFSET_PTE_ARRAY + (ipart - 1) * EF_MBR_PTE_SIZE);
      if (ipart > 4 || pte[EF_MBR_PTE_OFFSET_PARTITION_TYPE] == 0)
      {
        LEAVE_MKFS(EF_RET_MKFS_ABORTED);  /* No partition? */
      }
      b_vol = u32EFPortLoad(pte + EF_MBR_PTE_OFFSET_LBA_START);    /* Get volume start sector */
      sz_vol = u32EFPortLoad(pte + EF_MBR_PTE_OFFSET_LBA_SIZE);  /* Get volume size */
    }
  }
  else
  {  /* The volume is associated with a physical drive */
    if ( eEFPrvDriveIOCtrl(u8PhyDrvNb, GET_SECTOR_COUNT, &sz_vol) != EF_RET_OK)
    {
      LEAVE_MKFS(EF_RET_DISK_ERR);
    }
    /* To be partitioned? */
    if (!(fsopt & FM_SFD))
    {
      /* Create a single-partition on the drive in this function */
      if (    ( 0 != EF_CONF_LBA64 )
           && (sz_vol >= EF_CONF_GPT_MIN) )
      {
        /* Which partition type to create, MBR or GPT? */
        fsopt |= 0x80;    /* Partitioning is in GPT */
        b_vol = GPT_ALIGN / ss; sz_vol -= b_vol + GPT_ITEMS * EF_GPT_PTE_SIZE / ss + 1;  /* Estimated partition offset and size */
      }
      else
      {
        /* Partitioning is in MBR */
        if (sz_vol > N_SEC_TRACK)
        {
          b_vol = N_SEC_TRACK; sz_vol -= b_vol;  /* Estimated partition offset and size */
        }
      }
    }
  }
  if (sz_vol < 128) LEAVE_MKFS(EF_RET_MKFS_ABORTED);  /* Check if volume size is >=128s */

  /* Now start to create a FAT volume at b_vol and sz_vol */

  do {  /* Pre-determine the FAT type */
    if (    ( 0 != EF_CONF_LBA64 )
         && ( sz_vol >= 0x100000000 ) )
    {
      LEAVE_MKFS(EF_RET_MKFS_ABORTED);  /* Too large volume for FAT/FAT32 */
    }
    if (sz_au > 128)
    {
      sz_au = 128;  /* Invalid AU for FAT/FAT32? */
    }
    /* FAT32 possible? */
    if ( 0 != ( fsopt & FM_FAT32 ) )
    {
      /* no-FAT? */
      if ( 0 == ( fsopt & FM_FAT ) )
      {
        fsty = FS_FAT32; break;
      }
    }
    if (!(fsopt & FM_FAT))
      LEAVE_MKFS(EF_RET_INVALID_PARAMETER);  /* no-FAT? */
    fsty = FS_FAT16;
  } while (0);

  {  /* Create an FAT/FAT32 volume */
    do {
      pau = sz_au;
      /* Pre-determine number of clusters and FAT sub-type */
      if ( FS_FAT32 == fsty )
      {  /* FAT32 volume */
        if ( 0 == pau )
        {  /* AU auto-selection */
          n = (ef_u32_t)sz_vol / 0x20000;  /* Volume size in unit of 128KS */
          for ( i = 0, pau = 1; cst32[i] && cst32[i] <= n; i++, pau <<= 1 ) ;  /* Get from table */
        }
        n_clst = (ef_u32_t)sz_vol / pau;  /* Number of clusters */
        sz_fat = (n_clst * 4 + 8 + ss - 1) / ss;  /* FAT size [sector] */
        sz_rsv = 32;  /* Number of reserved sectors */
        sz_dir = 0;    /* No static directory */
        if (    ( n_clst <= EF_CLUTER_NB_MAX_FAT16 )
             || ( n_clst > EF_CLUTER_NB_MAX_FAT32 ) )
        {
          LEAVE_MKFS( EF_RET_MKFS_ABORTED );
        }
      }
      else
      {        /* FAT volume */
        /* au auto-selection */
        if ( 0 == pau )
        {
          n = (ef_u32_t)sz_vol / 0x1000;  /* Volume size in unit of 4KS */
          for ( i = 0, pau = 1; cst[i] && cst[i] <= n; i++, pau <<= 1 ) ;  /* Get from table */
        }
        n_clst = (ef_u32_t)sz_vol / pau;
        if ( n_clst > EF_CLUTER_NB_MAX_FAT12 )
        {
          n = n_clst * 2 + 4;    /* FAT size [byte] */
        }
        else
        {
          fsty = FS_FAT12;
          n = (n_clst * 3 + 1) / 2 + 3;  /* FAT size [byte] */
        }
        sz_fat = (n + ss - 1) / ss;    /* FAT size [sector] */
        sz_rsv = 1;            /* Number of reserved sectors */
        sz_dir = (ef_u32_t)u32RootDirNb * EF_DIR_ENTRY_SIZE / ss;  /* Root pu8Dir size [sector] */
      }
      b_fat = b_vol + sz_rsv;            /* FAT base */
      b_data = b_fat + sz_fat * u8FatsNb + sz_dir;  /* Data base */

      /* Align data area to erase block boundary (for flash memory media) */
      n = (ef_u32_t)(((b_data + sz_blk - 1) & ~(sz_blk - 1)) - b_data);  /* Sectors to next nearest from current data base */
      if ( FS_FAT32 == fsty )
      {    /* FAT32: Move FAT */
        sz_rsv += n; b_fat += n;
      }
      else
      {          /* FAT: Expand FAT */
        if ( 0 != ( n % u8FatsNb ) )
        {
          /* Adjust fractional error if needed */
          n--;
          sz_rsv++;
          b_fat++;
        }
        sz_fat += n / u8FatsNb;
      }

      /* Determine number of clusters and final check of validity of the FAT sub-type */
      if ( sz_vol < ( b_data + pau * 16 - b_vol ) )
      {
        LEAVE_MKFS(EF_RET_MKFS_ABORTED);  /* Too small volume? */
      }
      n_clst = ((ef_u32_t)sz_vol - sz_rsv - sz_fat * u8FatsNb - sz_dir) / pau;
      if ( FS_FAT32 == fsty )
      {
        if ( n_clst <= EF_CLUTER_NB_MAX_FAT16 )
        {
          /* Too few clusters for FAT32? */
          if ( ( sz_au == 0 ) && ( 0 != (sz_au = pau / 2) ) )
          {
            continue;  /* Adjust cluster size and retry */
          }
          LEAVE_MKFS(EF_RET_MKFS_ABORTED);
        }
      }
      if ( FS_FAT16 == fsty )
      {
        if ( n_clst > EF_CLUTER_NB_MAX_FAT16 )
        {  /* Too many clusters for FAT16 */
          if (sz_au == 0 && (pau * 2) <= 64)
          {
            sz_au = pau * 2;
            continue;    /* Adjust cluster size and retry */
          }
          if ( 0 != ( fsopt & FM_FAT32 ) )
          {
            fsty = FS_FAT32;
            continue;  /* Switch type to FAT32 and retry */
          }
          if (    ( 0 == sz_au )
               && ( (sz_au = pau * 2) <= 128 ) )
          {
            continue;  /* Adjust cluster size and retry */
          }
          LEAVE_MKFS(EF_RET_MKFS_ABORTED);
        }
        if  (n_clst <= EF_CLUTER_NB_MAX_FAT12)
        {  /* Too few clusters for FAT16 */
          if (    ( 0 == sz_au )
               && ( (sz_au = pau * 2) <= 128 ) )
          {
            continue;  /* Adjust cluster size and retry */
          }
          LEAVE_MKFS(EF_RET_MKFS_ABORTED);
        }
      }
      if (    ( FS_FAT12 == fsty )
           && ( n_clst > EF_CLUTER_NB_MAX_FAT12 ) )
      {
        LEAVE_MKFS(EF_RET_MKFS_ABORTED);  /* Too many clusters for FAT12 */
      }

      /* Ok, it is the valid cluster configuration */
      break;
    } while ( 1 );

    if ( 0 != EF_CONF_USE_TRIM )
    {
      /* Inform storage device that the volume area may be erased */
      lba[0] = b_vol;
      lba[1] = b_vol + sz_vol - 1;
       eEFPrvDriveIOCtrl( u8PhyDrvNb, CTRL_TRIM, lba );
    }
    /* Create FAT VBR */
    mem_set( buf, 0, ss );
    mem_cpy( buf + EF_BS_OFFSET_JMP_INST, "\xEB\xFE\x90" "MSDOS5.0", 11 );/* Boot jump code (x86), OEM name */
    vEFPortStoreu16( buf + EF_BS_BPB_FAT_OFFSET_SECTOR_SIZE, ss );        /* Sector size [byte] */
    buf[EF_BS_BPB_FAT_OFFSET_CLUSTER_SIZE] = (ef_u08_t)pau;        /* Cluster size [sector] */
    vEFPortStoreu16( buf + EF_BS_BPB_FAT_OFFSET_SECTORS_RESERVED_NB, (ef_u16_t)sz_rsv );  /* Size of reserved area */
    buf[EF_BS_BPB_FAT_OFFSET_FATS_NB] = (ef_u08_t)u8FatsNb;          /* Number of FATs */
    if ( FS_FAT32 == fsty )
    {
      vEFPortStoreu16( buf + EF_BS_BPB_FAT_OFFSET_ROOT_ENTRIES, (ef_u16_t) 0 );  /* Number of root directory entries */
    }
    else
    {
      vEFPortStoreu16( buf + EF_BS_BPB_FAT_OFFSET_ROOT_ENTRIES, (ef_u16_t) u32RootDirNb );  /* Number of root directory entries */
    }
    if ( 0x10000 > sz_vol )
    {
      vEFPortStoreu16( buf + EF_BS_BPB_FAT_OFFSET_SECTORS_COUNT, (ef_u16_t)sz_vol );  /* Volume size in 16-bit LBA */
    }
    else
    {
      vEFPortStoreu32( buf + EF_BS_BPB_FAT_OFFSET_SECTORS_LARGE_COUNT, (ef_u32_t)sz_vol );  /* Volume size in 32-bit LBA */
    }
    buf[EF_BS_BPB_FAT_OFFSET_MEDIA_DESCRIPTOR] = 0xF8;                          /* Media descriptor byte */
    vEFPortStoreu16( buf + EF_BS_BPB_FAT_OFFSET_TRACK_SIZE, 63 );             /* Number of sectors per track (for int13) */
    vEFPortStoreu16( buf + EF_BS_BPB_FAT_OFFSET_HEADS_NB, 255 );             /* Number of heads (for int13) */
    vEFPortStoreu32( buf + EF_BS_BPB_FAT_OFFSET_SECTORS_HIDDEN_NB, (ef_u32_t)b_vol );  /* Volume offset in the physical drive [sector] */
    if ( FS_FAT32 == fsty )
    {
      vEFPortStoreu32( buf + EF_BS_EBPB_FAT32_OFFSET_VOLUME_ID, EF_FATTIME_GET( ) ); /* VSN */
      vEFPortStoreu32( buf + EF_BS_EBPB_FAT32_OFFSET_FAT_SIZE, sz_fat );       /* FAT size [sector] */
      vEFPortStoreu32( buf + EF_BS_EBPB_FAT32_OFFSET_ROOT_DIRECTORY_NB, 2 );         /* Root directory cluster # (2) */
      vEFPortStoreu16( buf + EF_BS_EBPB_FAT32_OFFSET_FS_INFO_SECTOR, 1 );           /* Offset of FSINFO sector (VBR + 1) */
      vEFPortStoreu16( buf + EF_BS_EBPB_FAT32_OFFSET_BACKUPBOOT_SECTOR, 6 );        /* Offset of backup VBR (VBR + 6) */
      buf[EF_BS_EBPB_FAT32_OFFSET_DRIVE_NB] = 0x80;                    /* Drive number (for int13) */
      buf[EF_BS_EBPB_FAT32_OFFSET_SIGNATURE] = 0x29;                   /* Extended boot signature */
      mem_cpy( buf + EF_BS_EBPB_FAT32_OFFSET_VOLUME_LABEL, "NO NAME    " "FAT32   ", 19 );  /* Volume pxLabel, FAT signature */
    }
    else
    {
      vEFPortStoreu32( buf + EF_BS_EBPB_FAT16_OFFSET_VOLUME_ID, EF_FATTIME_GET( ) );  /* VSN */
      vEFPortStoreu16( buf + EF_BS_BPB_FAT_OFFSET_FAT16_SIZE, (ef_u16_t)sz_fat );  /* FAT size [sector] */
      buf[EF_BS_EBPB_FAT16_OFFSET_DRIVE_NB] = 0x80;            /* Drive number (for int13) */
      buf[EF_BS_EBPB_FAT16_OFFSET_SIGNATURE] = 0x29;            /* Extended boot signature */
      mem_cpy( buf + EF_BS_EBPB_FAT16_OFFSET_VOLUME_LABEL, "NO NAME    " "FAT     ", 19 );  /* Volume pxLabel, FAT signature */
    }
    vEFPortStoreu16( buf + EF_BS_OFFSET_SIGNATURE, 0xAA55 );          /* Signature (offset is fixed here regardless of sector size) */
    if ( EF_RET_OK !=  eEFPrvDriveWrite( u8PhyDrvNb, buf, b_vol, 1 ) )
    {
      LEAVE_MKFS(EF_RET_DISK_ERR);  /* Write it to the VBR sector */
    }

    /* Create FSINFO record if needed */
    if ( FS_FAT32 == fsty )
    {
       eEFPrvDriveWrite( u8PhyDrvNb, buf, b_vol + 6, 1 );    /* Write backup VBR (VBR + 6) */
      mem_set( buf, 0, ss );
      vEFPortStoreu32( buf + EF_BS_FAT32_FSI_OFFSET_SIGNATURE_LEAD, 0x41615252 );
      vEFPortStoreu32( buf + EF_BS_FAT32_FSI_OFFSET_SIGNATURE_NEXT, 0x61417272 );
      vEFPortStoreu32( buf + EF_BS_FAT32_FSI_OFFSET_FREE_CLUSTERS, n_clst - 1 );  /* Number of free clusters */
      vEFPortStoreu32( buf + EF_BS_FAT32_FSI_OFFSET_CLUSTER_LAST_ALLOC, 2 );      /* Last allocated cluster# */
      vEFPortStoreu16( buf + EF_BS_OFFSET_SIGNATURE, 0xAA55 );
       eEFPrvDriveWrite( u8PhyDrvNb, buf, b_vol + 7, 1 );    /* Write backup FSINFO (VBR + 7) */
       eEFPrvDriveWrite( u8PhyDrvNb, buf, b_vol + 1, 1 );    /* Write original FSINFO (VBR + 1) */
    }

    /* Initialize FAT area */
    mem_set( buf, 0, sz_buf * ss );
    xSector = b_fat;    /* FAT start sector */
    for (i = 0; i < u8FatsNb; i++)      /* Initialize FATs each */
    {
      if ( FS_FAT32 == fsty )
      {
        vEFPortStoreu32( buf + 0, 0xFFFFFFF8 );  /* FAT[0] */
        vEFPortStoreu32( buf + 4, 0xFFFFFFFF );  /* FAT[1] */
        vEFPortStoreu32( buf + 8, 0x0FFFFFFF );  /* FAT[2] (root directory) */
      }
      else
      {
        vEFPortStoreu32( buf + 0, (fsty == FS_FAT12) ? 0xFFFFF8 : 0xFFFFFFF8 );  /* FAT[0] and FAT[1] */
      }
      nsect = sz_fat;    /* Number of FAT sectors */
      do {  /* Fill FAT sectors */
        if ( nsect > sz_buf )
        {
          n = sz_buf;
        }
        else
        {
          n = nsect;
        }
        if ( EF_RET_OK !=  eEFPrvDriveWrite( u8PhyDrvNb, buf, xSector, (ef_u32_t)n ) )
        {
          LEAVE_MKFS( EF_RET_DISK_ERR );
        }
        mem_set( buf, 0, ss );  /* Rest of FAT all are cleared */
        xSector += n; nsect -= n;
      } while ( 0 != nsect );
    }

    /* Initialize root directory (fill with zero) */
    if ( FS_FAT32 == fsty )
    {
      nsect = pau;  /* Number of root directory sectors */
    }
    else
    {
      nsect = sz_dir;  /* Number of root directory sectors */
    }
    do {
      if ( nsect > sz_buf )
      {
        n = sz_buf;
      }
      else
      {
        n = nsect;
      }
      if ( EF_RET_OK !=  eEFPrvDriveWrite( u8PhyDrvNb, buf, xSector, (ef_u32_t) n ) )
      {
        LEAVE_MKFS(EF_RET_DISK_ERR);
      }
      xSector += n; nsect -= n;
    } while ( 0 != nsect );
  }

  /* A FAT volume has been created here */

  /* Determine system ID in the MBR partition table */
  if ( FS_FAT32 == fsty )
  {
    sys = 0x0C;    /* FAT32X */
  }
  else
  {
    if ( 0x10000 <= sz_vol )
    {
      sys = 0x06;  /* FAT12/16 (large) */
    }
    else
    {
      if ( FS_FAT16 == fsty )
      {
        sys = 0x04; /* FAT16 */
      }
      else
      {
        sys = 0x01; /* FAT12 */
      }
    }
  }

  /* Update partition information */
  if ( ( 0 != ipart ) )  /* Volume is in the existing partition */
  {
    if (    ( 0 == EF_CONF_LBA64 )
         || ( 0 == ( fsopt & 0x80 ) ) )
    {
      /* Update system ID in the partition table */
      if ( EF_RET_OK !=  eEFPrvDriveRead( u8PhyDrvNb, buf, 0, 1 ) )
      {
        LEAVE_MKFS(EF_RET_DISK_ERR);  /* Read the MBR */
      }
      buf[EF_MBR_OFFSET_PTE_ARRAY + (ipart - 1) * EF_MBR_PTE_SIZE + EF_MBR_PTE_OFFSET_PARTITION_TYPE] = sys;      /* Set system ID */
      if ( EF_RET_OK !=  eEFPrvDriveWrite( u8PhyDrvNb, buf, 0, 1 ) )
      {
        LEAVE_MKFS( EF_RET_DISK_ERR );  /* Write it back to the MBR */
      }
    }
  }
  else
  {
    /* Volume as a new single partition */
    if ( 0 == ( fsopt & FM_SFD ) )
    {
      /* Create partition table if not in SFD */
      lba[0] = sz_vol, lba[1] = 0;
      fr = eEFPrvPartitionCreate( u8PhyDrvNb, lba, sys, buf );
      if ( EF_RET_OK != fr )
      {
        LEAVE_MKFS( fr );
      }
    }
  }

  if ( EF_RET_OK !=  eEFPrvDriveIOCtrl( u8PhyDrvNb, CTRL_SYNC, 0 ) )
  {
    LEAVE_MKFS( EF_RET_DISK_ERR );
  }

  LEAVE_MKFS( EF_RET_OK );
}

#endif /* && EF_USE_MKFS */

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
