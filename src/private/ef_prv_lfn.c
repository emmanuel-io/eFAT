/**
 * ********************************************************************************************************************
 *  @file     ef_prv_lfn.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Code file for functions.
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
#include "ef_prv_lfn.h"
#include "ef_prv_string.h"
#include "ef_prv_unicode.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* FAT: Offset of LFN characters in the directory entry */
static const ef_u08_t LfnOfs[ 13 ] = { 1,  3,  5,  7,  9,      /* The first 5 */
                                       14, 16, 18, 20, 22, 24, /* The next  6 */
                                       28, 30 };               /* The last  2 */

/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */

/**
 *  @brief  Buffer type structure (LFN_Buffer_st)
 */
typedef struct LFN_Buffer_struct {
  ucs2_t xBuffer[ EF_LFN_UNITS_MAX + 1 ]; /**< Buffer to hold LFN string */
} LFN_Buffer_st;

/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */

/**
 *  LFN working buffers 32-Byte aligned for cache maintenance
 */
static LFN_Buffer_st xLFNBuffers[ EF_CONF_VOLUMES_NB ] __attribute__ ((aligned (32)));

/**
 *  LFN working buffers pointers
 */
static LFN_Buffer_st * pxLFNBuffers[ EF_CONF_VOLUMES_NB ];

/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */
ef_return_et eEFPrvLFNBufferPtrGet (
  ef_fs_st      *   pxFS,
  const ucs2_t  **  ppxBuffer
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );
  EF_ASSERT_PRIVATE( 0 != ppxBuffer );

  ef_return_et  eRetVal = EF_RET_OK;

  if (    ( 0 <= pxFS->u8LogicNumber )
       && ( EF_CONF_VOLUMES_NB > pxFS->u8LogicNumber ) )
  {
    /* Volume number */
    *ppxBuffer = pxLFNBuffers[ pxFS->u8LogicNumber ];
  }
  else
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  return eRetVal;
}

ef_return_et eEFPrvLFNBufferPtrSet (
  ef_fs_st      * pxFS,
  const ucs2_t  * pxBuffer
)
{
  EF_ASSERT_PRIVATE( 0 != pxFS );
  EF_ASSERT_PRIVATE( 0 != pxBuffer );

  ef_return_et  eRetVal = EF_RET_OK;

  if (    ( 0 <= pxFS->u8LogicNumber )
       && ( EF_CONF_VOLUMES_NB > pxFS->u8LogicNumber ) )
  {
    /* Volume number */
    pxLFNBuffers[ pxFS->u8LogicNumber ] = pxBuffer;
  }
  else
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  return eRetVal;
}

/* VFAT-LFN: Compare a part of file name with an LFN entry */
ef_return_et eEFPrvLFNCompare (
  const ucs2_t  * pxLFNBuffer,
  ef_u08_t      * pu8Dir
)
{
  EF_ASSERT_PRIVATE( 0 != pxLFNBuffer );
  EF_ASSERT_PRIVATE( 0 != pu8Dir );

  ef_return_et  eRetVal = EF_RET_ERROR;

  /* Else, if Check LDIR_FstClusLO */
  if ( 0 != u16EFPortLoad( pu8Dir + EF_DIR_FIRST_CLUSTER_LOW ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    /* The part of LFN match by default */
    eRetVal = EF_RET_OK;

    ucs2_t u16Char = 1;

    /* Offset in the LFN buffer */
    int i = ( ( pu8Dir[ EF_DIR_LFN_ORDER ] & 0x3F ) - 1 ) * 13;

    /* Process all characters in the entry */
    for ( ef_u32_t s = 0 ; s < 13 ; s++ )
    {
      /* Pick an LFN character */
      ucs2_t uc = u16EFPortLoad( pu8Dir + LfnOfs[ s ] );
      if ( 0 != u16Char )
      {
        /* Compare it */
        if (    ( i >= ( EF_LFN_UNITS_MAX + 1 ) )
             || ( ef_wtoupper( uc ) != ef_wtoupper( pxLFNBuffer[ i++ ] ) ) )
        {
          /* Not matched */
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
          break;
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
        u16Char = uc;
      }
      else
      {
        /* Check filler */
        if ( 0xFFFF != uc )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
          break;
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
      }
    }

    /* If     it is the Last segment
     *    AND matched but different length */
    if (    ( 0 != ( pu8Dir[ EF_DIR_LFN_ORDER ] & EF_DIR_LFN_LAST ) )
         && ( 0 != u16Char )
         && ( 0 != pxLFNBuffer[ i ] ) )
    {
      eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
    }
  }

  return eRetVal;
}

/* VFAT-LFN: Pick a part of file name from an LFN entry */
ef_return_et eEFPrvLFNPick (
  ucs2_t    * pxLFNBuffer,
  ef_u08_t  * pu8Dir
)
{
  EF_ASSERT_PRIVATE( 0 != pxLFNBuffer );
  EF_ASSERT_PRIVATE( 0 != pu8Dir );

  ef_return_et  eRetVal = EF_RET_ERROR;

  /* Else, if Check LDIR_FstClusLO */
  if ( 0 != u16EFPortLoad( pu8Dir + EF_DIR_FIRST_CLUSTER_LOW ) )
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  else
  {
    /* The part of LFN is valid */
    eRetVal = EF_RET_OK;

    /* Offset in the LFN buffer */
    int i = ( ( pu8Dir[ EF_DIR_LFN_ORDER ] & ~EF_DIR_LFN_LAST ) - 1 ) * 13;

    ucs2_t u16Char = 1;

    /* Process all characters in the entry */
    for ( ef_u32_t s = 0 ; s < 13 ; s++ )
    {
      /* Pick an LFN character */
      ucs2_t uc = u16EFPortLoad( pu8Dir + LfnOfs[ s ] );
      if ( 0 != u16Char )
      {
        /* Buffer overflow? */
        if ( ( EF_LFN_UNITS_MAX + 1 ) <= i )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
          break;
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
        /* Store it */
        u16Char = uc;
        pxLFNBuffer[ i++ ] = uc;
      }
      else
      {
        /* Check filler */
        if ( 0xFFFF != uc )
        {
          eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
          break;
        }
        else
        {
          EF_CODE_COVERAGE( );
        }
      }
    } /* loop */
    /* If     everything went well
     *    AND it is the last LFN part
     *    AND it is not terminated */
    if (    ( EF_RET_OK == eRetVal )
         && ( 0 != ( pu8Dir[ EF_DIR_LFN_ORDER ] & EF_DIR_LFN_LAST ) )
         && ( 0 != u16Char ) )
    {
      /* Buffer overflow? */
      if ( ( EF_LFN_UNITS_MAX + 1 ) <= i )
      {
        eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
      pxLFNBuffer[ i ] = 0;
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
  }
    return eRetVal;
}

/* FAT-LFN: Create an entry of LFN entries */
ef_return_et eEFPrvLFNPut (
  const ucs2_t * pxLFNBuffer,
  ef_u08_t     * pu8Dir,
  ef_u08_t       u8Order,
  ef_u08_t       u8CheckSum
)
{
  EF_ASSERT_PRIVATE( 0 != pxLFNBuffer );
  EF_ASSERT_PRIVATE( 0 != pu8Dir );

  ef_return_et  eRetVal = EF_RET_ERROR;

  if (    ( 0 == u8Order )
       || ( 20 < u8Order ) )
  {
    /* Set checksum */
    pu8Dir[ EF_DIR_LFN_CHECKSUM ] = u8CheckSum;
    /* Set attribute. LFN entry */
    pu8Dir[ EF_DIR_ATTRIBUTES ]   = EF_DIR_ATTRIB_BITS_LFN;
    pu8Dir[ EF_DIR_LFN_TYPE ]   = 0;
    vEFPortStoreu16( pu8Dir + EF_DIR_FIRST_CLUSTER_LOW, 0 );

    /* Get offset in the LFN working buffer */
    ef_u32_t  i   = ( u8Order - 1 ) * 13;
    ef_u32_t  s   = 0;
    ucs2_t u16Char  = 0;
    do {
      if ( 0xFFFF != u16Char )
      {
        /* Get an eEFPrvective character */
        u16Char = pxLFNBuffer[ i++ ];
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
      /* Put it */
      vEFPortStoreu16( pu8Dir + LfnOfs[ s ], u16Char );
      if ( 0 == u16Char )
      {
        /* Padding characters for following items */
        u16Char = 0xFFFF;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
    } while ( 13 > ++s );
    if (    ( 0xFFFF == u16Char )
         || ( 0 == pxLFNBuffer[ i ] ) )
    {
      /* Last LFN part is the start of LFN sequence */
      u8Order |= EF_DIR_LFN_LAST;
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
    /* Set the LFN order */
    pu8Dir[ EF_DIR_LFN_ORDER ] = u8Order;

    eRetVal = EF_RET_OK;
  }
  return eRetVal;
}

/* FAT-LFN: Create a Numbered SFN */
ef_return_et eEFPrvLFNCreateSFN (
    ef_u08_t        * pu8SFNNumBuffer,
    const ef_u08_t  * pu8SFNBuffer,
    const ucs2_t    * pxLFNBuffer,
    ef_u32_t          uiSequenceNb
)
{
  EF_ASSERT_PRIVATE( 0 != pu8SFNNumBuffer );
  EF_ASSERT_PRIVATE( 0 != pu8SFNBuffer );
  EF_ASSERT_PRIVATE( 0 != pxLFNBuffer );

  ef_return_et  eRetVal = EF_RET_ERROR;

  (void) eEFPortMemCopy( pu8SFNBuffer, pu8SFNNumBuffer, 11 );

  /* In case of many collisions */
  if ( uiSequenceNb > 5 )
  {
    /* Generate a hash number instead of sequential number */
    ef_u32_t  sreg = uiSequenceNb;
    while ( 0 != *pxLFNBuffer )
    {  /* Create a CRC as hash value */
      ucs2_t u16Char = *pxLFNBuffer++;
      for ( ef_u32_t i = 0 ; i < 16 ; i++ )
      {
        sreg = ( sreg << 1 ) + ( u16Char & 1 );
        u16Char >>= 1;
        if ( 0 != ( sreg & 0x10000 ) )
        {
          sreg ^= 0x11021;
        }
      }
    }
    uiSequenceNb = (ef_u32_t)sreg;
  }
  else
  {
    EF_CODE_COVERAGE( );
  }

  ef_u08_t   ns[ 8 ];

  /* itoa (hexdecimal) */
  ef_u32_t i = 7;
  do
  {
    ef_u08_t c = (ef_u08_t)( ( uiSequenceNb % 16 ) + '0' );
    if ( c > '9' )
    {
      c += 7;
    }
    ns[ i-- ] = c;
    uiSequenceNb /= 16;
  } while ( 0 != uiSequenceNb );
  ns[ i ] = '~';

  ef_u32_t      j;
  /* Append the number to the SFN body */
  for ( j = 0 ; ( j < i ) && ( pu8SFNNumBuffer[ j ] != ' ' ) ; j++ )
  {
    if ( EF_RET_OK != eEFPrvByteInDBCRanges1( pu8SFNNumBuffer[ j ] ) )
    {
      if ( j == ( i - 1 ) )
      {
        break;
      }
      else
      {
        EF_CODE_COVERAGE( );
      }
      j++;
    }
    else
    {
      EF_CODE_COVERAGE( );
    }
  }
  do
  {
    if ( 8 > i )
    {
      pu8SFNNumBuffer[ j++ ] = ns[ i++ ];
    }
    else
    {
      pu8SFNNumBuffer[ j++ ] = ' ';
    }
  } while ( j < 8 );

  eRetVal = EF_RET_OK;

  return eRetVal;
}

/* FAT-LFN: Calculate checksum of an SFN entry */
ef_return_et eEFPrvSFNChecksumGet (
    const ef_u08_t * pu8Dir,
    ef_u08_t       * pu8CheckSum
)
{
  EF_ASSERT_PRIVATE( 0 != pu8Dir );
  EF_ASSERT_PRIVATE( 0 != pu8CheckSum );

  ef_return_et  eRetVal = EF_RET_ERROR;

  ef_u08_t u8CheckSum = 0;

  for ( int i = 11 ; 0 != i ; i-- )
  {
    u8CheckSum = ( u8CheckSum >> 1 ) + ( u8CheckSum << 7 ) + *pu8Dir++;
  }
  *pu8CheckSum = u8CheckSum;

  eRetVal = EF_RET_OK;

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
