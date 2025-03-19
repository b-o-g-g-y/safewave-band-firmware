/*
  ******************************************************************************
  * @file    driver_timer.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Timer module driver.
  *          This file provides firmware functions to manage the Timer peripheral
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "driver_timer.h"

/************************************************************************************
 * @fn      timer_init
 *
 * @brief   timer initialize.
 *
 * @param   TIMERx: Timer handle.
 *          fu32_LoadCount: Timer Load Count.
 */
void timer_init(struct_Timer_t *TIMERx, uint32_t fu32_LoadCount, enum_Timer_DIV_t fe_DIV)
{
    TIMERx->LoadCount = fu32_LoadCount;

    TIMERx->Control.PRESCALE = fe_DIV;
    TIMERx->Control.CNT_MODE = 1;
}

/************************************************************************************
 * @fn      timer_int_clear
 *
 * @brief   timer interrupt status clear.
 *
 * @param   TIMERx: Timer handle.
 */
void timer_int_clear(struct_Timer_t *TIMERx)
{
    TIMERx->IntClear = 1;
}

/************************************************************************************
 * @fn      timer_start
 *
 * @brief   Timer start count.
 *
 * @param   TIMERx: Timer handle.
 */
void timer_start(struct_Timer_t *TIMERx)
{
    TIMERx->Control.CNT_EN = 1;
}

/************************************************************************************
 * @fn      timer_stop
 *
 * @brief   Timer stop count.
 *
 * @param   TIMERx: Timer handle.
 */
void timer_stop(struct_Timer_t *TIMERx)
{
    TIMERx->Control.CNT_EN = 0;
}

/************************************************************************************
 * @fn      timer_get_CurrentCount
 *
 * @brief   get Timer current count.
 *
 * @param   TIMERx: Timer handle.
 */
uint32_t timer_get_CurrentCount(struct_Timer_t *TIMERx)
{
    return TIMERx->CurrentValue;
}
