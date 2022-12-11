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
**    Implement the Table Sat conroller class
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
#include <math.h>

#include "app_cfg.h"
#include "sat_ctrl.h"


/**********************/
/** Global File Data **/
/**********************/

static SAT_CTRL_Class_t *SatCtrl = NULL;


/*******************************/
/** Local Function Prototypes **/
/*******************************/

static void SunAcqMode(void);
static void TestMode(void);


/******************************************************************************
** Function: SAT_CTRL_Constructor
**
** Initialize the Table Sat Controller object to a known state
**
** Notes:
**   1. This must be called prior to any other function.
**   2. The default table must be loaded prior to initializing controller
**      data that requires the table parameters.
**
*/
void SAT_CTRL_Constructor(SAT_CTRL_Class_t *SatCtrlPtr, INITBL_Class_t *IniTbl,
                          TBLMGR_Class_t *TblMgr)
{
   
   SatCtrl = SatCtrlPtr;
   
   memset(SatCtrl, 0, sizeof(SAT_CTRL_Class_t));  
   
   SAT_CTRL_TBL_Constructor(&SatCtrl->Tbl, INITBL_GetStrConfig(IniTbl, CFG_APP_CFE_NAME));
   TBLMGR_RegisterTblWithDef(TblMgr, SAT_CTRL_TBL_LoadCmd, 
                             SAT_CTRL_TBL_DumpCmd,  
                             INITBL_GetStrConfig(IniTbl, CFG_SAT_CTRL_TBL_DEF));
                             
   SatCtrl->Mode = TBL_SAT_CtrlMode_IDLE;
   SatCtrl->InitMode = true;
   SatCtrl->ExecPeriod = INITBL_GetIntConfig(IniTbl, CFG_SAT_CTRL_PERIOD);
   SatCtrl->ExecPerSec = 1000 / SatCtrl->ExecPeriod;
 
   /*
   ** Using ceil() causes last step to be limited if range is not an even number of steps
   ** Since starting with a 0 PWM value need to subtract 1 from the number of steps to reach max
   */
   SatCtrl->TestMode.PwmPerStep = (uint16)ceil((float)FAN_PWM_RANGE / (float)(SatCtrl->Tbl.Data.Test.Steps-1));

   LIDET_Constructor(&SatCtrl->LiDet, IniTbl);
   IMU_Constructor(&SatCtrl->Imu, IniTbl);
   FAN_Constructor(&SatCtrl->Fan, IniTbl);
   
} /* End SAT_CTRL_Constructor() */


/******************************************************************************
** Function: SAT_CTRL_ChildTask
**
** Notes:
**   1. Control mode must be run prior to managing the execution time because
**      a TimeInMode value of zero is used as an initialization flag.
*/
bool SAT_CTRL_ChildTask(CHILDMGR_Class_t *ChildMgr)
{
   
   switch (SatCtrl->Mode)
   {
      case TBL_SAT_CtrlMode_TEST:
         TestMode();
         break;
         
      case TBL_SAT_CtrlMode_SUN_ACQ:
         SunAcqMode();
         break;

      default: // TBL_SAT_CtrlMode_IDLE;
         break;
      
   } // End mode switch
   
   SatCtrl->ExecCntr++;
   if (SatCtrl->ExecCntr % SatCtrl->ExecPerSec)
   {
      SatCtrl->TimeInMode++;
   }
   
   OS_TaskDelay(SatCtrl->ExecPeriod);

   return true;
   
} /* End SAT_CTRL_ChildTask() */


/******************************************************************************
** Function: SAT_CTRL_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
** Notes:
**   1. Any counter or variable that is reported in HK telemetry that doesn't
**      change the functional behavior should be reset.
**
*/
void SAT_CTRL_ResetStatus(void)
{

   SAT_CTRL_TBL_ResetStatus();

   LIDET_ResetStatus();
   IMU_ResetStatus();
   FAN_ResetStatus();
   
} /* End SAT_CTRL_ResetStatus() */


/******************************************************************************
** Function: SAT_CTRL_SetModeCmd
**
** TODO - Add mode range protection
*/
bool SAT_CTRL_SetModeCmd(void *DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{
   
   const TBL_SAT_SetCtrlMode_Payload_t *SetCtrlMode = CMDMGR_PAYLOAD_PTR(MsgPtr, TBL_SAT_SetCtrlMode_t);
   bool RetStatus = true;
   TBL_SAT_CtrlMode_Enum_t PrevMode = SatCtrl->Mode;
 
   SatCtrl->Mode = SetCtrlMode->NewMode;
   SatCtrl->InitMode = true;
   SatCtrl->TimeInMode = 0;
   
   CFE_EVS_SendEvent (SAT_CTRL_SET_MODE_EID, CFE_EVS_EventType_INFORMATION, 
                      "Control mode changed from %d to %d", PrevMode, SatCtrl->Mode);
  
   return RetStatus;   
   
} /* End SAT_CTRL_SetModeCmd() */


/******************************************************************************
** Function: SunAcqMode 
**
*/
static void SunAcqMode(void)
{
   
   return;
   
} /* End SunAcqMode() */


/******************************************************************************
** Function: TestMode 
**
*/
static void TestMode(void)
{
 
   if (SatCtrl->InitMode)
   {
      SatCtrl->InitMode = false;
      SatCtrl->TestMode.CurPwm        = 0;
      SatCtrl->TestMode.CurStep       = 1;
      SatCtrl->TestMode.CyclesInStep  = 0;
      SatCtrl->TestMode.CyclesPerStep = SatCtrl->Tbl.Data.Test.TimeInStep *
                                        SatCtrl->ExecPerSec;
      
      CFE_EVS_SendEvent (SAT_CTRL_TEST_MODE_EID, CFE_EVS_EventType_INFORMATION, 
                         "Test mode initialized: Steps %d, CyclesPerStep %d, PwmPerStep %d, TimeInStep %d",
                         SatCtrl->Tbl.Data.Test.Steps, SatCtrl->TestMode.CyclesPerStep,
                         SatCtrl->TestMode.PwmPerStep, SatCtrl->Tbl.Data.Test.TimeInStep);
   
   }
   
   FAN_SetPwm(TBL_SAT_FanId_1, SatCtrl->TestMode.CurPwm);
   FAN_SetPwm(TBL_SAT_FanId_2, SatCtrl->TestMode.CurPwm);
   
   SatCtrl->TestMode.CyclesInStep++;
   
   if (SatCtrl->TestMode.CyclesInStep > SatCtrl->TestMode.CyclesPerStep)
   {
      SatCtrl->TestMode.CyclesInStep = 0;

      SatCtrl->TestMode.CurStep++;
      if (SatCtrl->TestMode.CurStep > SatCtrl->Tbl.Data.Test.Steps)
      {
         SatCtrl->TestMode.CurPwm  = 0;
         SatCtrl->TestMode.CurStep = 1;
      }
      else
      {
         SatCtrl->TestMode.CurPwm  += SatCtrl->TestMode.PwmPerStep;
      }   
   }
 
} /* End TestMode() */