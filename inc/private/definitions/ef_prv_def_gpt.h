/**
 * ********************************************************************************************************************
 *  @file     ef_prv_def_gpt.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private definitions for GUID Partition Table.
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
#ifndef EFAT_PRIVATE_GPT_DEFINITIONS_H
#define EFAT_PRIVATE_GPT_DEFINITIONS_H
#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/** @defgroup groupDEFINE_GPT  GUID Partion Table Support
 *  This is all what is needed to use GPT
 *  @{
 */

/**
 *  Header offset for signature (8 bytes)
 *  "EFI PART", (45h 46h 49h 20h 50h 41h 52h 54h)
 */
#define EF_GPT_HEADER_OFFSET_SIGNATURE      0

/**
 *  Header offset for Revision (4 bytes)
 *  For version 1.0, the value is  00h 00h 01h 00h
 */
#define EF_GPT_HEADER_OFFSET_REVISION       8

/**
 *  Header offset for Header size (4 bytes)
 */
#define EF_GPT_HEADER_OFFSET_HEADER_SIZE     12

/**
 *  Header offset for CRC32 of header
 *  (offset +0 up to header size) in little endian,
 *  with this field zeroed during calculation (4 bytes)
 */
#define EF_GPT_HEADER_OFFSET_HEADER_CRC32      16

/**
 *  Header offset for reserved zero area (4 bytes)
 */
#define EF_GPT_HEADER_OFFSET_HEADER_ZERO     20

/**
 *  Header offset for Current LBA (location of this header copy) (8 bytes)
 */
#define EF_GPT_HEADER_OFFSET_HEADER_LBA_CURRENT   24

/**
 *  Header offset for Backup LBA (location of the other header copy) (8 bytes)
 */
#define EF_GPT_HEADER_OFFSET_HEADER_LBA_BACKUP   32

/**
 *  Header offset for First usable LBA for partitions (primary partition table last LBA + 1)
 *  (8 bytes)
 */
#define EF_GPT_HEADER_OFFSET_PARTITIONS_LBA_FIRST   40

/**
 *  Header offset for Last usable LBA (secondary partition table first LBA − 1) (8 bytes)
 */
#define EF_GPT_HEADER_OFFSET_PARTITIONS_LBA_LAST   48

/**
 *  Header offset for Disk GUID in mixed endian (16 bytes)
 */
#define EF_GPT_HEADER_OFFSET_DISK_GUID  56

/**
 *  Header offset for Starting LBA of array of partition entries
 *       (always 2 in primary copy) (8 bytes)
 */
#define EF_GPT_HEADER_OFFSET_PTE_LBA_START    72

/**
 *  Header offset for Number of partition entries in array (4 bytes)
 */
#define EF_GPT_HEADER_OFFSET_PTE_NB    80

/**
 *  Header offset for Size of a single partition entry
 *       (usually 80h or 128) (4 bytes)
 */
#define EF_GPT_HEADER_OFFSET_PTE_SIZE  84

/**
 *  Header offset for CRC32 of partition entries array in little endian (4 bytes)
 */
#define EF_GPT_HEADER_OFFSET_PTE_ARRAY_CRC32    88

/**
 *  Header offset for Reserved padding; must be zeroes for the rest of the block
 *  (420 bytes for a sector size of 512 bytes; but can be more with
 *  larger sector sizes) (420+ bytes)
 */
#define EF_GPT_HEADER_OFFSET_PADDING    92

/**
 *  Size of partition table entry
 */
#define EF_GPT_PTE_SIZE      128

/**
 *  GPT PTE: PTE offset for Partition type GUID (mixed endian) (16 bytes)
 */
#define EF_GPT_PTE_OFFSET_TYPE_GUID    0

/**
 *  GPT PTE: PTE offset for Unique partition GUID (mixed endian) (16 bytes)
 */
#define EF_GPT_PTE_OFFSET_UNIQUE_GUID   16

/**
 *  GPT PTE: PTE offset for First LBA (little endian) (8 bytes)
 */
#define EF_GPT_PTE_OFFSET_LBA_FIRST   32

/**
 *  GPT PTE: PTE offset for Last LBA (inclusive, usually odd) (8 bytes)
 */
#define EF_GPT_PTE_OFFSET_LBA_LAST   40

/**
 *  GPT PTE: PTE offset for Attribute flags
 *  (e.g. bit 60 denotes read-only) (8 bytes)
 */
#define EF_GPT_PTE_OFFSET_ATTRIBUTE_FLAGS    48

/**
 *  GPT PTE: PTE offset for Partition name (36 UTF-16LE code units) (72 bytes)
 */
#define EF_GPT_PTE_OFFSET_PARTITION_NAME     56

/** @} */ /* end of groupDEFINE_GPT */

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_GPT_DEFINITIONS_H */
/* END OF FILE ***************************************************************************************************** */

