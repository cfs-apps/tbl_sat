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
**    Manage the Table Sat control parameter table
**
**  Notes:
**    1. Use the Singleton design pattern. A pointer to the table object
**       is passed to the constructor and saved for all other operations.
**       This is a table-specific file so it doesn't need to be re-entrant.
**
**  References:
**    1. OpenSatKit Object-based Application Developer's Guide
**    2. cFS Application Developer's Guide
**
*/

#ifndef _sat_ctrl_tbl_
#define _sat_ctrl_tbl_

/*
** Includes
*/

#include "app_cfg.h"

/***********************/
/** Macro Definitions **/
/***********************/

/*
** Event Message IDs
*/

#define SAT_CTRL_TBL_DUMP_EID  (SAT_CTRL_TBL_BASE_EID + 0)
#define SAT_CTRL_TBL_LOAD_EID  (SAT_CTRL_TBL_BASE_EID + 1)


/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Table - Local table copy used for table loads
** 
*/

typedef struct
{

   uint16  Steps;
   uint16  TimeInStep;

} SAT_CTRL_TBL_Test_t;


typedef struct
{

   SAT_CTRL_TBL_Test_t Test;
   
} SAT_CTRL_TBL_Data_t;


/******************************************************************************
** Class
*/

typedef struct
{

   /*
   ** Table Data
   */
   
   SAT_CTRL_TBL_Data_t     Data;
   
   /*
   ** Standard CJSON table data
   */
   
   const char*  AppName;
   bool         Loaded;   /* Has entire table been loaded? */
   uint8        LastLoadStatus;
   uint16       LastLoadCnt;
   
   size_t       JsonObjCnt;
   char         JsonBuf[SAT_CTRL_TBL_JSON_FILE_MAX_CHAR];   
   size_t       JsonFileLen;
   
} SAT_CTRL_TBL_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: SAT_CTRL_TBL_Constructor
**
** Initialize the Control Table table object.
**
** Notes:
**   1. The table values are not populated. This is done when the table is 
**      registered with the table manager.
**
*/
void SAT_CTRL_TBL_Constructor(SAT_CTRL_TBL_Class_t *TblObj, const char *AppName);


/******************************************************************************
** Function: SAT_CTRL_TBL_DumpCmd
**
** Command to write the table data from memory to a JSON file.
**
** Notes:
**  1. Function signature must match TBLMGR_DumpTblFuncPtr_t.
**  2. Can assume valid table file name because this is a callback from 
**     the app framework table manager.
**
*/
bool SAT_CTRL_TBL_DumpCmd(TBLMGR_Tbl_t *Tbl, uint8 DumpType, const char *Filename);


/******************************************************************************
** Function: SAT_CTRL_TBL_LoadCmd
**
** Command to copy the table data from a JSON file to memory.
**
** Notes:
**  1. Function signature must match TBLMGR_LoadTblFuncPtr_t.
**  2. Can assume valid table file name because this is a callback from 
**     the app framework table manager.
**
*/
bool SAT_CTRL_TBL_LoadCmd(TBLMGR_Tbl_t *Tbl, uint8 LoadType, const char *Filename);


/******************************************************************************
** Function: SAT_CTRL_TBL_ResetStatus
**
** Reset counters and status flags to a known reset state.  The behavior of
** the table manager should not be impacted. The intent is to clear counters
** and flags to a known default state for telemetry.
**
*/
void SAT_CTRL_TBL_ResetStatus(void);


#endif /* _histogram_tbl_ */

