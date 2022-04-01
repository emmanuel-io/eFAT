/**
 * ********************************************************************************************************************
 *  @file     ef_prv_lfn.h
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Private header for Long File Name support.
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
#ifndef EFAT_PRIVATE_LFN_H
#define EFAT_PRIVATE_LFN_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */


/* Includes -------------------------------------------------------------------------------------------------------- */
#include <efat.h>
#include "ef_port_diskio.h"
#include "ef_prv_def.h"
/* Local constant macros ------------------------------------------------------------------------------------------- */

/**
 *  The EF_LFN_UNITS_MAX defines size of the working buffer in UTF-16 code unit and it can
 *  be in range of 12 to 255. It is recommended to be set it 255 to fully support LFN
 *  specification.
 */
#define EF_LFN_UNITS_MAX  ( 255 )

/* Local function macros ------------------------------------------------------------------------------------------- */

#if ( 0 == EF_CONF_VFAT )

  /* Non-LFN configuration */
  #define EF_LFN_BUFFER_DEFINE
  #define EF_LFN_BUFFER_SET( pxFS )             ( EF_RET_OK )
  #define EF_LFN_BUFFER_GET( pxFS, ppxBuffer )  ( EF_RET_OK )
  #define EF_LFN_BUFFER_FREE()
  #define LEAVE_MKFS(res)  return res

#else

  /* LFN configurations */
  #define MAXDIRB(nc)  ((nc + 44U) / 15 * EF_DIR_ENTRY_SIZE)
  /* LFN enabled with static working buffer */
  #if ( EF_DEF_VFAT_BUFFER_STATIC == EF_CONF_VFAT_BUFFER )

    /* LFN working buffer */
    static ucs2_t LfnBuf[ EF_LFN_UNITS_MAX + 1 ];
    #define EF_LFN_BUFFER_DEFINE
    #define EF_LFN_BUFFER_SET( pxFS )             eEFPrvLFNBufferPtrSet( pxFS, LfnBuf)
    #define EF_LFN_BUFFER_GET( pxFS, ppxBuffer )  eEFPrvLFNBufferPtrGet( pxFS, ppxBuffer)
    #define EF_LFN_BUFFER_FREE()

  /* LFN enabled with dynamic working buffer on the stack */
  #elif ( EF_DEF_VFAT_BUFFER_STACK == EF_CONF_VFAT_BUFFER )

    /* LFN working buffer */
    #define EF_LFN_BUFFER_DEFINE                  ucs2_t lbuf[ EF_LFN_UNITS_MAX + 1 ];
    #define EF_LFN_BUFFER_SET( pxFS )             eEFPrvLFNBufferPtrSet( pxFS, lbuf)
    #define EF_LFN_BUFFER_GET( pxFS, ppxBuffer )  eEFPrvLFNBufferPtrGet( pxFS, ppxBuffer)
    #define EF_LFN_BUFFER_FREE()
    #define LEAVE_MKFS(res)  return res

  /* LFN enabled with dynamic working buffer on the heap */
  #elif ( EF_DEF_VFAT_BUFFER_DYNAMIC == EF_CONF_VFAT_BUFFER )

    #define EF_LFN_BUFFER_DEFINE    ucs2_t *lfn;  /* Pointer to LFN working buffer */
    #define EF_LFN_BUFFER_SET( pxFS )   { \
                                    lfn = ef_memalloc( ( EF_LFN_UNITS_MAX + 1 ) * 2 ); \
                                    if ( 0 == lfn ) \
                                    { \
                                      (void) eEFPrvFSUnlock( pxFS, EF_RET_NOT_ENOUGH_CORE ); \
                                      return EF_RET_NOT_ENOUGH_CORE; \
                                    }; \
                                    ( pxFS )->pxLFNBuffer = lfn; \
                                  }
    #define EF_LFN_BUFFER_FREE()  ef_memfree(lfn)
    #define LEAVE_MKFS(res)  { if (!work) ef_memfree(buf); return res; }
    #define MAX_MALLOC  0x8000  /* Must be >=EF_CONF_SS_MAX */

  #else

    #error Wrong setting of EF_CONF_VFAT_BUFFER

  #endif  /* EF_CONF_VFAT == 1 */

#endif  /* EF_CONF_VFAT == 0 */


/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

ef_return_et eEFPrvLFNBufferPtrGet (
  ef_fs_st      *   pxFS,
  const ucs2_t  **  ppxBuffer
);

ef_return_et eEFPrvLFNBufferPtrSet (
  ef_fs_st      * pxFS,
  const ucs2_t  * pxBuffer
);

/**
 *  @brief  VFAT-LFN: Compare a part of file name with an LFN entry
 *
 *  @param  pxLFNBuffer Pointer to the LFN working buffer to be compared
 *  @param  pu8Dir      Pointer to the directory entry containing the part of LFN
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success (Match)
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvLFNCompare (
  const ucs2_t  * pxLFNBuffer,
  ef_u08_t     * pu8Dir
);

/**
 *  @brief  VFAT-LFN: Pick a part of file name from an LFN entry
 *
 *  @param  pxLFNBuffer Pointer to the LFN working buffer
 *  @param  pu8Dir      Pointer to the directory entry containing the part of LFN
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvLFNPick (
  ucs2_t   * pxLFNBuffer,
  ef_u08_t * pu8Dir
);


/**
 *  @brief  VFAT-LFN: Create an entry of LFN entries
 *
 *  @param  pxLFNBuffer Pointer to the LFN working buffer
 *  @param  pu8Dir      Pointer to the directory entry LFN part to create
 *  @param  u8Order     LFN order (1-20)
 *  @param  u8CheckSum  Checksum of the corresponding SFN
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvLFNPut (
  const ucs2_t  * pxLFNBuffer,
  ef_u08_t     * pu8Dir,
  ef_u08_t       u8Order,
  ef_u08_t       u8CheckSum
);

/**
 *  @brief  VFAT-LFN: Create a Numbered SFN
 *
 *  @param  pu8SFNNumBuffer Pointer to the buffer to store numbered SFN
 *  @param  pu8SFNBuffer    Pointer to SFN
 *  @param  pxLFNBuffer     Pointer to the LFN buffer
 *  @param  uiSequenceNb    Sequence number
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvLFNCreateSFN (
  ef_u08_t        * pu8SFNNumBuffer,
  const ef_u08_t  * pu8SFNBuffer,
  const ucs2_t    * pxLFNBuffer,
  ef_u32_t          uiSequenceNb
);

/**
 *  @brief  VFAT-LFN: Calculate checksum of an SFN entry
 *
 *  @param  pu8Dir      Pointer to the directory entry containing the SFN
 *  @param  pu8CheckSum Pointer to the checksum of the corresponding SFN
 *
 *  @return Operation result
 *  @retval EF_RET_OK  Success
 *  @retval EF_RET_ERROR    An error occurred
 *  @retval EF_RET_ASSERT   Assertion failed
 */
ef_return_et eEFPrvSFNChecksumGet (
  const ef_u08_t * pu8Dir,
  ef_u08_t       * pu8CheckSum
);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PRIVATE_LFN_H */
/* END OF FILE ***************************************************************************************************** */
