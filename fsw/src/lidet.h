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
**    Define Light Detector class
**
**  Notes:
**    None 
**
**  References:
**    1. OpenSatKit Object-based Application Developer's Guide.
**    2. cFS Application Developer's Guide.
**
*/

#ifndef _lidet_
#define _lidet_

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

#define LIDET_CONSTRUCTOR_EID  (LIDET_BASE_EID + 0)

/***********************/
/** Macro Definitions **/
/***********************/



/**********************/
/** Type Definitions **/
/**********************/


typedef struct
{

   uint16  Counts;
   float   Angle;
   
} LIDET_Struct_t;

/******************************************************************************
** LIDET_Class
*/

typedef struct
{

   /*
   ** Class State Data
   */

   LIDET_Struct_t  One;
   LIDET_Struct_t  Two;
   LIDET_Struct_t  Three;
   
} LIDET_Class_t;



/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: LIDET_Constructor
**
** Initialize the Light Detector object to a known state
**
** Notes:
**   1. This must be called prior to any other function.
**
*/
void LIDET_Constructor(LIDET_Class_t *LiDetPtr, INITBL_Class_t *IniTbl);


/******************************************************************************
** Function: LIDET_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
** Notes:
**   1. Any counter or variable that is reported in HK telemetry that doesn't
**      change the functional behavior should be reset.
**
*/
void LIDET_ResetStatus(void);


#endif /* _lidet_ */
