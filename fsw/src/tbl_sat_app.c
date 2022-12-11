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
**    Implement the Table Sat application
**
**  Notes:
**    1. See tbl_sat_app.h for details.
**
**  References:
**    1. OpenSatKit Object-based Application Developer's Guide
**    2. cFS Application Developer's Guide
**
*/

/*
** Includes
*/

#include <string.h>
#include "tbl_sat_app.h"
#include "tbl_sat_eds_cc.h"

/***********************/
/** Macro Definitions **/
/***********************/

/* Convenience macros */
#define  INITBL_OBJ    (&(TblSat.IniTbl))
#define  CMDMGR_OBJ    (&(TblSat.CmdMgr))
#define  TBLMGR_OBJ    (&(TblSat.TblMgr))
#define  CHILDMGR_OBJ  (&(TblSat.ChildMgr))
#define  SAT_CTRL_OBJ  (&(TblSat.SatCtrl))


/*******************************/
/** Local Function Prototypes **/
/*******************************/

static int32 InitApp(void);
static int32 ProcessCommands(void);
static void SendStatusTlm(void);


/**********************/
/** File Global Data **/
/**********************/

/* 
** Must match DECLARE ENUM() declaration in app_cfg.h
** Defines "static INILIB_CfgEnum IniCfgEnum"
*/
DEFINE_ENUM(Config,APP_CONFIG)  


/*****************/
/** Global Data **/
/*****************/

TBL_SAT_Class_t  TblSat;


/******************************************************************************
** Function: TBL_SAT_AppMain
**
*/
void TBL_SAT_AppMain(void)
{

   uint32 RunStatus = CFE_ES_RunStatus_APP_ERROR;


   CFE_EVS_Register(NULL, 0, CFE_EVS_NO_FILTER);

   if (InitApp() == CFE_SUCCESS) /* Performs initial CFE_ES_PerfLogEntry() call */
   {  
   
      RunStatus = CFE_ES_RunStatus_APP_RUN;
      
   }
   
   /*
   ** Main process loop
   */
   while (CFE_ES_RunLoop(&RunStatus))
   {

      RunStatus = ProcessCommands(); /* Pends indefinitely & manages CFE_ES_PerfLogEntry() calls */

   } /* End CFE_ES_RunLoop */

   CFE_ES_WriteToSysLog("TBL_SAT App terminating, err = 0x%08X\n", RunStatus);   /* Use SysLog, events may not be working */

   CFE_EVS_SendEvent(TBL_SAT_EXIT_EID, CFE_EVS_EventType_CRITICAL, "TBL_SAT App terminating, err = 0x%08X", RunStatus);

   CFE_ES_ExitApp(RunStatus);  /* Let cFE kill the task (and any child tasks) */

} /* End of TBL_SAT_AppMain() */


/******************************************************************************
** Function: TBL_SAT_NoOpCmd
**
*/

bool TBL_SAT_NoOpCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CFE_EVS_SendEvent (TBL_SAT_NOOP_EID, CFE_EVS_EventType_INFORMATION,
                      "No operation command received for TBL_SAT App version %d.%d.%d",
                      TBL_SAT_MAJOR_VER, TBL_SAT_MINOR_VER, TBL_SAT_PLATFORM_REV);

   return true;


} /* End TBL_SAT_NoOpCmd() */


/******************************************************************************
** Function: TBL_SAT_ResetAppCmd
**
** Notes:
**   1. No need to pass an object reference to contained objects because they
**      already have a reference from when they were constructed
**
*/

bool TBL_SAT_ResetAppCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr)
{

   CMDMGR_ResetStatus(CMDMGR_OBJ);
   CHILDMGR_ResetStatus(CHILDMGR_OBJ);
   
   SAT_CTRL_ResetStatus();
	  
   return true;

} /* End TBL_SAT_ResetAppCmd() */


/******************************************************************************
** Function: InitApp
**
*/
static int32 InitApp(void)
{

   int32 Status = OSK_C_FW_CFS_ERROR;
   
   CHILDMGR_TaskInit_t ChildTaskInit;
   
   /*
   ** Initialize objects 
   */

   if (INITBL_Constructor(&TblSat.IniTbl, TBL_SAT_INI_FILENAME, &IniCfgEnum))
   {
   
      TblSat.PerfId        = INITBL_GetIntConfig(INITBL_OBJ, CFG_APP_PERF_ID);
      TblSat.CmdMid        = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_TBL_SAT_CMD_TOPICID));
      TblSat.SendStatusMid = CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_BC_SCH_1_HZ_TOPICID));
      
      CFE_ES_PerfLogEntry(TblSat.PerfId);

      /* Constructor sends error events */    
      ChildTaskInit.TaskName  = INITBL_GetStrConfig(INITBL_OBJ, CFG_CHILD_NAME);
      ChildTaskInit.PerfId    = INITBL_GetIntConfig(INITBL_OBJ, CFG_CHILD_PERF_ID);
      ChildTaskInit.StackSize = INITBL_GetIntConfig(INITBL_OBJ, CFG_CHILD_STACK_SIZE);
      ChildTaskInit.Priority  = INITBL_GetIntConfig(INITBL_OBJ, CFG_CHILD_PRIORITY);
      Status = CHILDMGR_Constructor(CHILDMGR_OBJ, 
                                    ChildMgr_TaskMainCallback,
                                    SAT_CTRL_ChildTask, 
                                    &ChildTaskInit); 
  
   } /* End if INITBL Constructed */
  
   if (Status == CFE_SUCCESS)
   {

      /* Must constructor table manager prior to any app objects that contain tables */
      TBLMGR_Constructor(TBLMGR_OBJ);

      SAT_CTRL_Constructor(SAT_CTRL_OBJ, INITBL_OBJ, TBLMGR_OBJ);

      /*
      ** Initialize app level interfaces
      */
      
      CFE_SB_CreatePipe(&TblSat.CmdPipe, INITBL_GetIntConfig(INITBL_OBJ, CFG_CMD_PIPE_DEPTH), INITBL_GetStrConfig(INITBL_OBJ, CFG_CMD_PIPE_NAME));  
      CFE_SB_Subscribe(TblSat.CmdMid,        TblSat.CmdPipe);
      CFE_SB_Subscribe(TblSat.SendStatusMid, TblSat.CmdPipe);

      CMDMGR_Constructor(CMDMGR_OBJ);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_NOOP_CMD_FC,   NULL, TBL_SAT_NoOpCmd,     0);
      CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_RESET_CMD_FC,  NULL, TBL_SAT_ResetAppCmd, 0);

OS_printf("sizeof(TBL_SAT_SetCtrlMode_Payload_t)=%ld\n", sizeof(TBL_SAT_SetCtrlMode_Payload_t));
      CMDMGR_RegisterFunc(CMDMGR_OBJ, TBL_SAT_SET_CTRL_MODE_CC,  SAT_CTRL_OBJ, SAT_CTRL_SetModeCmd,  2); //sizeof(TBL_SAT_SetCtrlMode_Payload_t));
      
      CFE_MSG_Init(CFE_MSG_PTR(TblSat.StatusTlm.TelemetryHeader), CFE_SB_ValueToMsgId(INITBL_GetIntConfig(INITBL_OBJ, CFG_TBL_SAT_STATUS_TLM_TOPICID)), sizeof(TBL_SAT_StatusTlm_t));
   
      /*
      ** Application startup event message
      */
      CFE_EVS_SendEvent(TBL_SAT_INIT_APP_EID, CFE_EVS_EventType_INFORMATION,
                        "TBL_SAT App Initialized. Version %d.%d.%d",
                        TBL_SAT_MAJOR_VER, TBL_SAT_MINOR_VER, TBL_SAT_PLATFORM_REV);
                        
   } /* End if CHILDMGR constructed */
   
   return(Status);

} /* End of InitApp() */


/******************************************************************************
** Function: ProcessCommands
**
*/
static int32 ProcessCommands(void)
{

   int32  RetStatus = CFE_ES_RunStatus_APP_RUN;
   int32  SysStatus;

   CFE_SB_Buffer_t* SbBufPtr;
   CFE_SB_MsgId_t   MsgId = CFE_SB_INVALID_MSG_ID;
   

   CFE_ES_PerfLogExit(TblSat.PerfId);
   SysStatus = CFE_SB_ReceiveBuffer(&SbBufPtr, TblSat.CmdPipe, CFE_SB_PEND_FOREVER);
   CFE_ES_PerfLogEntry(TblSat.PerfId);

   if (SysStatus == CFE_SUCCESS)
   {
      
      SysStatus = CFE_MSG_GetMsgId(&SbBufPtr->Msg, &MsgId);
 
      if (SysStatus == CFE_SUCCESS)
      {
  
         if (CFE_SB_MsgId_Equal(MsgId, TblSat.CmdMid)) 
         {
            
            CMDMGR_DispatchFunc(CMDMGR_OBJ, &SbBufPtr->Msg);
         
         } 
         else if (CFE_SB_MsgId_Equal(MsgId, TblSat.SendStatusMid))
         {

            SendStatusTlm();
            
         }
         else
         {
            
            CFE_EVS_SendEvent(TBL_SAT_INVALID_MID_EID, CFE_EVS_EventType_ERROR,
                              "Received invalid command packet, MID = 0x%08X",
                              CFE_SB_MsgIdToValue(MsgId));
         } 

      }
      else
      {
         
         CFE_EVS_SendEvent(TBL_SAT_INVALID_MID_EID, CFE_EVS_EventType_ERROR,
                           "CFE couldn't retrieve message ID from the message, Status = %d", SysStatus);
      }
      
   } /* Valid SB receive */ 
   else 
   {
   
         CFE_ES_WriteToSysLog("TBL_SAT software bus error. Status = 0x%08X\n", SysStatus);   /* Use SysLog, events may not be working */
         RetStatus = CFE_ES_RunStatus_APP_ERROR;
   }  
      
   return RetStatus;

} /* End ProcessCommands() */


/******************************************************************************
** Function: SendStatusTlm
**
*/
static void SendStatusTlm(void)
{
   
   TBL_SAT_StatusTlm_Payload_t *StatusTlmPayload = &TblSat.StatusTlm.Payload;
   
   StatusTlmPayload->ValidCmdCnt   = TblSat.CmdMgr.ValidCmdCnt;
   StatusTlmPayload->InvalidCmdCnt = TblSat.CmdMgr.InvalidCmdCnt;

   /*
   ** Controller 
   */ 
   
   StatusTlmPayload->CtrlMode        = TblSat.SatCtrl.Mode;
   StatusTlmPayload->CtrlTimeInMode  = TblSat.SatCtrl.TimeInMode;

   StatusTlmPayload->FanIoMapped     = TblSat.SatCtrl.Fan.IoMapped;

   StatusTlmPayload->Fan1PwmCmd      = TblSat.SatCtrl.Fan.One.PwmCmd;
   StatusTlmPayload->Fan1PulsePerSec = TblSat.SatCtrl.Fan.One.PulsePerSec;

   StatusTlmPayload->Fan2PwmCmd      = TblSat.SatCtrl.Fan.Two.PwmCmd;
   StatusTlmPayload->Fan2PulsePerSec = TblSat.SatCtrl.Fan.Two.PulsePerSec;
   
   CFE_SB_TimeStampMsg(CFE_MSG_PTR(TblSat.StatusTlm.TelemetryHeader));
   CFE_SB_TransmitMsg(CFE_MSG_PTR(TblSat.StatusTlm.TelemetryHeader), true);
   
} /* End SendStatusTlm() */


