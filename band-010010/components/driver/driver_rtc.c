/*
  ******************************************************************************
  * @file    driver_rtc.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   rtc module driver.
  *          This file provides firmware functions to manage the 
  *          Real Time Clock (RTC) peripheral
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "driver_rtc.h"

str_Time_t AlarmTime_A;
str_Time_t AlarmTime_B;

/*********************************************************************
 * @fn      rtc_GetCount
 *
 * @brief   Get rtc current counter value
 *
 * @param   None.
 * @return  None.
 */
void rtc_init(void)
{
    uint8_t lu8_TempValue;

    /* RTC clock enable */
    lu8_TempValue = ool_read(PMU_REG_CLK_EN);

    ool_write(PMU_REG_CLK_EN, lu8_TempValue | PMU_RTC_CLK_EN);

    /* RTC Reset disable */
    lu8_TempValue = ool_read(PMU_REG_RST_CTRL);

    ool_write(PMU_REG_RST_CTRL, lu8_TempValue & ~PMU_RTC_SFT_RST);
}

/*********************************************************************
 * @fn      rtc_AlarmConfig
 *
 * @brief   rtc alarm config
 *
 * @param   fe_Alarm:   alarm select.
            lu8_hour:   hour
            lu8_Minute: minute
            lu8_Second: second
 * @return  None.
 */
void rtc_AlarmConfig(e_alarm_t fe_Alarm, uint32_t fu32_hour, uint32_t fu32_Minute, uint32_t fu32_Second)
{
    uint32_t lu32_Second;

    lu32_Second  = fu32_hour * 3600;
    lu32_Second += fu32_Minute * 60;
    lu32_Second += fu32_Second;

    switch (fe_Alarm)
    {
        case AlARM_A: 
        {
            /* Timing backup */
            if (AlarmTime_A.FirstBackup == 0)
                AlarmTime_A.FirstBackup = lu32_Second;

            AlarmTime_A.CycleBackup = lu32_Second;
            
            /* Convert to count value */
            lu32_Second *= pmu_get_rc_clk(false);
    
            pmu_enable_isr(PMU_RTC_ALMA_INT_EN);

            ool_pd_write(PMU_REG_PD_RTC_ALARMA_CNT_0, (lu32_Second & 0xFF));
            ool_pd_write(PMU_REG_PD_RTC_ALARMA_CNT_1, (lu32_Second >>  8) & 0xFF);
            ool_pd_write(PMU_REG_PD_RTC_ALARMA_CNT_2, (lu32_Second >> 16) & 0xFF);
            ool_pd_write(PMU_REG_PD_RTC_ALARMA_CNT_3, (lu32_Second >> 24) & 0xFF);

            rtc_AlarmEnable(AlARM_A);
        }break;

        case AlARM_B: 
        {
            /* Timing backup */
            if (AlarmTime_B.FirstBackup == 0)
                AlarmTime_B.FirstBackup = lu32_Second;

            AlarmTime_B.CycleBackup = lu32_Second;
            
            /* Convert to count value */
            lu32_Second *= pmu_get_rc_clk(false);
    
            pmu_enable_isr(PMU_RTC_ALMB_INT_EN);

            ool_pd_write(PMU_REG_PD_RTC_ALARMB_CNT_0, (lu32_Second & 0xFF));
            ool_pd_write(PMU_REG_PD_RTC_ALARMB_CNT_1, (lu32_Second >>  8) & 0xFF);
            ool_pd_write(PMU_REG_PD_RTC_ALARMB_CNT_2, (lu32_Second >> 16) & 0xFF);
            ool_pd_write(PMU_REG_PD_RTC_ALARMB_CNT_3, (lu32_Second >> 24) & 0xFF);

            rtc_AlarmEnable(AlARM_B);
        }break;

        default: break; 
    }
}

/*********************************************************************
 * @fn      rtc_GetCount
 *
 * @brief   Get rtc current counter value
 *
 * @param   None.
 * @return  lu32_CountValue: rtc current counter value.
 */
uint32_t rtc_GetCount(void)
{
    uint32_t lu32_CountValue;

    lu32_CountValue  = ool_pd_read(PMU_REG_PD_RTC_UPD_CNT_0);
    lu32_CountValue |= ool_pd_read(PMU_REG_PD_RTC_UPD_CNT_1) << 8;
    lu32_CountValue |= ool_pd_read(PMU_REG_PD_RTC_UPD_CNT_2) << 16;
    lu32_CountValue |= ool_pd_read(PMU_REG_PD_RTC_UPD_CNT_3) << 24;
    
    return lu32_CountValue;
}

/*********************************************************************
 * @fn      rtc_CountUpdate
 *
 * @brief   Update RTC counter
 *
 * @param   fu32_Count: update value.
 * @return  None.
 */
void rtc_CountUpdate(uint32_t fu32_Count)
{
    uint8_t lu8_TempValue;
    
    ool_pd_write(PMU_REG_PD_RTC_UPD_CNT_0, (fu32_Count & 0xFF));
    ool_pd_write(PMU_REG_PD_RTC_UPD_CNT_1, (fu32_Count >>  8) & 0xFF);
    ool_pd_write(PMU_REG_PD_RTC_UPD_CNT_2, (fu32_Count >> 16) & 0xFF);
    ool_pd_write(PMU_REG_PD_RTC_UPD_CNT_3, (fu32_Count >> 24) & 0xFF);

    lu8_TempValue = ool_pd_read(PMU_REG_PD_RTC_CTRL);

    ool_pd_write(PMU_REG_PD_RTC_CTRL, lu8_TempValue |  PMU_RTC_UPD_EN);
    ool_pd_write(PMU_REG_PD_RTC_CTRL, lu8_TempValue & ~PMU_RTC_UPD_EN);

    AlarmTime_A.FirstBackup = 0;
    AlarmTime_B.FirstBackup = 0;
}

/*********************************************************************
 * @fn      rtc_ClockEnable
 *
 * @brief   RTC clock enable
 */
void rtc_ClockEnable(void)
{
    uint8_t lu8_TempValue;
    
    /* RTC clock enable */
    lu8_TempValue = ool_read(PMU_REG_CLK_EN);
    ool_write(PMU_REG_CLK_EN, lu8_TempValue | PMU_RTC_CLK_EN);
}

/*********************************************************************
 * @fn      rtc_ClockDisable
 *
 * @brief   RTC clock disable
 */
void rtc_ClockDisable(void)
{
    uint8_t lu8_TempValue;
    
    /* RTC clock enable */
    lu8_TempValue = ool_read(PMU_REG_CLK_EN);
    ool_write(PMU_REG_CLK_EN, lu8_TempValue & ~PMU_RTC_CLK_EN);
}

/*********************************************************************
 * @fn      rtc_ResetEnable
 *
 * @brief   RTC reset enable
 */
void rtc_ResetEnable(void)
{
    uint8_t lu8_TempValue;

    /* RTC Reset disable */
    lu8_TempValue = ool_read(PMU_REG_RST_CTRL);
    ool_write(PMU_REG_RST_CTRL, lu8_TempValue | PMU_RTC_SFT_RST);
}

/*********************************************************************
 * @fn      rtc_ResetDisable
 *
 * @brief   RTC reset disable
 */
void rtc_ResetDisable(void)
{
    uint8_t lu8_TempValue;
    
    /* RTC Reset disable */
    lu8_TempValue = ool_read(PMU_REG_RST_CTRL);
    ool_write(PMU_REG_RST_CTRL, lu8_TempValue & ~PMU_RTC_SFT_RST);
}

/*********************************************************************
 * @fn      rtc_AlarmEnable
 *
 * @brief   Alarm Enable
 */
void rtc_AlarmEnable(e_alarm_t fe_Alarm)
{
    uint8_t lu8_TempValue;

    /* RTC Alarm Enable */
    lu8_TempValue = ool_pd_read(PMU_REG_PD_RTC_CTRL);
    ool_pd_write(PMU_REG_PD_RTC_CTRL, lu8_TempValue | fe_Alarm);
} 

/*********************************************************************
 * @fn      rtc_ALarmDisable
 *
 * @brief   Alarm Disable
 */
void rtc_AlarmDisable(e_alarm_t fe_Alarm)
{
    uint8_t lu8_TempValue;

    /* RTC Alarm Disable */
    lu8_TempValue = ool_pd_read(PMU_REG_PD_RTC_CTRL);
    ool_pd_write(PMU_REG_PD_RTC_CTRL, lu8_TempValue & ~fe_Alarm);
}

/*********************************************************************
 * @fn      rtc_AlarmRead
 *
 * @brief   read rtc alarm config value
 *
 * @param   None.
 * @return  lu32_ConfigValue: alarm config value.
 */
uint32_t rtc_AlarmRead(e_alarm_t fe_Alarm)
{
    uint32_t lu32_ConfigValue;

    switch (fe_Alarm)
    {
        case AlARM_A: 
        {
            lu32_ConfigValue  = ool_pd_read(PMU_REG_PD_RTC_ALARMA_CNT_0);
            lu32_ConfigValue |= ool_pd_read(PMU_REG_PD_RTC_ALARMA_CNT_1) << 8;
            lu32_ConfigValue |= ool_pd_read(PMU_REG_PD_RTC_ALARMA_CNT_2) << 16;
            lu32_ConfigValue |= ool_pd_read(PMU_REG_PD_RTC_ALARMA_CNT_3) << 24;
        }break;

        case AlARM_B: 
        {
            lu32_ConfigValue  = ool_pd_read(PMU_REG_PD_RTC_ALARMB_CNT_0);
            lu32_ConfigValue |= ool_pd_read(PMU_REG_PD_RTC_ALARMB_CNT_1) << 8;
            lu32_ConfigValue |= ool_pd_read(PMU_REG_PD_RTC_ALARMB_CNT_2) << 16;
            lu32_ConfigValue |= ool_pd_read(PMU_REG_PD_RTC_ALARMB_CNT_3) << 24;
        }break;

        default: break; 
    }
    
    return lu32_ConfigValue;
}

/*********************************************************************
 * @fn      rtc_AlarmSet
 *
 * @brief   Set rtc alarm config value
 *
 * @param   None.
 * @return  lu32_ConfigValue: alarm config value.
 */
void rtc_AlarmSet(e_alarm_t fe_Alarm, uint32_t fu32_ConfigValue)
{
    switch (fe_Alarm)
    {
        case AlARM_A: 
        {
            ool_pd_write(PMU_REG_PD_RTC_ALARMA_CNT_0, (fu32_ConfigValue & 0xFF));
            ool_pd_write(PMU_REG_PD_RTC_ALARMA_CNT_1, (fu32_ConfigValue >>  8) & 0xFF);
            ool_pd_write(PMU_REG_PD_RTC_ALARMA_CNT_2, (fu32_ConfigValue >> 16) & 0xFF);
            ool_pd_write(PMU_REG_PD_RTC_ALARMA_CNT_3, (fu32_ConfigValue >> 24) & 0xFF);
        }break;

        case AlARM_B: 
        {
            ool_pd_write(PMU_REG_PD_RTC_ALARMB_CNT_0, (fu32_ConfigValue & 0xFF));
            ool_pd_write(PMU_REG_PD_RTC_ALARMB_CNT_1, (fu32_ConfigValue >>  8) & 0xFF);
            ool_pd_write(PMU_REG_PD_RTC_ALARMB_CNT_2, (fu32_ConfigValue >> 16) & 0xFF);
            ool_pd_write(PMU_REG_PD_RTC_ALARMB_CNT_3, (fu32_ConfigValue >> 24) & 0xFF);
        }break;
    }
}

/*********************************************************************
 * @fn      rtc_AlarmHandler
 *
 * @brief   Alarm interrupt handler 
 */
__WEAK void rtc_AlarmHandler(void)
{
    if (pmu_get_isr_state() & PMU_RTC_ALMA_INT_STATUS) 
    {
        pmu_clear_isr_state(PMU_RTC_ALMA_INT_CLR);

        /* Alarm cycle */
        AlarmTime_A.CycleBackup += AlarmTime_A.FirstBackup;

        rtc_AlarmConfig(AlARM_A, 0, 0, AlarmTime_A.CycleBackup);
    }
    else if (pmu_get_isr_state() & PMU_RTC_ALMB_INT_STATUS) 
    {
        pmu_clear_isr_state(PMU_RTC_ALMB_INT_CLR);

        /* Alarm cycle */
        AlarmTime_B.CycleBackup += AlarmTime_B.FirstBackup;

        rtc_AlarmConfig(AlARM_B, 0, 0, AlarmTime_B.CycleBackup);
    }
}
