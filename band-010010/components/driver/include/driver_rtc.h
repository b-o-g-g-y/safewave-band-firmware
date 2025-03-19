/*
  ******************************************************************************
  * @file    driver_rtc.h
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Header file of rtc HAL module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __DRIVER_RTC_H__
#define __DRIVER_RTC_H__

#include "stdint.h"
#include "stdbool.h"

#include "driver_pmu.h"

#include "sys_utils.h"

typedef struct 
{
    uint32_t FirstBackup;
    uint32_t CycleBackup;
}str_Time_t;

typedef enum
{
    AlARM_A = 0x08,
    AlARM_B = 0x04,
}e_alarm_t;

/* Exported functions --------------------------------------------------------*/

/* rtc_init */
void rtc_init(void);

/* rtc_AlarmConfig */
void rtc_AlarmConfig(e_alarm_t fe_Alarm, uint32_t fu32_hour, uint32_t fu32_Minute, uint32_t fu32_Second);

/* rtc_GetCount */
uint32_t rtc_GetCount(void);

/* rtc_CountUpdate */
void rtc_CountUpdate(uint32_t fu32_Count);

/* rtc_ClockEnable */
/* rtc_ClockDisable */
void rtc_ClockEnable(void);
void rtc_ClockDisable(void);

/* rtc_ResetEnable */
/* rtc_ResetDisable */
void rtc_ResetEnable(void);
void rtc_ResetDisable(void);

/* rtc_AlarmEnable */
/* rtc_AlarmDisable */
void rtc_AlarmEnable(e_alarm_t fe_Alarm);
void rtc_AlarmDisable(e_alarm_t fe_Alarm);

/* rtc_AlarmHandler */
void rtc_AlarmHandler(void);

#endif
