/**
 * ********************************************************************************************************************
 *  @file     ef_port_types.h
 *  @ingroup  group_eFAT_Portable
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Header file for types definitions.
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

#ifndef EF_PORT_TYPES_DEFINED
#define EF_PORT_TYPES_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#include <ef_conf.h> /* eFAT configuration options */

/**
 *  Integer types used for eFAT API
 */

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || defined(__cplusplus)  /* C99 or later */
#include <stdint.h>
#include <stdbool.h>
  typedef uint8_t       ef_u08_t;   /**<  8-bit unsigned integer */
  typedef uint16_t      ef_u16_t;   /**< 16-bit unsigned integer */
  typedef uint32_t      ef_u32_t;   /**< 32-bit unsigned integer */
  typedef uint64_t      ef_u64_t;   /**< 64-bit unsigned integer */
  typedef bool          ef_bool_t;  /**< boolean type */
  #define EF_BOOL_TRUE  ( 1 )       /**< boolean TRUE value */
  #define EF_BOOL_FALSE ( 0 )       /**< boolean FALSE value */
#else    /* Earlier than C99 */
  typedef unsigned char   ef_u08_t;  /**<  8-bit unsigned integer */
  typedef unsigned short  ef_u16_t;  /**< 16-bit unsigned integer */
  typedef unsigned long   ef_u32_t;  /**< 32-bit unsigned integer */
#endif


#ifdef __cplusplus
}
#endif

#endif /* EF_PORT_TYPES_DEFINED */
