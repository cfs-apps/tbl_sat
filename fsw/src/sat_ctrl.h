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
**    Define the Table Sat controller class
**
**  Notes:
**    1. Conceptually GPIO could be a separate object but since 
**       mapping is the only function performed by the GPIO object 
**       it's part of SAT_CTRL. Also since SAT_CTRL contains all
**       of the GPIO objects connected to the GPIO it makes it easy.
**
*/

#ifndef _sat_ctrl_
#define _sat_ctrl_

/*
** Includes
*/
#include "mqtt_gw_eds_typedefs.h"

#include "app_cfg.h"
#include "sat_ctrl_tbl.h"
#include "fan.h"


/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define SAT_CTRL_CONSTRUCTOR_EID    (SAT_CTRL_BASE_EID + 0)
#define SAT_CTRL_SET_MODE_EID       (SAT_CTRL_BASE_EID + 1)
#define SAT_CTRL_SET_CTRL_GAINS_EID (SAT_CTRL_BASE_EID + 2)
#define SAT_CTRL_CHILD_TASK_EID     (SAT_CTRL_BASE_EID + 3)
#define SATCTRL_SUN_ACQ_EID         (SAT_CTRL_BASE_EID + 4)
#define SAT_CTRL_TEST_MODE_EID      (SAT_CTRL_BASE_EID + 5)
#define SATCTRL_GET_MQTT_MSG_EID    (SAT_CTRL_BASE_EID + 6)

/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Command Packets
** - See EDS command definitions in tbl_sat.xml
*/

/******************************************************************************
** Telmetery Packets
*/


/******************************************************************************
** SAT_CTRL_Class
*/

typedef enum
{
   LIGHT_UNDEF      = 1,
   LIGHT_INCREASING = 2,
   LIGHT_DECREASING = 3
   
} SAT_CTRL_LightIntensity_t;


typedef struct
{
   bool                       NewSensorTlm;
   MQTT_GW_TblSatSensorTlm_t  SensorTlm;
   
} SAT_CTRL_Mqtt_t;


typedef struct
{
   uint32  TotalLight;
   double  SpinRate;
   
} SAT_CTRL_Sensor_t;


typedef struct
{
   uint16  CurPwm;
   uint16  CurStep;
   uint16  PwmPerStep;
   uint16  CyclesInStep;
   uint16  CyclesPerStep;
   
} SAT_CTRL_TestMode_t;

typedef struct
{

   TBL_SAT_SunAcqState_Enum_t  State;
   double  PosErr;
   double  RateErr;
   uint16  FanAPwmCmd;
   uint16  FanBPwmCmd;
   uint32  SurveyMaxLight;
   double  SurveyRotation;
   double  AcquireTolerance;
   SAT_CTRL_LightIntensity_t  LightIntensity;
   
} SAT_CTRL_SunAcqMode_t;

typedef struct
{

   /*
   ** Framework References
   */
   
   INITBL_Class_t *IniTbl;


   /*
   ** Class State Data
   */

   bool    GpioMapped;
   
   uint32  ExecPeriod;  // Execution period in milliseconds
   uint16  ExecPerSec;                   
   uint32  ExecCntr;

   CFE_SB_PipeId_t   MqttPipe;
   CFE_SB_MsgId_t    MqttSensorTlmMid;
   SAT_CTRL_Mqtt_t   Mqtt;
   SAT_CTRL_Sensor_t Sensor;
   FAN_Class_t       Fan;
   
   TBL_SAT_CtrlMode_Enum_t  Mode;
   bool                     InitMode;
   uint32                   TimeInMode;
   SAT_CTRL_TBL_Class_t     Tbl;
   
   SAT_CTRL_TestMode_t      TestMode;
   SAT_CTRL_SunAcqMode_t    SunAcqMode;
         
} SAT_CTRL_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: SAT_CTRL_Constructor
**
** Initialize the GPIO Controller object to a known state
**
** Notes:
**   1. This must be called prior to any other function.
**
*/
void SAT_CTRL_Constructor(SAT_CTRL_Class_t *GpioCtrlPtr, INITBL_Class_t *IniTbl,
                          TBLMGR_Class_t *TblMgr);


/******************************************************************************
** Function: SAT_CTRL_ChildTask
**
*/
bool SAT_CTRL_ChildTask(CHILDMGR_Class_t *ChildMgr);


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
void SAT_CTRL_ResetStatus(void);


/******************************************************************************
** Function: SAT_CTRL_SetModeCmd
**
*/
bool SAT_CTRL_SetModeCmd(void *DataObjPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: SAT_CTRL_SetCtrlGainsCmd
**
*/
bool SAT_CTRL_SetCtrlGainsCmd(void *DataObjPtr, const CFE_MSG_Message_t *MsgPtr);


#endif /* _sat_ctrl_ */
