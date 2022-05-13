/**
 * ********************************************************************************************************************
 *  @file     ef_port_system.c
 *  @ingroup  group_eFAT_Portable
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Code file for OS Dependent Functions for eFAT.
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

#include <efat.h>
#include "ef_prv_def.h"
#include "stdio.h"

/* FreeRTOS */
//static const EF_SYNC_t xffSyncObjects[ EF_CONF_VOLUMES_NB ];  /** Table of FreeRTOS mutex */
/* CMSIS-RTOS */
//static const EF_SYNC_t xffSyncObjects[ EF_CONF_VOLUMES_NB ];  /** Table of CMSIS-RTOS mutex */
/* DEFAULT NO RTOS */
//const EF_SYNC_t xffSyncObjects[ EF_CONF_VOLUMES_NB ] = { 0 };
ef_u08_t u8ffSyncObjects[ EF_CONF_VOLUMES_NB ] = { 0 };

/* Create a Synchronization Object */
ef_return_et eEFPortSyncObjectCreate (
  ef_u08_t   u8Volume,
  EF_SYNC_t * pxSyncObject
)
{
  EF_ASSERT_PRIVATE( 0 != pxSyncObject );

//  ef_return_et  eRetVal = EF_RET_ERROR;
  /* FreeRTOS */
//  *pxSyncObject = xSemaphoreCreateMutex();
//  return (int)(*xSyncObject != NULL);

  /* CMSIS-RTOS */
//  *pxSyncObject = osMutexCreate( &Mutex[ u8Volume ] );
//  return (int)(*xSyncObject != NULL);

  /* DEFAULT NO RTOS */
  //xffSyncObjects[ u8Volume ]
  *pxSyncObject = &u8ffSyncObjects[ u8Volume ];

//  eRetVal = EF_RET_OK;

  return EF_RET_OK;
}


/* Delete a Synchronization Object */
ef_return_et eEFPortSyncObjectDelete (
  EF_SYNC_t xSyncObject
)
{
  ef_return_et eRetVal = EF_RET_OK;

//  ef_return_et eRetVal = EF_RET_OK;
  /* FreeRTOS */
//  vSemaphoreDelete(xSyncObject);
//  return 1;

  /* CMSIS-RTOS */
//  return (int)(osMutexDelete(xSyncObject) == osOK);

  /* DEFAULT NO RTOS */
  *xSyncObject = 0;
  //&xSyncObject = 0;
//  return eRetVal;
  return eRetVal;
}


/* Request Grant to Access the Volume */
ef_return_et eEFPortSyncObjectTake (
  EF_SYNC_t xSyncObject
)
{
  ef_return_et eRetVal = EF_RET_OK;

  /* FreeRTOS */
//  return (int)(xSemaphoreTake(xSyncObject, EF_CONF_TIMEOUT) == pdTRUE);

  /* CMSIS-RTOS */
//  return (int)(osMutexWait(xSyncObject, EF_CONF_TIMEOUT) == osOK);

  /* DEFAULT NO RTOS */
  if ( 0 == *xSyncObject )
  {
    *xSyncObject = 1;
  }
  else
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  return eRetVal;
}


/* Release Grant to Access the Volume */
ef_return_et eEFPortSyncObjectGive (
  EF_SYNC_t xSyncObject
)
{
  ef_return_et eRetVal = EF_RET_OK;

  /* FreeRTOS */
//  xSemaphoreGive(xSyncObject);

  /* CMSIS-RTOS */
//  osMutexRelease(xSyncObject);

  /* DEFAULT NO RTOS */
  if ( 0 < *xSyncObject )
  {
    *((ef_u08_t**) xSyncObject ) -= 1;
  }
  else
  {
    eRetVal = EF_RETURN_CODE_HANDLER( EF_RET_ERROR );
  }
  return eRetVal;
}

ef_return_et eEFPrvPortAssertFailed (
  char  * pcFile,
  int     iLine
)
{
  /* Suppress Unused warnings */
  (void) pcFile;
  (void) iLine;

  return EF_RET_ASSERT;
}

ef_return_et eEFPrvPortReturnCodeHandler (
  ef_return_et  eRetVal,
  char        * pcRetString,
  char        * pcFile,
  int           iLine
)
{
  /* Suppress Unused warnings */
//  (void) eRetVal;
//  (void) pcFile;
//  (void) iLine;

//  if ( EF_RET_OK != eRetVal )
//  {
    printf( "Return code Handling :" );
//    printf( "Event at line %i in file %s", iLine, pcFile );
    printf( "Return Code is %s at line %d in file %s",
            pcRetString,
            iLine,
            pcFile
    );
//    for (int i = 0 ; 32767 > i ; i++ );
    printf( "Return code Handled" );
//  }

  return eRetVal;
}
