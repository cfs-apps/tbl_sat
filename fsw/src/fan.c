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
**    Implement the Fan Class methods
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
#include "fan.h"
#include "gpio.h"



/**********************/
/** Global File Data **/
/**********************/

static FAN_Class_t *Fan = NULL;


/*******************************/
/** Local Function Prototypes **/
/*******************************/

static void ConfigPwm(FAN_Struct_t *FanObj, TBL_SAT_FanId_Enum_t FanId, int Channel);

/******************************************************************************
** Function: FAN_Constructor
**
** Initialize the Fan object to a known state
**
** Notes:
**   1. This must be called prior to any other function.
**   2. TODO - Decide whether centralized Pi interface
**
*/
void FAN_Constructor(FAN_Class_t *FanPtr, INITBL_Class_t *IniTbl)
{
   
   Fan = FanPtr;
   
   memset(Fan, 0, sizeof(FAN_Class_t));

   Fan->One.PwmBcmId  = INITBL_GetIntConfig(IniTbl, CFG_FAN_1_PWM_BCM_ID);
   Fan->One.TachBcmId = INITBL_GetIntConfig(IniTbl, CFG_FAN_1_TACH_BCM_ID);
   Fan->Two.PwmBcmId  = INITBL_GetIntConfig(IniTbl, CFG_FAN_2_PWM_BCM_ID);
   Fan->Two.TachBcmId = INITBL_GetIntConfig(IniTbl, CFG_FAN_2_TACH_BCM_ID);

   if (gpio_map() < 0 || pwm_map() < 0) // map peripherals
   {
   
      Fan->IoMapped = false;
      CFE_EVS_SendEvent (FAN_CONSTRUCTOR_EID, CFE_EVS_EventType_ERROR, 
                         "GPIO or PWM mapping failed. Verify chip selection in pi_iolib config.h");

   }
   else
   {
      Fan->IoMapped = true;
      ConfigPwm(&Fan->One, TBL_SAT_FanId_1, PWM_CHANNEL0);
      ConfigPwm(&Fan->Two, TBL_SAT_FanId_2, PWM_CHANNEL1);

   } /* End if IO mapped */
    
} /* End FAN_Constructor() */


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
void FAN_ResetStatus(void)
{

   return;

} /* End FAN_ResetStatus() */


/******************************************************************************
** Function: FAN_SetPwm
**
** Notes:
**   1. TODO - Add PWM setting
**
*/
bool FAN_SetPwm(TBL_SAT_FanId_Enum_t FanId, uint16 Pwm)
{
   
   bool Limited = false;

   uint16 LimitedPwm = Pwm;

   if (LimitedPwm > FAN_MAX_PWM)
   {
      LimitedPwm = FAN_MAX_PWM;
      Limited = true;
   }
   
   if (FanId == TBL_SAT_FanId_1)
   {
      Fan->One.PwmCmd = LimitedPwm;
      if (Fan->IoMapped)
      {         
         DAT_CHANNEL0 = Fan->One.PwmCmd;
      }
   }
   else
   {
      Fan->Two.PwmCmd = LimitedPwm;
      if (Fan->IoMapped)
      {         
         DAT_CHANNEL1 = Fan->Two.PwmCmd;
      }
   }   
   
   return Limited;
   
} /* End FAN_SetPwm() */

/******************************************************************************
** Function: ConfigPwm
**
** Notes:
**   1. TODO - Document magic numbers
**
*/
static void ConfigPwm(FAN_Struct_t *FanObj, TBL_SAT_FanId_Enum_t FanId, int Channel)
{

      gpio_func(FanObj->PwmBcmId, ALT5); // Alternate function 5 is PWM

      FanObj->PwmChanCfg.pwm_register.pwm_bitfield.mode = PWM_CTL_MODE_PWM;
      FanObj->PwmChanCfg.pwm_register.pwm_bitfield.rptl = PWM_RPTL_STOP;
      FanObj->PwmChanCfg.pwm_register.pwm_bitfield.sbit = PWM_SBIT_LOW;
      FanObj->PwmChanCfg.pwm_register.pwm_bitfield.pola = PWM_POLA_DEFAULT;
      FanObj->PwmChanCfg.pwm_register.pwm_bitfield.usef = PWM_USEF_DATA;
      FanObj->PwmChanCfg.pwm_register.pwm_bitfield.msen = PWM_MSEN_MSRATIO;
      FanObj->PwmChanCfg.divisor =   19;
      FanObj->PwmChanCfg.range   = 1023;

      pwm_configure(Channel, &FanObj->PwmChanCfg);
      pwm_enable(Channel);

      CFE_EVS_SendEvent (FAN_CONSTRUCTOR_EID, CFE_EVS_EventType_INFORMATION, 
                         "Fan %d configured on PWM Channel %d BCM Pin %d", 
                         FanId, Channel, FanObj->PwmBcmId);

} /* End ConfigPwm() */