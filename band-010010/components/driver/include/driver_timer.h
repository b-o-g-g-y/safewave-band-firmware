/*
  ******************************************************************************
  * @file    driver_timer.h
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Header file of Timer HAL module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __DRIVER_TIMER_H__
#define __DRIVER_TIMER_H__

#include <stdint.h>
#include <stdbool.h>

#include "plf.h"

/** @addtogroup Timer_Registers_Section
  * @{
  */
/* ################################ Register Section Start ################################ */

/* Timer Control Register */
typedef struct
{
    uint32_t rsv_0    : 2;
    uint32_t PRESCALE : 2;
    uint32_t rsv_1    : 2;
    uint32_t CNT_MODE : 1;
    uint32_t CNT_EN   : 1;
    uint32_t rsv_2    : 24;
}REG_Control_t;


/* -------------------------------------------------*/
/*                   Timer Register                 */
/* -------------------------------------------------*/
typedef struct 
{
    volatile uint32_t         LoadCount;        /* Offset 0x00 */
    volatile uint32_t         CurrentValue;     /* Offset 0x04 */
    volatile REG_Control_t    Control;          /* Offset 0x08 */
    volatile uint32_t         IntClear;         /* Offset 0x0C */
}struct_Timer_t;

#define Timer0    ((struct_Timer_t *)(TIMER0_BASE))
#define Timer1    ((struct_Timer_t *)(TIMER1_BASE))

/* ################################ Register Section END ################################## */
/**
  * @}
  */

/** @addtogroup Timer_Initialization_Config_Section
  * @{
  */
/* ################################ Initialization¡¢Config Section Start ################################ */

/* Wire Prescale */
typedef enum 
{
    TIMER_DIV_NONE,     /* Wire_Reade */
    TIMER_DIV_16,    /* Wire_Write */    
    TIMER_DIV_256,
}enum_Timer_DIV_t;

/* ################################ Initialization¡¢Config Section END ################################## */
/**
  * @}
  */



/* Exported functions --------------------------------------------------------*/

/* timer_init */
void timer_init(struct_Timer_t *TIMERx, uint32_t fu32_LoadCount, enum_Timer_DIV_t fe_DIV);

/* timer_int_clear */
void timer_int_clear(struct_Timer_t *TIMERx);

/* timer_start */
/* timer_stop */
void timer_start(struct_Timer_t *TIMERx);
void timer_stop(struct_Timer_t *TIMERx);

/* timer_get_CurrentCount */
uint32_t timer_get_CurrentCount(struct_Timer_t *TIMERx);

#endif
