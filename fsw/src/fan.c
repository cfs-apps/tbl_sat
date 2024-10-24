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
*/

/*
** Include Files:
*/

#include <string.h>
#include "gpio.h"
#include "pwm.h"
#include "app_cfg.h"
#include "fan.h"




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
**   2. Can assume gpio_map() has been called and verified
**
*/
void FAN_Constructor(FAN_Class_t *FanPtr, INITBL_Class_t *IniTbl)
{
   
   Fan = FanPtr;
   
   memset(Fan, 0, sizeof(FAN_Class_t));

   Fan->A.PwmBcmId  = INITBL_GetIntConfig(IniTbl, CFG_FAN_A_PWM_BCM_ID);
   Fan->A.TachBcmId = INITBL_GetIntConfig(IniTbl, CFG_FAN_A_TACH_BCM_ID);
   Fan->B.PwmBcmId  = INITBL_GetIntConfig(IniTbl, CFG_FAN_B_PWM_BCM_ID);
   Fan->B.TachBcmId = INITBL_GetIntConfig(IniTbl, CFG_FAN_B_TACH_BCM_ID);

   if (pwm_map() >= 0)
   {
      Fan->PwmMapped = true;
      ConfigPwm(&Fan->A, TBL_SAT_FanId_A, PWM_CHANNEL0);
      ConfigPwm(&Fan->B, TBL_SAT_FanId_B, PWM_CHANNEL1);

   }
   else
   {
      Fan->PwmMapped = false;
      CFE_EVS_SendEvent (FAN_CONSTRUCTOR_EID, CFE_EVS_EventType_ERROR, 
                         "PWM mapping failed. Verify chip selection in rpi_iolib config.h and PWM pin assignments in JSON ini file");

   } /* End if PWM mapped */
    
} /* End FAN_Constructor() */


/******************************************************************************
** Function: FAN_OverridePwmCmd
**
** Override commanded PWM value.
**
** Notes:
**   1. The overridded value is not limited.
**
*/
bool FAN_OverridePwmCmd(void *DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{

   const TBL_SAT_OverrideFanPwm_CmdPayload_t *OverrideCmd = CMDMGR_PAYLOAD_PTR(MsgPtr, TBL_SAT_OverrideFanPwm_t);

   if (OverrideCmd->Duration == 0)
   {
      Fan->OverridePwmCmdEnabled = false;
   }
   else
   {
      Fan->OverridePwmCmdEnabled = true;
      Fan->OverridePwmCmdCount   = OverrideCmd->Duration;
      Fan->A.OverridePwmCmd = OverrideCmd->FanAPwm;
      Fan->B.OverridePwmCmd = OverrideCmd->FanBPwm;
      CFE_EVS_SendEvent (FAN_OVERRIDE_PWM_CMD_EID, CFE_EVS_EventType_INFORMATION,
                         "Starting fan PWM command override for %d cycles. Fan A PWM: %d, Fan B PWM: %d",
                         Fan->OverridePwmCmdCount, Fan->A.OverridePwmCmd, Fan->B.OverridePwmCmd);
   }

   return true;

} /* End FAN_OverridePwmCmd() */


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
   
   if (FanId == TBL_SAT_FanId_A)
   {
      Fan->A.PwmCmd = LimitedPwm;
      if (Fan->PwmMapped)
      {         
         if (Fan->OverridePwmCmdEnabled)
         {
            DAT_CHANNEL0 = Fan->A.OverridePwmCmd;
         }
         else
         {
            DAT_CHANNEL0 = Fan->A.PwmCmd;
         }
      }
   }
   else
   {
      Fan->B.PwmCmd = LimitedPwm;
      if (Fan->PwmMapped)
      {         
         if (Fan->OverridePwmCmdEnabled)
         {
            DAT_CHANNEL1 = Fan->B.OverridePwmCmd;
         }
         else
         {
            DAT_CHANNEL1 = Fan->B.PwmCmd;
         }
      }
   }
   if (Fan->OverridePwmCmdEnabled)
   {
      Fan->OverridePwmCmdCount--;
      if (Fan->OverridePwmCmdCount == 0)
      {
         Fan->OverridePwmCmdEnabled = false;
         CFE_EVS_SendEvent (FAN_SET_PWM_EID, CFE_EVS_EventType_INFORMATION,
                            "Override fan PWM command terminated");           
      }
   }
   
   return Limited;
   
} /* End FAN_SetPwm() */


/******************************************************************************
** Function: ConfigPwm
**
** Notes:
**   1. TODO - Document this mess and make a function
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
