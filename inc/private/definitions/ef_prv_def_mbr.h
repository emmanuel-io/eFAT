/**
 * ********************************************************************************************************************
 *  @file     ef_prv_def_mbr.h
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
#ifndef EFAT_PRIVATE_MBR_DEFINITIONS_H
#define EFAT_PRIVATE_MBR_DEFINITIONS_H
#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/**
 *  MBR: Offset for Bootstrap Code Area (446 bytes)
 */
#define EF_MBR_OFFSET_BOOTSTRAP  ( 0 )

/**
 *  MBR: Offset for Primary Partition Table (64 bytes)
 */
#define EF_MBR_OFFSET_PTE_ARRAY  ( 446 )

/**
 *  MBR: Primary Partition Entry SIZE
 */
#define EF_MBR_PTE_SIZE      ( 16)

/**
 *  MBR PTE: PTE offset for Disk Status / Boot Indicator (1 byte)
 */
#define EF_MBR_PTE_OFFSET_BOOT                ( 0 )

/**
 *  MBR PTE: PTE offset for CHS START Address Head part (1 byte)
 */
#define EF_MBR_PTE_OFFSET_CHS_START_HEAD      ( 1 )

/**
 *  MBR PTE: PTE offset for CHS START Address Sector/Cylinder part (1 byte)
 */
#define EF_MBR_PTE_OFFSET_CHS_START_SECTOR    ( 2 )

/**
 *  MBR PTE: PTE offset for CHS START Address Cylinder part (1 byte)
 */
#define EF_MBR_PTE_OFFSET_CHS_START_CYLINDER  ( 3 )

/**
 *  MBR PTE: PTE offset for Partition type (1 byte)
 */
#define EF_MBR_PTE_OFFSET_PARTITION_TYPE      ( 4 )

/**
 *  MBR PTE: PTE offset for CHS END Address Head part (1 byte)
 */
#define EF_MBR_PTE_OFFSET_CHS_END_HEAD        ( 5 )

/**
 *  MBR PTE: PTE offset for CHS END Address Sector/Cylinder part (1 byte)
 */
#define EF_MBR_PTE_OFFSET_CHS_END_SECTOR      ( 6 )

/**
 *  MBR PTE: PTE offset for CHS END Address Cylinder part (1 byte)
 */
#define EF_MBR_PTE_OFFSET_CHS_END_CYLINDER    ( 7 )

/**
 *  MBR PTE: PTE offset for LBA of first absolute sector in the partition (4 bytes)
 */
#define EF_MBR_PTE_OFFSET_LBA_START           ( 8 )

/**
 *  MBR PTE: PTE offset for Number of sectors (LBAs) in partition (4 bytes)
 */
#define EF_MBR_PTE_OFFSET_LBA_SIZE            ( 12 )

/* Public function macros -------------------------------------------------------------------------------------------------------------- */
/* Public typedefs, structures, unions and enums --------------------------------------------------------------------------------------- */
/* Public variables -------------------------------------------------------------------------------------------------------------------- */
/* Public function prototypes----------------------------------------------------------------------------------------------------------- */

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_MBR_DEFINITIONS_H */
/* END OF FILE ***************************************************************************************************** */

