/*
  ******************************************************************************
  * @file    driver_wdt.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   wdt module driver.
  *          This file provides firmware functions to manage the 
  *          Watchdog Timer (WDT) peripheral
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "driver_wdt.h"

/*********************************************************************
 * @fn      wdt_init
 *
 * @brief   Refresh the Watchdog. 
 */
void wdt_init(wdt_Init_t Init)
{
    uint8_t lu8_TempValue;

    /* WdtCount¡¢Timeout */
    wdt_Set_Count(Init.WdtCount);
    wdt_Set_Timeout(Init.Timeout);

    /* PMU_REG_RST_CTRL */
    lu8_TempValue = ool_read(PMU_REG_RST_CTRL);

    ool_write(PMU_REG_RST_CTRL, lu8_TempValue & ~PMU_WTD_RST_DIS);

    /* PMU_REG_WTD_CTRL¡¢IRQ_EN */
    if (Init.IRQ_Enable) 
    {
        pmu_enable_isr(PMU_WTD_INT_EN);
    }

    lu8_TempValue  = Init.IRQ_Enable;
    lu8_TempValue |= PMU_WTD_CTRL_EN;

    ool_write(PMU_REG_WTD_CTRL, lu8_TempValue);

    lu8_TempValue = ool_read(PMU_REG_PORTE_XOR_EN);

    ool_write(PMU_REG_PORTE_XOR_EN, lu8_TempValue | 0x10);
}

/*********************************************************************
 * @fn      wdt_IRQ_handler
 *
 * @brief   WDT Interrupt handler. 
 */
__WEAK void wdt_IRQ_handler(void)
{
    if (pmu_get_isr_state() & PMU_WTD_INT_STATUS) 
    {
        pmu_clear_isr_state(PMU_WTD_INT_CLR);

        wdt_Refresh();
    }
}

/*********************************************************************
 * @fn      wdt_Refresh
 *
 * @brief   Refresh the Watchdog. 
 */
void wdt_Refresh(void)
{
    uint8_t lu8_TempValue;

    lu8_TempValue = ool_read(PMU_REG_WTD_CTRL);

    ool_write(PMU_REG_WTD_CTRL, lu8_TempValue | PMU_WDT_CTRL_CLR);
    co_delay_10us(5);
    ool_write(PMU_REG_WTD_CTRL, lu8_TempValue & ~PMU_WDT_CTRL_CLR);
}

/*********************************************************************
 * @fn      wdt_Interrupt_Enable
 *
 * @brief   Watchdog Interrupt Enable.
 */
void wdt_Interrupt_Enable(void)
{
    uint8_t lu8_TempValue;
    
    lu8_TempValue = ool_read(PMU_REG_WTD_CTRL);
    ool_write(PMU_REG_WTD_CTRL, lu8_TempValue | PMU_WDT_CTRL_IRQ_EN);
}

/*********************************************************************
 * @fn      wdt_Interrupt_Disable
 *
 * @brief   Watchdog Interrupt Disable.
 */
void wdt_Interrupt_Disable(void)
{
    uint8_t lu8_TempValue;
    
    lu8_TempValue = ool_read(PMU_REG_WTD_CTRL);
    ool_write(PMU_REG_WTD_CTRL, lu8_TempValue & ~PMU_WDT_CTRL_IRQ_EN);
}

/*********************************************************************
 * @fn      wdt_Enable
 *
 * @brief   Watchdog Enable.
 */
void wdt_Enable(void)
{
    uint8_t lu8_TempValue;

    lu8_TempValue = ool_read(PMU_REG_WTD_CTRL);
    ool_write(PMU_REG_WTD_CTRL, lu8_TempValue | PMU_WTD_CTRL_EN);
}

/*********************************************************************
 * @fn      wdt_Disable
 *
 * @brief   Watchdog Disable.
 */
void wdt_Disable(void)
{
    uint8_t lu8_TempValue;

    lu8_TempValue = ool_read(PMU_REG_WTD_CTRL);
    ool_write(PMU_REG_WTD_CTRL, lu8_TempValue & ~PMU_WTD_CTRL_EN);
}

/*********************************************************************
 * @fn      wdt_Set_Count
 *
 * @brief   Set watchdog count. 
 *
 * @param   WdtCount: watchdog count (0x000000 ~ 0xFFFFFF).
 * @return  None.
 */
void wdt_Set_Count(uint32_t WdtCount)
{
    WdtCount &= 0xFFFFFF;
    
    ool_write(PMU_REG_WTD_CNT_0,  WdtCount        & 0xFF);    // Address 0x27. LSB
    ool_write(PMU_REG_WTD_CNT_1, (WdtCount >> 8)  & 0xFF);
    ool_write(PMU_REG_WTD_CNT_2, (WdtCount >> 16) & 0xFF);    // Address 0x29. MSB
}

/*********************************************************************
 * @fn      wdt_Set_Timeout
 *
 * @brief   Set watchdog Timeout. Timeout system reset
 *          Attention: Timeout Set to 0 the system will not reset.
 *
 * @param   WdtCount: watchdog Timeout (0x00 ~ 0xFF).
 * @return  None.
 */
void wdt_Set_Timeout(uint8_t WdtTimeout)
{
    ool_write(PMU_REG_WTD_TOUT_LEN, WdtTimeout);
}






