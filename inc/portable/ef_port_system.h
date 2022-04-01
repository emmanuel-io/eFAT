/**
 * ********************************************************************************************************************
 *  @file     ef_port_system.h
 *  @ingroup  group_eFAT_Portable
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Header file for portable system functions.
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
#ifndef EFAT_PORTABLE_SYSTEM_DEFINED
#define EFAT_PORTABLE_SYSTEM_DEFINED
#ifdef __cplusplus
  extern "C" {
#endif
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */
#include "ef_def.h"
/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */

/* FreeRTOS */
//  typedef  SemaphoreHandle_t  EF_SYNC_t;
/* CMSIS-RTOS */
//  typedef  osMutexDef_t EF_SYNC_t;
/* DEFAULT NO RTOS */
/**
 *  Object for synchronisation
 */
typedef  ef_u08_t* EF_SYNC_t;

  /* Public functions prototypes---------------------------------------------- */

/* RTC function */
/**
 *  @brief  Get RTC Time in FAT file system formating style
 *
 *  @return The RTC time packed in a unsigned 32 bits vector
 */
ef_u32_t u32EFPortFatTimeGet (
  void
);

/**
 *  @brief  Create a Synchronization Object
 *          This function is called in f_mount() function to create a new
 *          synchronization object for the volume, such as semaphore and mutex.
 *
 *  @param  u8Volume      Corresponding volume (logical drive number)
 *  @param  pxSyncObject  Pointer to return the created sync object
 *
 *  @return Operation result
 *  @retval EF_RET_OK           Success
 *  @retval EF_RET_SYS_ERROR    An error occurred
 *  @retval EF_RET_ASSERT       Assertion failed
 */
ef_return_et eEFPortSyncObjectCreate (
  ef_u08_t   u8Volume,
  EF_SYNC_t * pxSyncObject
);

/**
 *  @brief  Delete a Synchronization Object
 *          This function is called in f_mount() function to delete a
 *          synchronization object that created with eEFPortSyncObjectCreate() function.
 *
 *  @param  xSyncObject  Sync object tied to the logical drive to be deleted
 *
 *  @return Operation result
 *  @retval EF_RET_OK         Success
 *  @retval EF_RET_SYS_ERROR  An error occurred
 *  @retval EF_RET_ASSERT     Assertion failed
 */
ef_return_et eEFPortSyncObjectDelete (
  EF_SYNC_t xSyncObject
);

/**
 *  @brief  Request Grant to Access the Volume
 *          This function is called on entering file functions to lock the volume.
 *
 *  @param  xSyncObject  Sync object to wait
 *
 *  @return Operation result
 *  @retval EF_RET_OK         Success
 *  @retval EF_RET_SYS_ERROR  An error occurred
 *  @retval EF_RET_ASSERT     Assertion failed
 */

/* Lock sync object */
ef_return_et eEFPortSyncObjectTake (
  EF_SYNC_t xSyncObject
);
/**
 *  @brief  Release Grant to Access the Volume
 *          This function is called on leaving file functions to unlock the volume.
 *
 *  @param  xSyncObject  Sync object to be signaled
 *
 *  @return Operation result
 *  @retval EF_RET_OK         Success
 *  @retval EF_RET_SYS_ERROR  An error occurred
 *  @retval EF_RET_ASSERT     Assertion failed
 */
ef_return_et eEFPortSyncObjectGive (
  EF_SYNC_t xSyncObject
);

//#endif

/**
 *  @brief  Portable Assertion failure function
 *
 *  @param  pcFile  Pointer to the file name string
 *  @param  iLine   Line number where the assertion failed
 *
 *  @return Operation result
 *  @retval EF_RET_ASSERT     Assertion failed
 */
ef_return_et eEFPrvPortAssertFailed (
  char  * pcFile,
  int     iLine
);


/**
 *  @brief  Portable return code handler function
 *
 *  @param  eRetVal The return code to return
 *  @param  pcFile  Pointer to the file name string
 *  @param  iLine   Line number where the assertion failed
 *
 *  @return Operation result
 *  @retval EF_RET_ASSERT     Assertion failed
 */
ef_return_et eEFPrvPortReturnCodeHandler (
  ef_return_et  eRetVal,
  char        * pcRetString,
  char        * pcFile,
  int           iLine
);

/* ***************************************************************************************************************** */
#ifdef __cplusplus
}
#endif
#endif /* EFAT_PORTABLE_SYSTEM_DEFINED */
/* END OF FILE ***************************************************************************************************** */
