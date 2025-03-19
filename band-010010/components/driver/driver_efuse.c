/*
  ******************************************************************************
  * @file    driver_efuse.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   eFuse module driver.
  *          This file provides firmware functions to manage the eFuse.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "driver_efuse.h"

/************************************************************************************
 * @fn      eFuse_read
 *
 * @brief   read eFuse data. total of 128-bit
 *
 * @param   fp_Data: read data buffer.
 */
void eFuse_read(uint32_t *fp_Data)
{ 
    /* eFuse clock enable */
    __SYSTEM_EFUSE_CLK_ENABLE();

    /* config read mode */
    EFUSE->eFuse_Ctrl = EFUSE_READ_MODE;
    /* wait config done */
    while(!(EFUSE->eFuse_Ctrl & EFUSE_CHECK_DONE));
    /* read data */
    fp_Data[0] = EFUSE->eFuse_Data0;
    fp_Data[1] = EFUSE->eFuse_Data1;
    fp_Data[2] = EFUSE->eFuse_Data2;
    fp_Data[3] = EFUSE->eFuse_Data3;
}
