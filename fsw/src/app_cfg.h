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
**    Define application configurations for the Table Sat application
**
**  Notes:
**    1. These macros can only be built with the application and can't
**       have a platform scope because the same app_cfg.h filename is used for
**       all applications following the object-based application design.
**    2. This reference doesn't directly apply to this file but discusses
**       Raspian interrupts/events which applies to several of the objects
**       in this app_cfg
**       https://www.i-programmer.info/programming/hardware/14114-raspberry-pi-iot-in-c-events-a-interrupts.html
**
*/

#ifndef _app_cfg_
#define _app_cfg_

/*
** Includes
*/

#include "tbl_sat_eds_typedefs.h"

#include "tbl_sat_platform_cfg.h"
#include "app_c_fw.h"


/******************************************************************************
** Application Macros
*/

/*
** Versions:
**
** 1.0 - Initial release
*/

#define  TBL_SAT_MAJOR_VER   0
#define  TBL_SAT_MINOR_VER   9


/******************************************************************************
** JSON initialization declarations
**
** GPIO Pin assignments must match the tutorial's wiring diagraam
** Phycical  BCM   
**   Pin     ID   Role
**   
**    3       2   I2C Data
**    5       3   I2C Clock
**   12      18   FAN A Pulse Width Modulation Control
**   18      24   FAN A Tachometer (pulses per revolution) 
**   35      19   FAN B Pulse Width Modulation control
**   37      26   FAN B Tachometer (pulses per revolution) 
**
*/

#define CFG_APP_CFE_NAME     APP_CFE_NAME
#define CFG_APP_PERF_ID      APP_PERF_ID

#define CFG_CMD_PIPE_NAME    APP_CMD_PIPE_NAME
#define CFG_CMD_PIPE_DEPTH   APP_CMD_PIPE_DEPTH

#define CFG_TBL_SAT_CMD_TOPICID         TBL_SAT_CMD_TOPICID
#define CFG_BC_SCH_1_HZ_TOPICID         BC_SCH_1_HZ_TOPICID
#define CFG_TBL_SAT_STATUS_TLM_TOPICID  TBL_SAT_STATUS_TLM_TOPICID
#define CFG_MQTT_GW_TOPIC_4_TLM_TOPICID MQTT_GW_TOPIC_4_TLM_TOPICID

#define CFG_CHILD_NAME       CHILD_NAME
#define CFG_CHILD_PERF_ID    CHILD_PERF_ID
#define CFG_CHILD_STACK_SIZE CHILD_STACK_SIZE
#define CFG_CHILD_PRIORITY   CHILD_PRIORITY

#define CFG_SAT_CTRL_MQTT_PIPE_NAME   SAT_CTRL_MQTT_PIPE_NAME
#define CFG_SAT_CTRL_MQTT_PIPE_DEPTH  SAT_CTRL_MQTT_PIPE_DEPTH
#define CFG_SAT_CTRL_PERIOD           SAT_CTRL_PERIOD
#define CFG_SAT_CTRL_TBL_DEF          SAT_CTRL_TBL_DEF

#define CFG_I2C_SDA_BCM_ID    I2C_SDA_BCM_ID
#define CFG_I2C_SCL_BCM_ID    I2C_SCL_BCM_ID
#define CFG_FAN_A_PWM_BCM_ID  FAN_A_PWM_BCM_ID
#define CFG_FAN_A_TACH_BCM_ID FAN_A_TACH_BCM_ID
#define CFG_FAN_B_PWM_BCM_ID  FAN_B_PWM_BCM_ID
#define CFG_FAN_B_TACH_BCM_ID FAN_B_TACH_BCM_ID
      

#define APP_CONFIG(XX) \
   XX(APP_CFE_NAME,char*) \
   XX(APP_PERF_ID,uint32) \
   XX(APP_CMD_PIPE_NAME,char*) \
   XX(APP_CMD_PIPE_DEPTH,uint32) \
   XX(TBL_SAT_CMD_TOPICID,uint32) \
   XX(BC_SCH_1_HZ_TOPICID,uint32) \
   XX(TBL_SAT_STATUS_TLM_TOPICID,uint32) \
   XX(MQTT_GW_TOPIC_4_TLM_TOPICID,uint32) \
   XX(CHILD_NAME,char*) \
   XX(CHILD_PERF_ID,uint32) \
   XX(CHILD_STACK_SIZE,uint32) \
   XX(CHILD_PRIORITY,uint32) \
   XX(SAT_CTRL_MQTT_PIPE_NAME,char*) \
   XX(SAT_CTRL_MQTT_PIPE_DEPTH,uint32) \
   XX(SAT_CTRL_PERIOD,uint32) \
   XX(SAT_CTRL_TBL_DEF,char*) \
   XX(I2C_SDA_BCM_ID,uint32) \
   XX(I2C_SCL_BCM_ID,uint32) \
   XX(FAN_A_PWM_BCM_ID,uint32) \
   XX(FAN_A_TACH_BCM_ID,uint32) \
   XX(FAN_B_PWM_BCM_ID,uint32) \
   XX(FAN_B_TACH_BCM_ID,uint32) \

DECLARE_ENUM(Config,APP_CONFIG)


/******************************************************************************
** Event Macros
**
** Define the base event message IDs used by each object/component used by the
** application. There are no automated checks to ensure an ID range is not
** exceeded so it is the developer's responsibility to verify the ranges. 
*/

#define TBL_SAT_BASE_EID      (APP_C_FW_APP_BASE_EID +  0)
#define SAT_CTRL_BASE_EID     (APP_C_FW_APP_BASE_EID + 10)
#define SAT_CTRL_TBL_BASE_EID (APP_C_FW_APP_BASE_EID + 20)
#define FAN_BASE_EID          (APP_C_FW_APP_BASE_EID + 30)

/******************************************************************************
** SAT_CTRL Table Macros
*/

#define SAT_CTRL_TBL_JSON_FILE_MAX_CHAR  4090 
#define SAT_CTRL_TBL_NAME                "Control Parameters" 

#endif /* _app_cfg_ */
