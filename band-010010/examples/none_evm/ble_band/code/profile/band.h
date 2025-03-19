/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */

#ifndef SP_GATT_PROFILE_H
#define SP_GATT_PROFILE_H

/*
 * INCLUDES (包含头文件)
 */
#include <stdio.h>
#include <string.h>
#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"


/*
 * MACROS (宏定义)
 */

/*
 * CONSTANTS (常量定义)
 */
// Simple Profile attributes index.
enum
{
    SP_IDX_SERVICE,

    SP_IDX_APPID_DECLARATION,
    SP_IDX_APPID_VALUE,
    SP_IDX_APPID_CFG,
    SP_IDX_APPID_USER_DESCRIPTION,

    SP_IDX_SYSTEM_DECLARATION,
    SP_IDX_SYSTEM_VALUE,
    SP_IDX_SYSTEM_USER_DESCRIPTION,

    SP_IDX_BAT_DECLARATION,
    SP_IDX_BAT_VALUE,
    SP_IDX_BAT_CFG,
    SP_IDX_BAT_USER_DESCRIPTION,

    SP_IDX_NAME_DECLARATION,
    SP_IDX_NAME_VALUE,
    SP_IDX_NAME_USER_DESCRIPTION,

    SP_IDX_NB,
};

// Simple GATT Profile Service UUID
#define SP_SVC_UUID              0xFFFE

//notify "624236dc-751b-11ed-a1eb-0242ac120002" 
//#define SP_CHAR1_TX_UUID {0x02,0x00,0x12,0xac,0x42,0x02,0xeb,0xa1,0xed,0x11,0x1b,0x75,0xdc,0x36,0x42,0x62}

//马达控制//"81b2497c-8230-11ed-a1eb-0242ac120002"
#define APPID_UUID  {0x02,0x00,0x12,0xac,0x42,0x02,0xeb,0xa1,0xed,0x11,0x30,0x82,0x7c,0x49,0xb2,0x81}



//关机 "12d9cf1a-751b-11ed-a1eb-0242ac120002"
#define SYSTEM_UUID {0x02,0x00,0x12,0xac,0x42,0x02,0xeb,0xa1,0xed,0x11,0x1b,0x75,0x1a,0xcf,0xd9,0x12}

 //读电池电量"47cd799a-8233-11ed-a1eb-0242ac120002"
#define BAT_UUID {0x02,0x00,0x12,0xac,0x42,0x02,0xeb,0xa1,0xed,0x11,0x33,0x82,0x9a,0x79,0xcd,0x47}

//读写设备名字//"543eff2a-751b-11ed-a1eb-0242ac120002" 
#define NAME_UUID {0x02,0x00,0x12,0xac,0x42,0x02,0xeb,0xa1,0xed,0x11,0x1b,0x75,0x2a,0xff,0x3e,0x54}

/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */
extern const gatt_attribute_t simple_profile_att_table[];
void show_reg(uint8_t *data,uint32_t len,uint8_t dbg_on);
/*
 * LOCAL VARIABLES (本地变量)
 */


/*
 * PUBLIC FUNCTIONS (全局函数)
 */
/*********************************************************************
 * @fn      sp_gatt_add_service
 *
 * @brief   Simple Profile add GATT service function.
 *          添加GATT service到ATT的数据库里面。
 *
 * @param   None.
 *
 *
 * @return  None.
 */
void sp_gatt_add_service(void);



#endif







