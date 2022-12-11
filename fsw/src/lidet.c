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
**    Implement the Light Detector Class methods
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
#include "lidet.h"

/**********************/
/** Global File Data **/
/**********************/

static LIDET_Class_t *LiDet = NULL;


/*******************************/
/** Local Function Prototypes **/
/*******************************/


/******************************************************************************
** Function: LIDET_Constructor
**
** Initialize the LiDet object to a known state
**
** Notes:
**   1. This must be called prior to any other function.
**
*/
void LIDET_Constructor(LIDET_Class_t *LiDetPtr, INITBL_Class_t *IniTbl)
{
   
   LiDet = LiDetPtr;
   
   memset(LiDet, 0, sizeof(LIDET_Class_t));
   
} /* End LIDET_Constructor() */


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
void LIDET_ResetStatus(void)
{

   return;

} /* End LIDET_ResetStatus() */


