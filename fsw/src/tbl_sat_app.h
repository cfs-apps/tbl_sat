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
**    Define the Table Sat application
**
**  Notes:
**    1. This app has been designed to facilitate swapping out hardware
**       sensors and actuators.  For example if a different fan is used
**       and as long as the term 'fan' can be used for the functional 
**       interface name then the only files that may have to change are
**       app_cfg.h, cpu1_tbl_sat_ini.json, and the fan.h, fan.c
**
*/

#ifndef _tbl_sat_app_
#define _tbl_sat_app_

/*
** Includes
*/

#include "app_cfg.h"
#include "sat_ctrl.h"

/***********************/
/** Macro Definitions **/
/***********************/

/*
** Events
*/

#define TBL_SAT_INIT_APP_EID    (TBL_SAT_BASE_EID + 0)
#define TBL_SAT_NOOP_EID        (TBL_SAT_BASE_EID + 1)
#define TBL_SAT_EXIT_EID        (TBL_SAT_BASE_EID + 2)
#define TBL_SAT_INVALID_MID_EID (TBL_SAT_BASE_EID + 3)


/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Command Packets
** - See EDS command definitions in tbl_sat.xml
*/


/******************************************************************************
** Telmetery Packets
** - See EDS command definitions in tbl_sat.xml
*/


/******************************************************************************
** TBL_SAT_Class
*/
typedef struct 
{

   /* 
   ** App Framework
   */ 
   
   INITBL_Class_t     IniTbl; 
   CFE_SB_PipeId_t    CmdPipe;
   CMDMGR_Class_t     CmdMgr;
   TBLMGR_Class_t     TblMgr;
   CHILDMGR_Class_t   ChildMgr;   
   
   /*
   ** Telemetry Packets
   */
   
   TBL_SAT_StatusTlm_t  StatusTlm;

   /*
   ** App State & Objects
   */ 
       
   uint32             PerfId;
   CFE_SB_MsgId_t     CmdMid;
   CFE_SB_MsgId_t     SendStatusMid;
   
   SAT_CTRL_Class_t   SatCtrl;
 
} TBL_SAT_Class_t;


/*******************/
/** Exported Data **/
/*******************/

extern TBL_SAT_Class_t  TblSat;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: TBL_SAT_AppMain
**
*/
void TBL_SAT_AppMain(void);


/******************************************************************************
** Function: TBL_SAT_NoOpCmd
**
*/
bool TBL_SAT_NoOpCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: TBL_SAT_ResetAppCmd
**
*/
bool TBL_SAT_ResetAppCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr);


#endif /* _tbl_sat_app_ */
