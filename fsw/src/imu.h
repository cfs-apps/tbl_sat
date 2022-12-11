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
**    Define GPIO Controller class
**
**  Notes:
**    1. TODO - Add IMU reference
**    3. TODO - Consider adding a map command if it fails during init. 
**
**  References:
**    1. OpenSatKit Object-based Application Developer's Guide.
**    2. cFS Application Developer's Guide.
**
*/

#ifndef _imu_
#define _imu_

/*
** Includes
*/

#include "app_cfg.h"


/***********************/
/** Macro Definitions **/
/***********************/

/*
** Event Message IDs
*/

#define IMU_CONSTRUCTOR_EID  (IMU_BASE_EID + 0)


/**********************/
/** Type Definitions **/
/**********************/

typedef struct 
{

   float  X;
   float  Y;
   float  Z;

} IMU_Axis_t;


/******************************************************************************
** IMU_Class
*/

typedef struct
{

   /*
   ** Class State Data
   */

   IMU_Axis_t  Rate;
   
} IMU_Class_t;



/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: IMU_Constructor
**
** Initialize the Imu object to a known state
**
** Notes:
**   1. This must be called prior to any other function.
**
*/
void IMU_Constructor(IMU_Class_t *ImuPtr, INITBL_Class_t *IniTbl);


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
void IMU_ResetStatus(void);


#endif /* _imu_ */
