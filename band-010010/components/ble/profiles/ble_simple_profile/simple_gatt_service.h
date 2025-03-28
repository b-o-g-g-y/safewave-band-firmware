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

    SP_IDX_CHAR1_DECLARATION,
    SP_IDX_CHAR1_VALUE,
    SP_IDX_CHAR1_CFG,
    SP_IDX_CHAR1_USER_DESCRIPTION,

    SP_IDX_CHAR2_DECLARATION,
    SP_IDX_CHAR2_VALUE,
    SP_IDX_CHAR2_USER_DESCRIPTION,

    SP_IDX_CHAR3_DECLARATION,
    SP_IDX_CHAR3_VALUE,
    SP_IDX_CHAR3_USER_DESCRIPTION,

    SP_IDX_CHAR4_DECLARATION,
    SP_IDX_CHAR4_VALUE,
    SP_IDX_CHAR4_CFG,
    SP_IDX_CHAR4_USER_DESCRIPTION,

    SP_IDX_CHAR5_DECLARATION,
    SP_IDX_CHAR5_VALUE,
    SP_IDX_CHAR5_USER_DESCRIPTION,

    SP_IDX_NB,
};

// Simple GATT Profile Service UUID
#define SP_SVC_UUID              0xFFF0

#define SP_CHAR1_TX_UUID            {0xb8, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}
#define SP_CHAR2_RX_UUID            {0xba, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}
#define SP_CHAR3_UUID            0xFFF3
#define SP_CHAR4_UUID            0xFFF4
#define SP_CHAR5_UUID            0xFFF5

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







