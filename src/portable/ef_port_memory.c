/**
 * ********************************************************************************************************************
 *  @file     ef_port_memory.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1

 *  @brief    Memory functions.
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
#include <ef_prv_def.h>
#include "ef_port_types.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/* Copy memory to memory */
ef_return_et eEFPortMemCopy (
  const void  * pvSrc,
  void        * pvDst,
  ef_u32_t      u32BytesNb
)
{
  EF_ASSERT_PRIVATE( 0 != pvSrc );
  EF_ASSERT_PRIVATE( 0 != pvDst );

  ef_u08_t       * pu8Dst = (ef_u08_t*) pvDst;
  const ef_u08_t * pu8Src = (const ef_u08_t*) pvSrc;

  for ( ef_u32_t i = u32BytesNb ; 0 != i ; i-- )
  {
    *pu8Dst++ = *pu8Src++;
  }

  return EF_RET_OK;
}

/* Set memory to zero */
ef_return_et eEFPortMemZero (
  void    * pvDst,
  ef_u32_t  u32BytesNb
)
{
  EF_ASSERT_PRIVATE( 0 != pvDst );

  ef_u08_t * pu8Dst = (ef_u08_t*) pvDst;

  for ( ef_u32_t i = u32BytesNb ; 0 != i ; i-- )
  {
    *pu8Dst++ = 0;
  }

  return EF_RET_OK;
}

/* Fill memory block */
ef_return_et eEFPortMemSet (
  void      * pvDst,
  ef_u08_t   u8Value,
  ef_u32_t   u32BytesNb
)
{
  EF_ASSERT_PRIVATE( 0 != pvDst );

  ef_u08_t * pu8Dst = (ef_u08_t*) pvDst;

  for ( ef_u32_t i = u32BytesNb ; 0 != i ; i-- )
  {
    *pu8Dst++ = u8Value;
  }

  return EF_RET_OK;
}

/* Compare memory block */
ef_return_et eEFPortMemCompare (
  const void  * pvBufferA,
  const void  * pvBufferB,
  ef_u32_t      u32Count
)
{
  EF_ASSERT_PRIVATE( 0 != pvBufferA );
  EF_ASSERT_PRIVATE( 0 != pvBufferB );

  ef_return_et  eRetVal = EF_RET_OK;
  const ef_u08_t *pu8BufferA = (const ef_u08_t *) pvBufferA;
  const ef_u08_t *pu8BufferB = (const ef_u08_t *) pvBufferB;

  do
  {
    if ( 0 != ( *pu8BufferA++ - *pu8BufferB++ ) )
    {
      eRetVal = EF_RET_ERROR;
      break;
    }
  } while ( 0 != (--u32Count) );

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
