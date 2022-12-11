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
**  References:
**    1. OpenSatKit Object-based Application Developer's Guide.
**    2. cFS Application Developer's Guide.
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
   
   { &TblData.Test.Steps,       sizeof(TblData.Test.Steps),       false,    JSONNumber, false,  { "test-steps",          (sizeof("test-steps")-1)}          },
   { &TblData.Test.TimeInStep,  sizeof(TblData.Test.TimeInStep),  false,    JSONNumber, false,  { "test-time-in-step",   (sizeof("test-time-in-step")-1)}   },

};


/******************************************************************************
** Function: SAT_CTRL_TBL_Constructor
**
** Notes:
**    1. This must be called prior to any other functions
**
*/
void SAT_CTRL_TBL_Constructor(SAT_CTRL_TBL_Class_t *SatCtrlTblPtr, const char *AppName)
{

   SatCtrlTbl = SatCtrlTblPtr;

   CFE_PSP_MemSet(SatCtrlTbl, 0, sizeof(SAT_CTRL_TBL_Class_t));
 
   SatCtrlTbl->AppName = AppName;
   
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
bool SAT_CTRL_TBL_DumpCmd(TBLMGR_Tbl_t *Tbl, uint8 DumpType, const char *Filename)
{

   bool       RetStatus = false;
   int32      SysStatus;
   osal_id_t  FileHandle;
   os_err_name_t OsErrStr;
   char DumpRecord[256];
   char SysTimeStr[128];

   
   SysStatus = OS_OpenCreate(&FileHandle, Filename, OS_FILE_FLAG_CREATE, OS_READ_WRITE);
   
   if (SysStatus == OS_SUCCESS)
   {
 
      sprintf(DumpRecord,"{\n   \"app-name\": \"%s\",\n   \"tbl-name\": \"Table Sat Control\",\n", SatCtrlTbl->AppName);
      OS_write(FileHandle, DumpRecord, strlen(DumpRecord));

      CFE_TIME_Print(SysTimeStr, CFE_TIME_GetTime());
      sprintf(DumpRecord,"   \"description\": \"Table dumped at %s\",\n",SysTimeStr);
      OS_write(FileHandle, DumpRecord, strlen(DumpRecord));

      sprintf(DumpRecord,"   \"steps\": %d,\n", SatCtrlTbl->Data.Test.Steps);
      OS_write(FileHandle, DumpRecord, strlen(DumpRecord));

      sprintf(DumpRecord,"   \"time-in-step\": %d,\n", SatCtrlTbl->Data.Test.TimeInStep);
      OS_write(FileHandle, DumpRecord, strlen(DumpRecord));
       
      sprintf(DumpRecord,"   }\n}\n");
      OS_write(FileHandle, DumpRecord, strlen(DumpRecord));

      OS_close(FileHandle);

      CFE_EVS_SendEvent(SAT_CTRL_TBL_DUMP_EID, CFE_EVS_EventType_DEBUG,
                        "Successfully created dump file %s", Filename);
                        
      RetStatus = true;

   } /* End if file create */
   else
   {
      OS_GetErrorName(SysStatus, &OsErrStr);
      CFE_EVS_SendEvent(SAT_CTRL_TBL_DUMP_EID, CFE_EVS_EventType_ERROR,
                        "Error creating dump file '%s', status=%s",
                        Filename, OsErrStr);
   
   } /* End if file create error */

   return RetStatus;
   
} /* End of SAT_CTRL_TBL_DumpCmd() */


/******************************************************************************
** Function: SAT_CTRL_TBL_LoadCmd
**
** Notes:
**  1. Function signature must match TBLMGR_LoadTblFuncPtr_t.
**  2. This could migrate into table manager but I think I'll keep it here so
**     user's can add table processing code if needed.
*/
bool SAT_CTRL_TBL_LoadCmd(TBLMGR_Tbl_t *Tbl, uint8 LoadType, const char *Filename)
{

   bool  RetStatus = false;

   if (CJSON_ProcessFile(Filename, SatCtrlTbl->JsonBuf, SAT_CTRL_TBL_JSON_FILE_MAX_CHAR, LoadJsonData))
   {
      SatCtrlTbl->Loaded = true;
      SatCtrlTbl->LastLoadStatus = TBLMGR_STATUS_VALID;
      RetStatus = true;   
   }
   else
   {
      SatCtrlTbl->LastLoadStatus = TBLMGR_STATUS_INVALID;
   }

   return RetStatus;
   
} /* End SAT_CTRL_TBL_LoadCmd() */


/******************************************************************************
** Function: SAT_CTRL_TBL_ResetStatus
**
*/
void SAT_CTRL_TBL_ResetStatus(void)
{

   SatCtrlTbl->LastLoadStatus = TBLMGR_STATUS_UNDEF;
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
   
      memcpy(&SatCtrlTbl->Data,&TblData, sizeof(SAT_CTRL_TBL_Data_t));
      SatCtrlTbl->LastLoadCnt = ObjLoadCnt;
      CFE_EVS_SendEvent(SAT_CTRL_TBL_LOAD_EID, CFE_EVS_EventType_DEBUG, 
                        "Successfully loaded %d JSON objects",
                        (unsigned int)ObjLoadCnt);
      RetStatus = true;
      
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */

