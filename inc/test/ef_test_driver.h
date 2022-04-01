/**
 * ********************************************************************************************************************
 *  @file     ef_test_driver.h
 *  @ingroup  GroupeFATTest
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Header for functions for testing sdcard functionalities.
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
#ifndef EFAT_TEST_DRIVER_H
#define EFAT_TEST_DRIVER_H

#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include "efat.h"

/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */

#if 0
/**
 *  @brief  Test function return code (ef_test_return_et)
 */
typedef enum {
  EF_RET_OK = 0,              /**< (0) Succeeded */
  *  @retval 0 Everything went well !
  *  @retval 1 Insufficient work area to run the program.
  *  @retval 2 Disk_initalizationfailed
  *  @retval 3 Get drive size failed
  *  @retval 4 Insufficient drive size to test.
  *  @retval 5 Get sector size failed.
  *  @retval 6 Single sector write test disk_write failed
  *  @retval 7 Single sector write test disk_ioctl(CTRL_SYNC) failed
  *  @retval 8 Single sector write test disk readback failed
  *  @retval 10  Single sector write test Read data differs from the data written
  *  @retval 11  Multiple sector write test disk_write failed
  *  @retval 12  Multiple sector write test disk_ioctl(CTRL_SYNC) failed
  *  @retval 13  Multiple sector write test disk readback failed
  *  @retval 14  Multiple sector write test Read data differs from the data written
  *  @retval 15  Single sector write test (unaligned buffer address) disk_write failed
  *  @retval 16  Single sector write test (unaligned buffer address) disk_ioctl(CTRL_SYNC) failed
  *  @retval 17  Single sector write test (unaligned buffer address) disk readback failed
  *  @retval 18  Single sector write test (unaligned buffer address) Read data differs from the data written
  *  @retval 19  4GB barrier test disk_write failed 1
  *  @retval 20  4GB barrier test disk_write failed 2
  *  @retval 21  4GB barrier test disk_ioctl(CTRL_SYNC) failed
  *  @retval 22  4GB barrier test disk readback failed 1
  *  @retval 23  4GB barrier test disk readback failed 2
  *  @retval 24  4GB barrier test Read data differs from the data written
} ef_test_return_et;
#endif
/* Local function macros ------------------------------------------------------------------------------------------- */

/**
 *	@brief	Test the SD Card DiskIO functionalities
 *			Low level disk I/O module function checker
 *
 *	@note	WARNING: The data on the target drive will be lost!
 *		int rc;
 *		DWORD buff[EF_MAX_SS];  Working buffer (4 sector in size)
 *
 *		Check function/compatibility of the physical drive #0
 *
 *		rc = s32TestSdCardDiskIO(0, 3, buff, sizeof buff);
 *
 *		if (rc)
 *		{
 *			printf( "Sorry the function/compatibility test failed. (rc=%d)\nFatFs will not work with this disk driver.\n", rc );
 *		}
 *		else
 *		{
 *			printf( "Congratulations! The disk driver works well.\n" );
 *		}
 *
 *	@param	u8PhyDrvNb	  Physical drive number to be checked (all data on the drive will be lost)
 *	@param	u32Cycles	    Number of test cycles
 *	@param	pu8Buffer	    Pointer to the working buffer
 *	@param	u32BufferSize	Size of the working buffer in unit of byte
 *
 *	@return The test check Failure Id
 *	@retval 0	Everything went well !
 *	@retval 1	Insufficient work area to run the program.
 *	@retval 2	Disk_initalizationfailed
 *	@retval 3	Get drive size failed
 *	@retval 4	Insufficient drive size to test.
 *	@retval 5	Get sector size failed.
 *	@retval 6	Single sector write test disk_write failed
 *	@retval 7	Single sector write test disk_ioctl(CTRL_SYNC) failed
 *	@retval 8	Single sector write test disk readback failed
 *	@retval 10	Single sector write test Read data differs from the data written
 *	@retval 11	Multiple sector write test disk_write failed
 *	@retval 12	Multiple sector write test disk_ioctl(CTRL_SYNC) failed
 *	@retval 13	Multiple sector write test disk readback failed
 *	@retval 14	Multiple sector write test Read data differs from the data written
 *	@retval 15	Single sector write test (unaligned buffer address) disk_write failed
 *	@retval 16	Single sector write test (unaligned buffer address) disk_ioctl(CTRL_SYNC) failed
 *	@retval 17	Single sector write test (unaligned buffer address) disk readback failed
 *	@retval 18	Single sector write test (unaligned buffer address) Read data differs from the data written
 *	@retval 19	4GB barrier test disk_write failed 1
 *	@retval 20	4GB barrier test disk_write failed 2
 *	@retval 21	4GB barrier test disk_ioctl(CTRL_SYNC) failed
 *	@retval 22	4GB barrier test disk readback failed 1
 *	@retval 23	4GB barrier test disk readback failed 2
 *	@retval 24	4GB barrier test Read data differs from the data written
 */
int32_t s32TestPrvDrive (
  ef_u08_t   u8PhyDrvNb,
  ef_u32_t   u32Cycles,
  ef_u08_t * pu8Buffer,
  ef_u32_t   u32BufferSize
);

#if 0
/**
 * @brief	Test the SD Card Raw Speed Read/Write Throughput
 *
 *	@param	pdrv		Physical drive number
 *	@param	lba			Start LBA for read/write test
 *	@param	len			Number of bytes to read/write (must be multiple of sz_buff)
 *	@param	buff		Read/write buffer
 *	@param	sz_buff		Size of read/write buffer (must be multiple of EF_MAX_SS)
 *	@param	pxTimes		Pointer to transactions time structure (in ticks)
 *
 *	@return The test check performed
 *	@retval ERROR	if operation is not permitted or test failed
 *	@retval SUCCESS	if test successful
 */
ErrorStatus eTestSDCardRawSpeed (
	ef_u08_t		pdrv,		/* Physical drive number */
	ef_u32_t	lba,		/* Start LBA for read/write test */
	ef_u32_t	len,		/* Number of bytes to read/write (must be multiple of sz_buff) */
	void*		  buff,		  /* Read/write buffer */
	ef_u32_t	sz_buff,	/* Size of read/write buffer (must be multiple of EF_MAX_SS) */
	test_sdcard_raw_times_st * pxTimes	/* Pointer to transaction time in ticks, first write, then read */
);

/**
 * @brief	Test the SD Card File Speed Read/Write Throughput
 *
 *	@param	u32Bytes	Number of bytes to read/write
 *	@param	pu8Buffer	Read/write buffer
 *	@param	pxTimes		Pointer to transactions time structure (in ticks)
 *
 *	@return	status of the operation
 *	@retval	SUCCESS if everything is fine
 *	@retval	ERROR if file opening failed
 */

ErrorStatus	eTestSDCardFileTimes (
	ef_u32_t	u32Bytes,
	ef_u08_t *	pu8Buffer,
	test_sdcard_file_times_st * pxTime
);
#endif

/* Local variables ------------------------------------------------------------------------------------------------- */
/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_TEST_DRIVER_H */
/* END OF FILE ***************************************************************************************************** */
