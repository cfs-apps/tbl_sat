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
   
      CFE_EVS_SendEvent (FAN_CONSTRUCTOR_EID, CFE_EVS_EventType_ERROR, 
                         "GPIO or PWM mapping failed");
      Fan->IoMapped = false;

   }
   else
   {
      Fan->IoMapped = true;  
      gpio_func(Fan->One.PwmBcmId, ALT5); // Alternate function 5 is PWM

      // TODO - Document this mess      
      Fan->One.PwmChanCfg.pwm_register.pwm_bitfield.mode = PWM_CTL_MODE_PWM;
      Fan->One.PwmChanCfg.pwm_register.pwm_bitfield.rptl = PWM_RPTL_STOP;
      Fan->One.PwmChanCfg.pwm_register.pwm_bitfield.sbit = PWM_SBIT_LOW;
      Fan->One.PwmChanCfg.pwm_register.pwm_bitfield.pola = PWM_POLA_DEFAULT;
      Fan->One.PwmChanCfg.pwm_register.pwm_bitfield.usef = PWM_USEF_DATA;
      Fan->One.PwmChanCfg.pwm_register.pwm_bitfield.msen = PWM_MSEN_MSRATIO;
      Fan->One.PwmChanCfg.divisor = 1953;
      Fan->One.PwmChanCfg.range   = 5120;

      pwm_configure(PWM_CHANNEL0, &Fan->One.PwmChanCfg);
      pwm_enable(PWM_CHANNEL0);

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
   }   
   
   return Limited;
   
} /* End FAN_SetPwm() */

