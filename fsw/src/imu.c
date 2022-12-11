/*
**  Copyright 2022 bitValence, Inc.
**  All Rights Reserved.
**
**  This program is free software; you can modify and/or redistribute it
**  under the terms of the GNU Affero General Public License
**  as published by the Free Software Foundation; version 3 with
**  attribution addendums as found in the LICENSE.txt
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Affero General Public License for more details.
**
**  Purpose:
**    Implement the IMU Class methods
**
**  Notes:
**    None
**
**  References:
**    1. OpenSatKit Object-based Application Developer's Guide.
**    2. cFS Application Developer's Guide.
**
*/

/*
** Include Files:
*/

#include <string.h>

#include "app_cfg.h"
#include "imu.h"

/**********************/
/** Global File Data **/
/**********************/

static IMU_Class_t *Imu = NULL;


/*******************************/
/** Local Function Prototypes **/
/*******************************/


/******************************************************************************
** Function: IMU_Constructor
**
** Initialize the Imu object to a known state
**
** Notes:
**   1. This must be called prior to any other function.
**
*/
void IMU_Constructor(IMU_Class_t *ImuPtr, INITBL_Class_t *IniTbl)
{
   
   Imu = ImuPtr;
   
   memset(Imu, 0, sizeof(IMU_Class_t));
   
} /* End IMU_Constructor() */


/******************************************************************************
** Function: IMU_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
** Notes:
**   1. Any counter or variable that is reported in HK telemetry that doesn't
**      change the functional behavior should be reset.
**
*/
void IMU_ResetStatus(void)
{

   return;

} /* End IMU_ResetStatus() */

