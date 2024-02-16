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
**    1. Both fans are contained in a single object so the singleton
**       design pattern can be used. The other option is to have one
**       object for each fan which means one set of functions and
**       separate data structures declared for each fan. This would
**       require a pointer to be passed as the first parameter in each
**       method. Since the fans are simple this seemed like overkill. 
**       Also the filename was kept singular.
**    2. Noctua NF-A4x10 5V PWM, https://noctua.at/en/nf-a4x10-5v-pwm
**    3. The original design included reading the fan tachometer. This
**       hardware wiring has been removed, but there are still some
**       software variables that have not been removed in case it is
**       added back in. One challenge is configuring interrupts.
**    TODO - Consider adding a map command if it fails during init.
**
*/

#ifndef _fan_
#define _fan_

/*
** Includes
*/

#include "app_cfg.h"
#include "pwm.h"

/***********************/
/** Macro Definitions **/
/***********************/

#define FAN_MIN_PWM       0
#define FAN_MAX_PWM    2047
#define FAN_PWM_RANGE  (FAN_MAX_PWM - FAN_MIN_PWM)

/*
** Event Message IDs
*/

#define FAN_CONSTRUCTOR_EID       (FAN_BASE_EID + 0)
#define FAN_OVERRIDE_PWM_CMD_EID  (FAN_BASE_EID + 1)
#define FAN_SET_PWM_EID           (FAN_BASE_EID + 2)


/**********************/
/** Type Definitions **/
/**********************/


typedef struct
{

   uint8  PwmBcmId;
   uint8  TachBcmId;
   
   uint16 PwmCmd;
   uint16 PulsePerSec;
   uint16 OverridePwmCmd;
   
   pwm_channel_config PwmChanCfg;
   
} FAN_Struct_t;

/******************************************************************************
** FAN_Class
*/

typedef struct
{

   /*
   ** Class State Data
   */
   
   bool    PwmMapped;   
   bool    OverridePwmCmdEnabled;
   uint32  OverridePwmCmdCount;

   FAN_Struct_t  A;
   FAN_Struct_t  B;
   

   
} FAN_Class_t;



/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: FAN_Constructor
**
** Initialize the Fan object to a known state
**
** Notes:
**   1. This must be called prior to any other function.
**
*/
void FAN_Constructor(FAN_Class_t *FanPtr, INITBL_Class_t *IniTbl);


/******************************************************************************
** Function: FAN_OverridePwmCmd
**
** Override the computed PWM with a value that is not limited. A duration of
** zero will stop an override.
*/
bool FAN_OverridePwmCmd(void *DataObjPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: FAN_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
** Notes:
**   1. Any counter or variable that is reported in HK telemetry that doesn't
**      change the functional behavior should be reset.
**
*/
void FAN_ResetStatus(void);


/******************************************************************************
** Function: FAN_SetPwm
**
** Return value indicates whether the PWM value was limited.
*/
bool FAN_SetPwm(TBL_SAT_FanId_Enum_t Fan, uint16 Pwm);


#endif /* _fan_ */
