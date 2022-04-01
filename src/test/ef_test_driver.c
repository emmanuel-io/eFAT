/**
 * ********************************************************************************************************************************************************************************
 * @file    ef_test_driver.c
 * @ingroup GroupeFATTest
 * @author  SAG Application Team
 * @version V1.0.0
 * @date    04-04-2019
 * @brief   Test functions for testing sdcard functionalities
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
#include <ef_port_memory.h>
#include <stdio.h>
#include <string.h>

#include "efat.h"

#include "ef_prv_drive.h"
#include "ef_test_driver.h"
/* Local constant macros ------------------------------------------------------------------------------------------- */
/**
 *  Size of message handled
*/
#define DEBUG_STRING_SIZE ( 100 )

/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */

/**
 *  @brief Pseudo random number generator
 *
 *  @param  u32pns   0:Initialize, !0:Read
 *
 *  @return The pseudo random number generated
 */
static ef_u32_t u32PseudoRandomGenerator (
  ef_u32_t u32pns
);

/* Local functions ------------------------------------------------------------------------------------------------- */
static ef_u32_t u32PseudoRandomGenerator (
  ef_u32_t pns
)
{
  static ef_u32_t u32lfsr;

  if ( 0 != pns )
  {
    u32lfsr = pns;
    for (ef_u32_t n = 0 ; n < 32 ; n++ )
    {
      u32PseudoRandomGenerator( 0 );
    }
  }
  if ( 0 != ( u32lfsr & 0x00000001 ) )
  {
    u32lfsr >>= 1;
    u32lfsr ^= 0x80200003;
  }
  else
  {
    u32lfsr >>= 1;
  }
  return u32lfsr;
}

/* Public functions ------------------------------------------------------------------------------------------------ */

int32_t s32TestPrvDrive (
  ef_u08_t    u8PhyDrvNb,	  /* Physical drive number to be checked (all data on the drive will be lost) */
  ef_u32_t    u32Cycles,		  /* Number of test cycles */
  ef_u08_t  * pu8Buffer,	  /* Pointer to the working buffer */
  ef_u32_t    u32BufferSize	/* Size of the working buffer in unit of byte */
)
{
  int32_t   s32RetVal = 0;

  /* Test Insufficient work area to run the program */
  if ( u32BufferSize >= EF_CONF_SECTOR_SIZE + 8 )
  {
    /* Loop for as many cycles as needed */
    for ( ef_u32_t u32CyclesCount = 1 ; u32CyclesCount <= u32Cycles ; u32CyclesCount++ )
    {
      ef_u32_t      n;
      ef_lba_t      lba;
      ef_lba_t      lba2;
      ef_u32_t      pns = 1;
      ef_u08_t    * pbuff = (ef_u08_t*) pu8Buffer;
      ef_return_et  eRetVal;
      /* Test cycle START */

      /* Test disk_initalize */
      if ( EF_RET_DISK_NOINIT != eEFPrvDriveInitialize( u8PhyDrvNb ) )
      {
        s32RetVal = 2;
        break;
      }
      /* Test get drive size */
      ef_u32_t  u32DriveSize = 0;
      if ( EF_RET_OK != eEFPrvDriveIOCtrl(  u8PhyDrvNb,
                                            GET_SECTOR_COUNT,
                                            &u32DriveSize ) )
      {
        s32RetVal = 3;
        break;
      }
      /* Test insufficient drive size to test */
      if ( 128 > u32DriveSize )
      {
        s32RetVal = 4;
        break;
      }

      ef_u16_t  u16SectorSize = EF_CONF_SECTOR_SIZE;
      if ( 0 == EF_CONF_SECTOR_SIZE_FIXED )
      {
        /* Test Get sector size */
        u16SectorSize = 0;
        eRetVal =  eEFPrvDriveIOCtrl( u8PhyDrvNb,
                                        GET_SECTOR_SIZE,
                                        &u16SectorSize );
        if ( EF_RET_OK != eRetVal )
        {
          s32RetVal = 5;
          break;
        }
      }

      /* Test Get block size */
      ef_u32_t  u32EraseBlockSize = 0;
      eRetVal =  eEFPrvDriveIOCtrl( u8PhyDrvNb, GET_BLOCK_SIZE, &u32EraseBlockSize );
      if ( EF_RET_OK != eRetVal )
      {
        s32RetVal = 101;
        break;
      }
      if (    ( EF_RET_OK == eRetVal )
           || ( 2 <= u32EraseBlockSize ) )
      {
      }
      else
      {
        s32RetVal = 101;
        //break;
        printf( " Size of the erase block is unknown.\r\n" );
      }

      /* Single sector write test */
      lba = 0;
      for ( n = 0 , u32PseudoRandomGenerator(pns) ; n < u16SectorSize ; n++ )
      {
        pbuff[ n ] = (ef_u08_t)u32PseudoRandomGenerator(0);
      }
      /* Test Single sector write test */
      if ( EF_RET_OK != eEFPrvDriveWrite( u8PhyDrvNb, pbuff, lba, 1 ) )
      {
        s32RetVal = 6;
        break;
      }
      /* Test IOCtrl Sync */
      if ( EF_RET_OK != eEFPrvDriveIOCtrl( u8PhyDrvNb, CTRL_SYNC, 0 ) )
      {
        s32RetVal = 7;
        break;
      }
      eEFPortMemZero( pbuff, u16SectorSize );
      /* Single sector read test */
      if ( EF_RET_OK != eEFPrvDriveRead( u8PhyDrvNb, pbuff, lba, 1 ) )
      {
        s32RetVal = 8;
        break;
      }
      for ( n = 0, u32PseudoRandomGenerator(pns) ; n < u16SectorSize && pbuff[n] == (ef_u08_t)u32PseudoRandomGenerator(0) ; n++ )
      {
        ;
      }
      if ( EF_RET_OK != eRetVal )
      {
        s32RetVal = 10;
        break;
      }
      pns++;

      /* Multiple sector write test */
      lba = 5;
      ef_u32_t  u32SectorsNb = u32BufferSize / u16SectorSize;
      if ( 4 < u32SectorsNb )
      {
        u32SectorsNb = 4;
      }
      if ( 1 < u32SectorsNb )
      {
        for ( n = 0, u32PseudoRandomGenerator( pns ); n < (ef_u32_t)(u16SectorSize * u32SectorsNb); n++)
        {
          pbuff[ n ] = (ef_u08_t)u32PseudoRandomGenerator( 0 );
        }
        /* Write multiple sectors */
        eRetVal =  eEFPrvDriveWrite( u8PhyDrvNb, pbuff, lba, u32SectorsNb );
        if ( EF_RET_OK != eRetVal )
        {
          s32RetVal = 11;
          break;
        }
        /* Sync Control */
         eRetVal =  eEFPrvDriveIOCtrl( u8PhyDrvNb, CTRL_SYNC, 0 );
         if ( EF_RET_OK != eRetVal )
         {
           s32RetVal = 12;
           break;
         }
         eEFPortMemZero( pbuff, u16SectorSize * u32SectorsNb );
        /* Read multiple sectors */
        eRetVal =  eEFPrvDriveRead( u8PhyDrvNb, pbuff, lba, u32SectorsNb );
        if ( EF_RET_OK != eRetVal )
        {
          s32RetVal = 13;
          break;
        }
        for ( n = 0, u32PseudoRandomGenerator( pns ); n < (ef_u32_t)(u16SectorSize * u32SectorsNb) && pbuff[ n ] == (ef_u08_t)u32PseudoRandomGenerator(0); n++) ;
        if ( EF_RET_OK != eRetVal )
        {
          s32RetVal = 14;
          break;
        }
      }
      else
      {
        printf( " Test skipped.\r\n" );
      }
      pns++;

      printf( "**** Single sector write test (unaligned buffer address) ****\r\n" );
      lba = 5;
      for ( n = 0, u32PseudoRandomGenerator( pns ) ; n < u16SectorSize ; n++ )
      {
        pbuff[ n+3 ] = (ef_u08_t)u32PseudoRandomGenerator( 0 );
      }
      /* Write single sector (unaligned buffer address)  */
      eRetVal =  eEFPrvDriveWrite( u8PhyDrvNb, pbuff+3, lba, 1 );
      if ( EF_RET_OK != eRetVal )
      {
        s32RetVal = 15;
        break;
      }
      /* Sync Control */
      eRetVal =  eEFPrvDriveIOCtrl( u8PhyDrvNb, CTRL_SYNC, 0 );
      if ( EF_RET_OK != eRetVal )
      {
        s32RetVal = 16;
        break;
      }
      eEFPortMemZero( pbuff + 5, u16SectorSize );
      eRetVal =  eEFPrvDriveRead( u8PhyDrvNb, pbuff+5, lba, 1 );
      if ( EF_RET_OK != eRetVal )
      {
        s32RetVal = 17;
        break;
      }
      for ( n = 0, u32PseudoRandomGenerator( pns ) ; n < u16SectorSize && pbuff[ n+5 ] == (ef_u08_t)u32PseudoRandomGenerator(0) ; n++ ) ;
      if ( n != u16SectorSize )
      {
        s32RetVal = 18;
        break;
      }
      pns++;

      printf( "**** 4GB barrier test ****\r\n" );
      if ( u32DriveSize >= ( 128 + ( 0x80000000 / ( u16SectorSize / 2 ) ) ) )
      {
        lba = 6;
        lba2 = lba + ( 0x80000000 / ( u16SectorSize / 2 ) );
        for ( n = 0, u32PseudoRandomGenerator(pns) ; n < (ef_u32_t)(u16SectorSize * 2) ; n++ )
        {
          pbuff[n] = (ef_u08_t)u32PseudoRandomGenerator(0);
        }
        /* Write single sector (below 4GB barrier)  */
        eRetVal =  eEFPrvDriveWrite( u8PhyDrvNb, pbuff, lba, 1 );
        if ( EF_RET_OK != eRetVal )
        {
          s32RetVal = 19;
          break;
        }
        /* Write single sector (above 4GB barrier)  */
        eRetVal =  eEFPrvDriveWrite( u8PhyDrvNb, pbuff+u16SectorSize, lba2, 1 );
        if ( EF_RET_OK != eRetVal )
        {
          s32RetVal = 20;
          break;
        }
        /* Sync Control */
        eRetVal =  eEFPrvDriveIOCtrl( u8PhyDrvNb, CTRL_SYNC, 0 );
        if ( EF_RET_OK != eRetVal )
        {
          s32RetVal = 21;
          break;
        }
        eEFPortMemZero( pbuff, u16SectorSize * 2 );
        /* Read single sector (below 4GB barrier)  */
        eRetVal =  eEFPrvDriveRead( u8PhyDrvNb, pbuff, lba, 1 );
        if ( EF_RET_OK != eRetVal )
        {
          s32RetVal = 22;
          break;
        }
        /* Read single sector (above 4GB barrier)  */
        if ( EF_RET_OK != eEFPrvDriveRead( u8PhyDrvNb, pbuff+u16SectorSize, lba2, 1 ) )
        {
          s32RetVal = 23;
          break;
        }
        for (n = 0, u32PseudoRandomGenerator(pns) ; pbuff[n] == (ef_u08_t)u32PseudoRandomGenerator(0) && n < (ef_u32_t)(u16SectorSize * 2) ; n++ )
        {
          ;
        }
        if ( n != (ef_u32_t)( u16SectorSize * 2 ) )
        {
          s32RetVal = 24;
          break;
        }
      }
      else
      {
        printf( " Test skipped.\r\n" );
      }
      pns++;

    } /* Loop for as many cycles as needed */
  }
  else
  {
    s32RetVal = 1;
  }

  return s32RetVal;
} /* s32TestSdCardDiskIO */

#if 0
ErrorStatus eTestSDCardRawSpeed (
	ef_u08_t		u8PhyDrvNb,				/* Physical drive number */
	ef_u32_t	u32LBAStart,			/* Start LBA for read/write test */
	ef_u32_t	u32BytesNb,				/* Number of bytes to read/write (must be multiple of u32BufferSize) */
	void*		pvBuffer,				/* Read/write buffer */
	ef_u32_t	u32BufferSize,			/* Size of read/write buffer (must be multiple of EF_CONF_SS_MAX) */
	test_sdcard_raw_times_st * pxTimes	/* Pointer to transaction time in ticks, first write, then read */
)
{
	ErrorStatus eResult = ERROR;
	ef_u32_t	u32SectorSize	= EF_CONF_SS_MAX;

    pxTimes->u32Write = 0;
    pxTimes->u32Read = 0;

    if (		( EF_MIN_SS != EF_CONF_SS_MAX )
    		&&	( EF_RET_OK !=  eEFPrvDriveIOCtrl( u8PhyDrvNb, GET_SECTOR_SIZE, &u32SectorSize ) ) )
	{
		if ( 0 != DEBUG_ENABLE )
		{
			printf( "\ndisk_ioctl() failed.\r\n" );
		}
	}
    else
    {
        ef_u32_t	u32SectorCount	= u32BytesNb / u32SectorSize;
        ef_u32_t	u32Offset		= 0;
        ef_u32_t	u32Timer		= 0;
        char		cBuffer[ DEBUG_STRING_SIZE ];

        eResult = ERROR;
		snprintf(	cBuffer,
					DEBUG_STRING_SIZE,
					"Starting raw write test at sector %lu in %lu bytes of data chunks...",
					(ef_u32_t) u32LBAStart,
					(ef_u32_t) u32BufferSize );
		printf( cBuffer );

		ef_u08_t		u8SectorCount	= 128;

		u32Timer = HAL_GetTick( );
		for ( u32Offset = 0 ; u32Offset < u32SectorCount ; u32Offset += ( (ef_u32_t) u8SectorCount ) )
		{
			eResult = ERROR;
			if ( ( (ef_u32_t) 128 ) > ( u32SectorCount - u32Offset ) )
			{
				u8SectorCount 	= ( u32SectorCount - u32Offset );
			}
			if ( EF_RET_OK !=  eEFPrvDriveWrite( u8PhyDrvNb, pvBuffer, u32LBAStart + u32Offset, u8SectorCount ) )
			{
				if ( 0 != DEBUG_ENABLE )
				{
					printf( "\ndisk_write() failed.\r\n" );
				}
				break;
			}
			else
			{
				eResult = SUCCESS;
			}
		}
		if ( SUCCESS == eResult )
		{
			if ( EF_RET_OK !=  eEFPrvDriveIOCtrl( u8PhyDrvNb, CTRL_SYNC, 0 ) )
			{
				if ( 0 != DEBUG_ENABLE )
				{
					printf( "\ndisk_ioctl() failed.\r\n" );
				}
			}
			else
			{
				eResult = SUCCESS;
			}
		}
		if ( SUCCESS == eResult )
		{
			eResult = ERROR;
			u32Timer = HAL_GetTick( ) - u32Timer;
			pxTimes->u32Write = u32Timer;
			if ( 0 != DEBUG_ENABLE )
			{
				snprintf(	cBuffer,
							DEBUG_STRING_SIZE,
							"\n%lu bytes written and it took %lu timer ticks.\r\n",
							(ef_u32_t) u32BytesNb,
							(ef_u32_t) u32Timer );
				printf( cBuffer );
				snprintf(	cBuffer,
							DEBUG_STRING_SIZE,
							"Starting raw read test at sector %lu in %lu bytes of data chunks...",
							(ef_u32_t) u32LBAStart,
							(ef_u32_t) u32BufferSize );
				printf( cBuffer );
			}

			ef_u08_t		u8SectorCount	= 128;

			u32Timer = HAL_GetTick( );
			for ( u32Offset = 0 ; u32Offset < u32SectorCount ; u32Offset += ( (ef_u32_t) u8SectorCount ) )
			{
				eResult = ERROR;
				if ( ( (ef_u32_t) 128 ) > ( u32SectorCount - u32Offset ) )
				{
					u8SectorCount 	= ( u32SectorCount - u32Offset );
				}
				if ( EF_RET_OK !=  eEFPrvDriveRead( u8PhyDrvNb, pvBuffer, u32LBAStart + u32Offset, u8SectorCount ) )
				{
					if ( 0 != DEBUG_ENABLE )
					{
						printf( "\ndisk_read() failed.\r\n" );
					}
					break;
				}
				else
				{
					eResult = SUCCESS;
				}
			}
		}
		if ( SUCCESS == eResult )
		{
			u32Timer = HAL_GetTick( ) - u32Timer;
			pxTimes->u32Read = u32Timer;
			if ( 0 != DEBUG_ENABLE )
			{
				snprintf(	cBuffer,
							DEBUG_STRING_SIZE,
							"\n%lu bytes read and it took %lu timer ticks.\r\n",
							(ef_u32_t) u32BytesNb,
							(ef_u32_t) u32Timer );
				printf( cBuffer );
				printf( "Test completed.\r\n" );
			}
		}
	}
    return SUCCESS;
} /* s32TestSDCardRawSpeed */

ErrorStatus eTestSDCardFileTimes( ef_u32_t u32Bytes, ef_u08_t * pu8Buffer, test_sdcard_file_times_st * pxTimes )
{
	ErrorStatus eResult = ERROR;
	ef_u32_t		u32BytesRet = 0;            		/* Bytes written / read */
	FIL			xSdCardFile __attribute__ ((aligned (32)));
	const char	cTmpFileName[ ] = "TEMP.tmp";

    /* Create a file as new */
    if ( FR_OK != f_open( &xSdCardFile, cTmpFileName, FA_CREATE_ALWAYS | FA_WRITE ) )
	{
		/* This should never occur! */
		printf( "\r\nTemp File open Failed\r\n" );
	}
    else
    {
		ef_u32_t	u32Ticks = HAL_GetTick( );
		if ( FR_OK != f_write( &xSdCardFile, pu8Buffer, u32Bytes, &u32BytesRet ) )
		{
			/* This should never occur! */
			printf( "\r\nTemp File Write Failed\r\n" );
		}
		else if ( u32BytesRet != u32Bytes )
		{
			/* This should never occur! */
			printf( "\r\nTemp File Write Failed\r\n" );
		}
		else
		{
			eResult = SUCCESS;
		}
		pxTimes->u32Write = HAL_GetTick( ) - u32Ticks;
	}
	if ( SUCCESS == eResult )
	{
		eResult = ERROR;
		if ( FR_OK != f_close( &xSdCardFile ) )
		{
			/* This should never occur! */
			printf( "\r\nTemp File Close Failed\r\n" );
		}
		/* Open an existing file as */
		else if ( FR_OK != f_open( &xSdCardFile, cTmpFileName, FA_OPEN_EXISTING | FA_READ ) )
		{
			/* This should never occur! */
			printf( "\r\nTemp File open Failed\r\n" );
		}
		else
		{
			ef_u32_t	u32Ticks = HAL_GetTick( );
			if ( FR_OK != f_read( &xSdCardFile, pu8Buffer, u32Bytes, &u32BytesRet ) )
			{
				/* This should never occur! */
				printf( "\r\nTemp File Read Failed\r\n" );
			}
			else if ( u32BytesRet != u32Bytes )
			{
				/* This should never occur! */
				printf( "\r\nTemp File Read Failed\r\n" );
			}
			else
			{
				eResult = SUCCESS;
			}
			pxTimes->u32Read = HAL_GetTick( ) - u32Ticks;
			eResult = SUCCESS;
		}
	}

	if ( SUCCESS == eResult )
	{
		eResult = ERROR;
		if ( FR_OK != f_close( &xSdCardFile ) )
		{
			/* This should never occur! */
			printf( "\r\nTemp File Close Failed\r\n" );
		}
		if ( FR_OK != f_unlink( cTmpFileName ) )
		{
			/* This should never occur! */
			printf( "\r\nTemp File Unlink Failed\r\n" );
		}
		else
		{
			eResult = SUCCESS;
		}
	}

	return eResult;
} /* eTestSDCardFileTimes */
#endif
/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
