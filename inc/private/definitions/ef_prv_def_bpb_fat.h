/**
 * ********************************************************************************************************************
 *  @file     ef_prv_def_bpb_fat.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private definitions for Master Boot Record.
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
#ifndef EFAT_PRIVATE_BPB_FAT_DEFINITIONS_H
#define EFAT_PRIVATE_BPB_FAT_DEFINITIONS_H
#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/** @defgroup groupDEFINE_BS  Boot sector
 *  This is all what is needed to use Boot sectors
 *  @{
 */

/**
 *  Offset for Boot Jump instruction (3 bytes)
 */
#define EF_BS_OFFSET_JMP_INST   ( 0 )

/**
 *  Offset for OEM NAME (8 bytes)
 */
#define EF_BS_OFFSET_OEM_NAME   ( 3 )

/**
 *  Offset for Signature word (2 bytes)
 */
#define EF_BS_OFFSET_SIGNATURE  ( 510 )

/** @} */ /* end of groupDEFINE_BS */

/** @defgroup groupDEFINE_BS_BPB_FAT  Bios Parameter Block for FAT12/16/32
 *  This is all what is needed to use Bios Parameter Block for FAT12/16/32
 *  @ingroup groupDEFINE_BS
 *  @{
 */

/**
 *  Offset for Sector Size in Bytes (2 bytes)
 */
#define EF_BS_BPB_FAT_OFFSET_SECTOR_SIZE          ( 11 )

/**
 *  Offset for Cluster Size in Sectors (1 byte)
 */
#define EF_BS_BPB_FAT_OFFSET_CLUSTER_SIZE         ( 13 )

/**
 *  Offset for reserved area in Sectors (2 bytes)
 *  The boot record sectors are included in this value.
 */
#define EF_BS_BPB_FAT_OFFSET_SECTORS_RESERVED_NB  ( 14 )

/**
 *  Offset for Number of FATs on the storage media. (1 byte)
 *  Often this value is 2.
 */
#define EF_BS_BPB_FAT_OFFSET_FATS_NB              ( 16 )

/**
 *  Offset for Number of directory entries (2 bytes)
 *  (must be set so that the root directory occupies entire sectors).
 */
#define EF_BS_BPB_FAT_OFFSET_ROOT_ENTRIES         ( 17 )

/**
 *  Offset for The total sectors in the logical volume. (2 bytes)
 *  If this value is 0, it means there are more than 65535 sectors in the volume,
 *  and the actual count is stored in the Large Sector Count entry at 0x20.
 */
#define EF_BS_BPB_FAT_OFFSET_SECTORS_COUNT        ( 19 )

/**
 *  Offset for Media descriptor type (1 byte)
 */
#define EF_BS_BPB_FAT_OFFSET_MEDIA_DESCRIPTOR     ( 21 )

/**
 *  Offset for FAT Size in Sectors (2 bytes)
 */
#define EF_BS_BPB_FAT_OFFSET_FAT16_SIZE           ( 22 )

/**
 *  Offset for Track Size in Sectors (2 bytes)
 */
#define EF_BS_BPB_FAT_OFFSET_TRACK_SIZE           ( 24 )

/**
 *  Offset for Number of heads or sides (2 bytes)
 *  on the storage media for int13h.
 */
#define EF_BS_BPB_FAT_OFFSET_HEADS_NB             ( 26 )

/**
 *  Offset for Number of hidden sectors (4 bytes)
 *  (i.e. the LBA of the beginning of the partition.)
 */
#define EF_BS_BPB_FAT_OFFSET_SECTORS_HIDDEN_NB    ( 28 )

/**
 *  Offset for The total sectors in the logical volume (4 bytes)
 */
#define EF_BS_BPB_FAT_OFFSET_SECTORS_LARGE_COUNT  ( 32 )

/** @} */ /* end of groupDEFINE_BS */

/** @defgroup groupDEFINE_BS_EBPB_FAT16  Extended Bios Parameter Block for FAT12/16
 *  This is all what is needed to use Extended Bios Parameter Block for FAT12/16
 *  @ingroup groupDEFINE_BS_BPB_FAT
 *  @{
 */

/**
 *  Offset for Physical drive number for int 13h (1 byte)
 */
#define EF_BS_EBPB_FAT16_OFFSET_DRIVE_NB          ( 36 )

/**
 *  Offset for WindowsNT error statusFlags (1 byte)
 *  Flags in Windows NT. Reserved otherwise.
 */
#define EF_BS_EBPB_FAT16_OFFSET_NT_FLAGS_RESERVED ( 37 )

/**
 *  Offset for Extended boot signature (1 byte)
 *  (must be 0x28 or 0x29)
 */
#define EF_BS_EBPB_FAT16_OFFSET_SIGNATURE         ( 38 )

/**
 *  Offset for VolumeID 'Serial' number (4 bytes)
 *  Used for tracking volumes between computers, not used.
 */
#define EF_BS_EBPB_FAT16_OFFSET_VOLUME_ID         ( 39 )

/**
 *  Offset for Volume label string (11 bytes)
 *  This field is padded with spaces.
 */
#define EF_BS_EBPB_FAT16_OFFSET_VOLUME_LABEL      ( 43 )

/**
 *  Offset for System identifier string (8 bytes)
 *  This field is a string representation of the FAT file system type,
 *  It is padded with spaces.
 *  note: The spec says never to trust the contents of this string for any use.
 */
#define EF_BS_EBPB_FAT16_OFFSET_FS_TYPE           ( 54 )

/**
 *  Offset for Boot code (448 bytes)
 */
#define EF_BS_EBPB_FAT16_OFFSET_BOOT_CODE         ( 62 )

/** @} */ /* end of groupDEFINE_BS_EBPB_FAT16 */

/** @defgroup groupDEFINE_BS_EBPB_FAT32  Extended Bios Parameter Block for FAT32
 *  This is all what is needed to use Extended Bios Parameter Block for FAT32
 *  @ingroup groupDEFINE_BS_BPB_FAT
 *  @{
 */

/**
 *  Offset for FAT Size in Sectors (4 bytes)
 */
#define EF_BS_EBPB_FAT32_OFFSET_FAT_SIZE          ( 36 )

/**
 *  Offset for Extended flags (2 bytes)
 */
#define EF_BS_EBPB_FAT32_OFFSET_EXTENDED_FLAGS    ( 40 )

/**
 *  Offset for Filesystem Version (2 bytes)
 *  The high byte is the major version and the low byte is the minor version.
 */
#define EF_BS_EBPB_FAT32_OFFSET_FAT_VERSION       ( 42 )

/**
 *  Offset for Root directory cluster number (4 bytes)
 *  Often this field is set to 2.
 */
#define EF_BS_EBPB_FAT32_OFFSET_ROOT_DIRECTORY_NB ( 44 )

/**
 *  Offset for the FSInfo structure sector number (2 bytes)
 */
#define EF_BS_EBPB_FAT32_OFFSET_FS_INFO_SECTOR    ( 48 )

/**
 *  Offset for the backup boot sector number (2 bytes)
 */
#define EF_BS_EBPB_FAT32_OFFSET_BACKUPBOOT_SECTOR ( 50 )

  /**
   *  BS EPBP FAT3: Offset for Physical drive number for int 13h (1 byte)
   */
#define EF_BS_EBPB_FAT32_OFFSET_DRIVE_NB          ( 64 )

/**
 *  Offset for WindowsNT error statusFlags (1 byte)
 *  Flags in Windows NT. Reserved otherwise.
 */
#define EF_BS_EBPB_FAT32_OFFSET_NT_FLAGS_RESERVED ( 65 )

/**
 *  Offset for Extended boot signature (1 byte)
 *  (must be 0x28 or 0x29)
 */
#define EF_BS_EBPB_FAT32_OFFSET_SIGNATURE         ( 66 )
/**
 *  Offset for VolumeID 'Serial' number (4 bytes)
 *  Used for tracking volumes between computers, not used.
 */
#define EF_BS_EBPB_FAT32_OFFSET_VOLUME_ID         ( 67 )

/**
 *  Offset for Volume label string (11 bytes)
 *  This field is padded with spaces.
 */
#define EF_BS_EBPB_FAT32_OFFSET_VOLUME_LABEL      ( 71 )

/**
 *  ystem identifier string (8 bytes)
 *  System identifier string. Always "FAT32   ".
 *  note: The spec says never to trust the contents of this string for any use.
 */
#define EF_BS_EBPB_FAT32_OFFSET_FS_TYPE           ( 82 )

/**
 *  Offset for Boot code (420 bytes)
 */
#define EF_BS_EBPB_FAT32_OFFSET_BOOT_CODE         ( 90 )

/** @} */ /* end of groupDEFINE_BS_EBPB_FAT32 */

/** @defgroup groupDEFINE_BS_FAT32_FSI  FileSystem Info Structure for FAT32
 *  This is all what is needed to use FileSystem Info Structure for FAT32
 *  @ingroup groupDEFINE_BS_BPB_FAT
 *  @{
 */

/**
 *  Offset for Leading signature (4 bytes)
 *  (must be 0x41615252 to indicate a valid FSInfo structure)
 */
#define EF_BS_FAT32_FSI_OFFSET_SIGNATURE_LEAD     ( 0 )

/**
 *  Offset for Main reserved bytes (420 bytes)
 *  these bytes should never be used
 */
#define EF_BS_FAT32_FSI_OFFSET_RESERVED_MAIN      ( 4 )

/**
 *  Offset for Next signature (4 bytes)
 *  (must be 0x61417272 to indicate a valid FSInfo structure)
 */
#define EF_BS_FAT32_FSI_OFFSET_SIGNATURE_NEXT     ( 484 )

/**
 *  Offset for known free cluster (4 bytes)
 *  Contains the last known free cluster count on the volume.
 *  If the value is 0xFFFFFFFF, then the free count is unknown
 *  and must be computed. However, this value might be incorrect
 *  and should at least be range checked (<= volume cluster count)
 */
#define EF_BS_FAT32_FSI_OFFSET_FREE_CLUSTERS      ( 488 )

/**
 *  Offset for Last allocated cluster (4 bytes)
 *  Indicates the cluster number at which the filesystem driver should
 *  start looking for available clusters. If the value is 0xFFFFFFFF,
 *  then there is no hint and the driver should start searching at 2.
 *  Typically this value is set to the last allocated cluster number.
 *  As the previous field, this value should be range checked.
 */
#define EF_BS_FAT32_FSI_OFFSET_CLUSTER_LAST_ALLOC ( 492 )

/**
 *  Offset for Last reserved bytes (12 bytes)
 */
#define EF_BS_FAT32_FSI_OFFSET_RESERVED_LAST       ( 496 )

/**
 *  Offset for Trailing signature (4 bytes)
 *  (must be 0xAA550000 to indicate a valid FSInfo structure)
 */
#define EF_BS_FAT32_FSI_OFFSET_SIGNATURE_TRAIL     ( 508 )

/** @} */ /* end of groupDEFINE_BS_FAT32_FSI */

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_BPB_FAT_DEFINITIONS_H */
/* END OF FILE ***************************************************************************************************** */

