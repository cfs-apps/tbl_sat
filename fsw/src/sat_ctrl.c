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
*/

/*
** Include Files:
*/

#include <string.h>
#include <math.h>
#include "gpio.h"
#include "app_cfg.h"
#include "sat_ctrl.h"

#define RAD_2_DEG 57.29577951326093

/**********************/
/** Global File Data **/
/**********************/

static SAT_CTRL_Class_t *SatCtrl = NULL;


/*******************************/
/** Local Function Prototypes **/
/*******************************/

static bool GetMqttMessages(bool PendForMsg);
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
   
   int32  SbStatus;
   
   SatCtrl = SatCtrlPtr;
   
   memset(SatCtrl, 0, sizeof(SAT_CTRL_Class_t));  
   
   SAT_CTRL_TBL_Constructor(&SatCtrl->Tbl);
   TBLMGR_RegisterTblWithDef(TblMgr, SAT_CTRL_TBL_NAME,
                             SAT_CTRL_TBL_LoadCmd, SAT_CTRL_TBL_DumpCmd,  
                             INITBL_GetStrConfig(IniTbl, CFG_SAT_CTRL_TBL_DEF));      
                    
   SatCtrl->Mode = TBL_SAT_CtrlMode_IDLE;
   SatCtrl->InitMode = true;
   SatCtrl->ExecPeriod = INITBL_GetIntConfig(IniTbl, CFG_SAT_CTRL_PERIOD);
   SatCtrl->ExecPerSec = 1000 / SatCtrl->ExecPeriod;
   SatCtrl->SunAcqMode.State = TBL_SAT_SunAcqState_UNDEF;
 
   /*
   ** Using ceil() causes last step to be limited if range is not an even number of steps
   ** Since starting with a 0 PWM value need to subtract 1 from the number of steps to reach max
   */
   SatCtrl->TestMode.PwmPerStep = (uint16)ceil((float)FAN_PWM_RANGE / (float)(SatCtrl->Tbl.Data.Test.Steps-1));

   if (gpio_map() >= 0) // map peripherals
   {

      SatCtrl->GpioMapped = true;
      FAN_Constructor(&SatCtrl->Fan, IniTbl);

   }
   else
   {
      SatCtrl->GpioMapped = false;
      CFE_EVS_SendEvent (SAT_CTRL_CONSTRUCTOR_EID, CFE_EVS_EventType_ERROR, 
                         "GPIO mapping failed. Verify chip selection in rpi_iolib config.h");

   } /* End if IO mapped */
 
   SatCtrl->MqttSensorTlmMid = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(IniTbl, CFG_MQTT_GW_TOPIC_4_TLM_TOPICID));
   SbStatus = CFE_SB_CreatePipe(&SatCtrl->MqttPipe, INITBL_GetIntConfig(IniTbl, CFG_SAT_CTRL_MQTT_PIPE_DEPTH), INITBL_GetStrConfig(IniTbl, CFG_SAT_CTRL_MQTT_PIPE_NAME));  
   if (SbStatus == CFE_SUCCESS)
   {
      SbStatus = CFE_SB_Subscribe(SatCtrl->MqttSensorTlmMid, SatCtrl->MqttPipe);
   }
   else
   {
      CFE_EVS_SendEvent (SAT_CTRL_CONSTRUCTOR_EID, CFE_EVS_EventType_ERROR, 
                         "SB pipe creation failed. Status = 0x%0X04", SbStatus);
   }

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
         SunAcqMode(); // Pends for MQTT message
         break;

      default:
         // TBL_SAT_CtrlMode_IDLE;
         // Pending for messages makes it easy to do overrides
         GetMqttMessages(true);
         break;
      
   } // End mode switch
   
   //TODO: Fix time in mode 
   SatCtrl->ExecCntr++;
   if (SatCtrl->ExecCntr % SatCtrl->ExecPerSec)
   {
      SatCtrl->TimeInMode++;
   }

   if (SatCtrl->Mode != TBL_SAT_CtrlMode_SUN_ACQ)
   {
      OS_TaskDelay(SatCtrl->ExecPeriod);
   }

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

   FAN_ResetStatus();
   
} /* End SAT_CTRL_ResetStatus() */


/******************************************************************************
** Function: SAT_CTRL_SetModeCmd
**
** TODO - Add mode range protection
*/
bool SAT_CTRL_SetModeCmd(void *DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{
   
   const TBL_SAT_SetCtrlMode_CmdPayload_t *SetCtrlMode = CMDMGR_PAYLOAD_PTR(MsgPtr, TBL_SAT_SetCtrlMode_t);
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
** Function: SAT_CTRL_SetCtrlGainsCmd
**
*/
bool SAT_CTRL_SetCtrlGainsCmd(void *DataObjPtr, const CFE_MSG_Message_t *MsgPtr)
{
   
   const TBL_SAT_SetCtrlGains_CmdPayload_t *SetCtrlGains = CMDMGR_PAYLOAD_PTR(MsgPtr, TBL_SAT_SetCtrlGains_t);
   bool RetStatus = true;
 
   float PrevPosGain  = SatCtrl->Tbl.Data.PosGain;
   float PrevRateGain = SatCtrl->Tbl.Data.RateGain;

   SatCtrl->Tbl.Data.PosGain  = SetCtrlGains->PosGain;
   SatCtrl->Tbl.Data.RateGain = SetCtrlGains->RateGain;
   
   CFE_EVS_SendEvent (SAT_CTRL_SET_CTRL_GAINS_EID, CFE_EVS_EventType_INFORMATION, 
                      "(Pos,Rate) control gains changed from (%0.6f,%0.6f) to (%0.6f,%0.6f)",
                      PrevPosGain,PrevRateGain,SatCtrl->Tbl.Data.PosGain,SatCtrl->Tbl.Data.RateGain);
  
   return RetStatus;   
   
} /* End SAT_CTRL_SetCtrlGainsCmd() */


/******************************************************************************
** Function: GetMqttMessages
**
** TODO: Expand beyond rate & provide status & delte time
*/
static bool GetMqttMessages(bool PendForMsg)
{
   
   int32  SbStatus = CFE_SUCCESS;
   int32  SysStatus;

   CFE_SB_Buffer_t  *SbBufPtr;
   CFE_SB_MsgId_t   MsgId = CFE_SB_INVALID_MSG_ID;

   SatCtrl->Mqtt.NewSensorTlm = false;

   if (PendForMsg)
   {
      SbStatus = CFE_SB_ReceiveBuffer(&SbBufPtr, SatCtrl->MqttPipe, CFE_SB_PEND_FOREVER);
   }
   else
   {
      OS_printf("*****Child polling for messages\n");
      SbStatus = CFE_SB_ReceiveBuffer(&SbBufPtr, SatCtrl->MqttPipe, CFE_SB_POLL);
   }
   
   if (SbStatus == CFE_SUCCESS)
   {
      SysStatus = CFE_MSG_GetMsgId(&SbBufPtr->Msg, &MsgId);

      if (SysStatus == CFE_SUCCESS)
      {

         if (CFE_SB_MsgId_Equal(MsgId,SatCtrl->MqttSensorTlmMid))
         {
            MQTT_GW_TblSatSensorTlm_t *SensorTlm = (MQTT_GW_TblSatSensorTlm_t *)&SbBufPtr->Msg;
            SatCtrl->Mqtt.NewSensorTlm = true;
            OS_printf("MQTT sensor payload: DT: %.6f, RateX: %.6f, RateY: %.6f, RateZ: %.6f, LuxA: %d, LuxB: %d\n",
                      SensorTlm->Payload.DeltaTime, SensorTlm->Payload.RateX, SensorTlm->Payload.RateY, 
                      SensorTlm->Payload.RateZ, SensorTlm->Payload.LuxA, SensorTlm->Payload.LuxB);
            memcpy(&SatCtrl->Mqtt.SensorTlm, &SbBufPtr->Msg, sizeof(MQTT_GW_TblSatSensorTlm_t));
         } 
         else
         {   
            CFE_EVS_SendEvent(SATCTRL_GET_MQTT_MSG_EID, CFE_EVS_EventType_ERROR,
                              "Received invalid MQTT packet, MID = 0x%04X", 
                              CFE_SB_MsgIdToValue(MsgId));
         }

      } /* End if got message ID */
   } /* End if received buffer */
   
   return SatCtrl->Mqtt.NewSensorTlm;

} /* End GetMqttMessages() */


/******************************************************************************
** Function: SunAcqMode 
**
*/
static void SunAcqMode(void)
{

   double LightDelta, AngleDelta, Ctrl;

   if (SatCtrl->InitMode)
   {
      SatCtrl->InitMode = false;
      SatCtrl->Sensor.SpinRate = 0.0;
      SatCtrl->Sensor.SpinRate = 0.0;      
      SatCtrl->SunAcqMode.State = TBL_SAT_SunAcqState_SURVEY;
      SatCtrl->SunAcqMode.SurveyMaxLight = 0;
      SatCtrl->SunAcqMode.SurveyRotation = 0.0;
      SatCtrl->SunAcqMode.LightIntensity = LIGHT_UNDEF;
   }
   
   if (GetMqttMessages(true))
   {
      SatCtrl->Sensor.TotalLight = 
          SatCtrl->Mqtt.SensorTlm.Payload.LuxA + 
          SatCtrl->Mqtt.SensorTlm.Payload.LuxB;
          
      SatCtrl->Sensor.SpinRate = SatCtrl->Mqtt.SensorTlm.Payload.RateZ*RAD_2_DEG;;
      AngleDelta = SatCtrl->Sensor.SpinRate * SatCtrl->Mqtt.SensorTlm.Payload.DeltaTime;
      
   }
   
   switch (SatCtrl->SunAcqMode.State)
   {
      case TBL_SAT_SunAcqState_SURVEY:
          SatCtrl->SunAcqMode.FanAPwmCmd = 0.0;
          SatCtrl->SunAcqMode.FanBPwmCmd = SatCtrl->Tbl.Data.SurveyFanPwm;
          SatCtrl->SunAcqMode.SurveyRotation += AngleDelta;
          OS_printf("SatCtrl->SunAcqMode.SurveyRotation: %.6f\n", SatCtrl->SunAcqMode.SurveyRotation);
          if (SatCtrl->SunAcqMode.SurveyRotation < 360.0)
          {
             if (SatCtrl->Sensor.TotalLight > SatCtrl->SunAcqMode.SurveyMaxLight)
             {
                SatCtrl->SunAcqMode.SurveyMaxLight = SatCtrl->Sensor.TotalLight;
             }
          }
          else
          {
             SatCtrl->SunAcqMode.State = TBL_SAT_SunAcqState_ACQUIRE;
             SatCtrl->SunAcqMode.AcquireTolerance = 
                (double)SatCtrl->SunAcqMode.SurveyMaxLight * 0.1;
          }
          break;
      case TBL_SAT_SunAcqState_ACQUIRE:
          // Leave fans in survey mode config to get within 10% of maxlight
          LightDelta = fabs((double)SatCtrl->SunAcqMode.SurveyMaxLight - (double)SatCtrl->Sensor.TotalLight);
          if (LightDelta < SatCtrl->SunAcqMode.AcquireTolerance)
          {
             SatCtrl->SunAcqMode.State = TBL_SAT_SunAcqState_HOLD;
          } 
          break;
      case TBL_SAT_SunAcqState_HOLD:
          Ctrl = SatCtrl->Sensor.SpinRate * SatCtrl->Tbl.Data.RateGain +
                 AngleDelta * SatCtrl->Tbl.Data.PosGain;
          OS_printf("Ctrl: %.6f\n", Ctrl);
          SatCtrl->SunAcqMode.FanAPwmCmd = 0.0;
          SatCtrl->SunAcqMode.FanBPwmCmd = 0.0;
          break;
      default:
          SatCtrl->InitMode = true;
          CFE_EVS_SendEvent(SATCTRL_SUN_ACQ_EID, CFE_EVS_EventType_ERROR,
             "Invlaid SunAcq state %d, resetting the controller", 
             SatCtrl->SunAcqMode.State);
   }
   
   FAN_SetPwm(TBL_SAT_FanId_A, SatCtrl->SunAcqMode.FanAPwmCmd);
   FAN_SetPwm(TBL_SAT_FanId_B, SatCtrl->SunAcqMode.FanBPwmCmd);
   
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
   
   FAN_SetPwm(TBL_SAT_FanId_A, SatCtrl->TestMode.CurPwm);
   FAN_SetPwm(TBL_SAT_FanId_B, SatCtrl->TestMode.CurPwm);
   
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
