/**
 * ********************************************************************************************************************
 *  @file     ef_port_load_store.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Load/Store multi-byte word in a little endian memory byte array.
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
/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/*-----------------------------------------------------------------------*/
/* Load/Store multi-byte word in the FAT structure                       */
/*-----------------------------------------------------------------------*/

/* Load a 2-byte little-endian word */
ef_return_et eEFPortLoadu16 (
  const ef_u08_t * pu8src,
  ef_u16_t       * pu16Value
)
{
  EF_ASSERT_PRIVATE( 0 != pu8src );
  EF_ASSERT_PRIVATE( 0 != pu16Value );

  ef_u16_t     u16Value;

  u16Value = (ef_u16_t) pu8src[ 1 ];
  u16Value = u16Value << 8 | (ef_u16_t) pu8src[ 0 ];

  *pu16Value = u16Value;

  return EF_RET_OK;
}

/* Load a 4-byte little-endian word */
ef_return_et eEFPortLoadu32(
  const ef_u08_t * pu8src,
  ef_u32_t       * pu32Value
)
{
  EF_ASSERT_PRIVATE( 0 != pu8src );
  EF_ASSERT_PRIVATE( 0 != pu32Value );

  ef_u32_t     u32Value;

  u32Value = (ef_u32_t) pu8src[ 3 ];
  u32Value = u32Value << 8 | (ef_u32_t) pu8src[ 2 ];
  u32Value = u32Value << 8 | (ef_u32_t) pu8src[ 1 ];
  u32Value = u32Value << 8 | (ef_u32_t) pu8src[ 0 ];

  *pu32Value = u32Value;

  return EF_RET_OK;
}

/* Load an 8-byte little-endian word */
ef_return_et eEFPortLoadu64(
  const ef_u08_t * pu8src,
  ef_u64_t       * pu64Value
)
{
  EF_ASSERT_PRIVATE( 0 != pu8src );
  EF_ASSERT_PRIVATE( 0 != pu64Value );

  ef_u64_t     u64Value;

  u64Value = (ef_u64_t) pu8src[ 7 ];
  u64Value = u64Value << 8 | (ef_u64_t) pu8src[ 6 ];
  u64Value = u64Value << 8 | (ef_u64_t) pu8src[ 5 ];
  u64Value = u64Value << 8 | (ef_u64_t) pu8src[ 4 ];
  u64Value = u64Value << 8 | (ef_u64_t) pu8src[ 3 ];
  u64Value = u64Value << 8 | (ef_u64_t) pu8src[ 2 ];
  u64Value = u64Value << 8 | (ef_u64_t) pu8src[ 1 ];
  u64Value = u64Value << 8 | (ef_u64_t) pu8src[ 0 ];

  *pu64Value = u64Value;

  return EF_RET_OK;
}

ef_u16_t u16EFPortLoad( const ef_u08_t* pu8src )  /*   Load a 2-byte little-endian word */
{
  EF_ASSERT_PRIVATE( 0 != pu8src );

  ef_u16_t u16RetVal;

  u16RetVal = (ef_u16_t) pu8src[ 1 ];
  u16RetVal = ( u16RetVal << 8 ) | (ef_u16_t) pu8src[ 0 ];

  return u16RetVal;
}

ef_u32_t u32EFPortLoad( const ef_u08_t* pu8src )  /* Load a 4-byte little-endian word */
{
  EF_ASSERT_PRIVATE( 0 != pu8src );

  ef_u32_t u32RetVal;

  u32RetVal = (ef_u32_t) pu8src[ 3 ];
  u32RetVal = u32RetVal << 8 | (ef_u32_t) pu8src[ 2 ];
  u32RetVal = u32RetVal << 8 | (ef_u32_t) pu8src[ 1 ];
  u32RetVal = u32RetVal << 8 | (ef_u32_t) pu8src[ 0 ];

  return u32RetVal;
}

ef_u64_t u64EFPortLoad( const ef_u08_t* pu8src )  /* Load an 8-byte little-endian word */
{
  EF_ASSERT_PRIVATE( 0 != pu8src );

  ef_u64_t u64RetVal;

  u64RetVal = (ef_u64_t) pu8src[ 7 ];
  u64RetVal = u64RetVal << 8 | (ef_u64_t) pu8src[ 6 ];
  u64RetVal = u64RetVal << 8 | (ef_u64_t) pu8src[ 5 ];
  u64RetVal = u64RetVal << 8 | (ef_u64_t) pu8src[ 4 ];
  u64RetVal = u64RetVal << 8 | (ef_u64_t) pu8src[ 3 ];
  u64RetVal = u64RetVal << 8 | (ef_u64_t) pu8src[ 2 ];
  u64RetVal = u64RetVal << 8 | (ef_u64_t) pu8src[ 1 ];
  u64RetVal = u64RetVal << 8 | (ef_u64_t) pu8src[ 0 ];

  return u64RetVal;
}

/* Store a 2-byte word in little-endian */
ef_return_et eEFPortStoreu16( ef_u08_t* pu8dst, ef_u16_t u16value )
{
  EF_ASSERT_PRIVATE( 0 != pu8dst );

  *pu8dst++ = (ef_u08_t)u16value;
  u16value >>= 8;
  *pu8dst++ = (ef_u08_t)u16value;

  return EF_RET_OK;
}

/* Store a 4-byte word in little-endian */
ef_return_et eEFPortStoreu32( ef_u08_t* pu8dst, ef_u32_t u32value )
{
  EF_ASSERT_PRIVATE( 0 != pu8dst );

  *pu8dst++ = (ef_u08_t)u32value;
  u32value >>= 8;
  *pu8dst++ = (ef_u08_t)u32value;
  u32value >>= 8;
  *pu8dst++ = (ef_u08_t)u32value;
  u32value >>= 8;
  *pu8dst++ = (ef_u08_t)u32value;

  return EF_RET_OK;
}

/* Store an 8-byte word in little-endian */
ef_return_et eEFPortStoreu64( ef_u08_t* pu8dst, ef_u64_t u64value )
{
  EF_ASSERT_PRIVATE( 0 != pu8dst );

  *pu8dst++ = (ef_u08_t)u64value;
  u64value >>= 8;
  *pu8dst++ = (ef_u08_t)u64value;
  u64value >>= 8;
  *pu8dst++ = (ef_u08_t)u64value;
  u64value >>= 8;
  *pu8dst++ = (ef_u08_t)u64value;
  u64value >>= 8;
  *pu8dst++ = (ef_u08_t)u64value;
  u64value >>= 8;
  *pu8dst++ = (ef_u08_t)u64value;
  u64value >>= 8;
  *pu8dst++ = (ef_u08_t)u64value;
  u64value >>= 8;
  *pu8dst++ = (ef_u08_t)u64value;

  return EF_RET_OK;
}


/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
