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
**    Implement the Histogram table
**
**  Notes:
**    1. The static "TblData" serves as a table load buffer. Table dump data is
**       read directly from table owner's table storage.
**
*/

/*
** Include Files:
*/

#include <string.h>
#include "sat_ctrl_tbl.h"


/***********************/
/** Macro Definitions **/
/***********************/


/**********************/
/** Type Definitions **/
/**********************/


/************************************/
/** Local File Function Prototypes **/
/************************************/

static bool LoadJsonData(size_t JsonFileLen);


/**********************/
/** Global File Data **/
/**********************/

static SAT_CTRL_TBL_Class_t *SatCtrlTbl = NULL;

static SAT_CTRL_TBL_Data_t TblData; /* Working buffer for loads */

static CJSON_Obj_t JsonTblObjs[] = {

   /* Table Data Address        Table Data Length                 Updated,  Data Type,  Float,  core-json query string,  length of query string(exclude '\0') */
   
   { &TblData.SurveyFanPwm,     sizeof(TblData.SurveyFanPwm),     false,    JSONNumber, false,  { "survey-fan-pwm",      (sizeof("survey-fan-pwm")-1)}      },
   { &TblData.PosGain,          sizeof(TblData.PosGain),          false,    JSONNumber, true,   { "pos-gain",            (sizeof("pos-gain")-1)}            },
   { &TblData.RateGain,         sizeof(TblData.RateGain),         false,    JSONNumber, true,   { "rate-gain",           (sizeof("rate-gain")-1)}           },
   { &TblData.Test.Steps,       sizeof(TblData.Test.Steps),       false,    JSONNumber, false,  { "test-steps",          (sizeof("test-steps")-1)}          },
   { &TblData.Test.TimeInStep,  sizeof(TblData.Test.TimeInStep),  false,    JSONNumber, false,  { "test-time-in-step",   (sizeof("test-time-in-step")-1)}   }

};


/******************************************************************************
** Function: SAT_CTRL_TBL_Constructor
**
** Notes:
**    1. This must be called prior to any other functions
**
*/
void SAT_CTRL_TBL_Constructor(SAT_CTRL_TBL_Class_t *SatCtrlTblPtr)
{

   SatCtrlTbl = SatCtrlTblPtr;

   CFE_PSP_MemSet(SatCtrlTbl, 0, sizeof(SAT_CTRL_TBL_Class_t));   
   
   SatCtrlTbl->JsonObjCnt = (sizeof(JsonTblObjs)/sizeof(CJSON_Obj_t));
         
} /* End SAT_CTRL_TBL_Constructor() */


/******************************************************************************
** Function: SAT_CTRL_TBL_DumpCmd
**
** Notes:
**  1. Function signature must match TBLMGR_DumpTblFuncPtr_t.
**  2. Can assume valid table filename because this is a callback from 
**     the app framework table manager that has verified the file.
**  3. DumpType is unused.
**  4. File is formatted so it can be used as a load file. It does not follow
**     the cFE table file format. 
**  5. Creates a new dump file, overwriting anything that may have existed
**     previously
*/
bool SAT_CTRL_TBL_DumpCmd(osal_id_t FileHandle)
{

   char DumpRecord[256];
   sprintf(DumpRecord,"   \"pos-gain\": %0.6f,\n", SatCtrlTbl->Data.PosGain);
   OS_write(FileHandle, DumpRecord, strlen(DumpRecord));

   sprintf(DumpRecord,"   \"rate-gain\": %0.6f,\n", SatCtrlTbl->Data.RateGain);
   OS_write(FileHandle, DumpRecord, strlen(DumpRecord));

   sprintf(DumpRecord,"   \"steps\": %d,\n", SatCtrlTbl->Data.Test.Steps);
   OS_write(FileHandle, DumpRecord, strlen(DumpRecord));

   sprintf(DumpRecord,"   \"time-in-step\": %d,\n", SatCtrlTbl->Data.Test.TimeInStep);
   OS_write(FileHandle, DumpRecord, strlen(DumpRecord));

   sprintf(DumpRecord,"   }\n");
  OS_write(FileHandle, DumpRecord, strlen(DumpRecord));

   return true;
   
} /* End of SAT_CTRL_TBL_DumpCmd() */


/******************************************************************************
** Function: SAT_CTRL_TBL_LoadCmd
**
** Notes:
**  1. Function signature must match TBLMGR_LoadTblFuncPtr_t.
*/
bool SAT_CTRL_TBL_LoadCmd(APP_C_FW_TblLoadOptions_Enum_t LoadType, const char *Filename)
{

   bool  RetStatus = false;

   if (CJSON_ProcessFile(Filename, SatCtrlTbl->JsonBuf, SAT_CTRL_TBL_JSON_FILE_MAX_CHAR, LoadJsonData))
   {
      SatCtrlTbl->Loaded = true;
      RetStatus = true;
   }

   return RetStatus;
   
} /* End SAT_CTRL_TBL_LoadCmd() */


/******************************************************************************
** Function: SAT_CTRL_TBL_ResetStatus
**
*/
void SAT_CTRL_TBL_ResetStatus(void)
{

   SatCtrlTbl->LastLoadCnt = 0;
 
} /* End SAT_CTRL_TBL_ResetStatus() */


/******************************************************************************
** Function: LoadJsonData
** 
** Notes:
**  1. See file prologue for full/partial table load scenarios
*/
static bool LoadJsonData(size_t JsonFileLen)
{

   bool      RetStatus = false;
   size_t    ObjLoadCnt;


   SatCtrlTbl->JsonFileLen = JsonFileLen;

   /* 
   ** 1. Copy table owner data into local table buffer
   ** 2. Process JSON file which updates local table buffer with JSON supplied values
   ** 3. If valid, copy local buffer over owner's data 
   */
   
   memcpy(&TblData, &SatCtrlTbl->Data, sizeof(SAT_CTRL_TBL_Data_t));
   
   ObjLoadCnt = CJSON_LoadObjArray(JsonTblObjs, SatCtrlTbl->JsonObjCnt, SatCtrlTbl->JsonBuf, SatCtrlTbl->JsonFileLen);

   /* Only accept fixed sized bin arrays */
   if (!SatCtrlTbl->Loaded && (ObjLoadCnt != SatCtrlTbl->JsonObjCnt))
   {

      CFE_EVS_SendEvent(SAT_CTRL_TBL_LOAD_EID, CFE_EVS_EventType_ERROR, 
                        "Table has never been loaded and new table only contains %d of %d data objects",
                        (unsigned int)ObjLoadCnt, (unsigned int)SatCtrlTbl->JsonObjCnt);
   
   }
   else
   {
      
      memcpy(&SatCtrlTbl->Data, &TblData, sizeof(SAT_CTRL_TBL_Data_t));
      SatCtrlTbl->LastLoadCnt = ObjLoadCnt;
      CFE_EVS_SendEvent(SAT_CTRL_TBL_LOAD_EID, CFE_EVS_EventType_DEBUG, 
                        "Successfully loaded %d JSON objects",
                        (unsigned int)ObjLoadCnt);
      RetStatus = true;
      
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */

