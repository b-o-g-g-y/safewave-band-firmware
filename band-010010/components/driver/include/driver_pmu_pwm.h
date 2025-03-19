/*
  ******************************************************************************
  * @file    driver_pmu_pwm.h
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Header file of PMU PWM HAL module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __DRIVER_PMU_PWM_H__
#define __DRIVER_PMU_PWM_H__

#include "stdint.h"
#include "stdbool.h"

#include "driver_pmu.h"

typedef enum
{
    PWM_0,
    PWM_1,
    PWM_2,
    PWM_3,
}e_PWM_t;


enum pwm_invert_t
{
    PWM_INVERT_DISABLE,
    PWM_INVERT_ENABLE,
};

/**
  * @brief  PWM config parameter
  */
typedef struct 
{
    enum system_port_t     Port;         /* This parameter can be a value of @ref system_port_t */
    enum system_port_bit_t Pin;          /* This parameter can be a value of @ref system_port_bit_t */
    uint16_t               Period;       /* This parameter can be a a 16-bit Size */
    uint16_t               HighCount;    /* This parameter can be a a 16-bit Size */
    bool                   Invert;       /* This parameter can be a value of @ref pwm_invert_t */
}str_PWMParam_t;


/* Exported functions --------------------------------------------------------*/

/* pmu_pwm_config */
void pmu_pwm_config(e_PWM_t PWM, str_PWMParam_t PWMParam);

/* pmu_pwm_single_start */
void pmu_pwm_single_start(e_PWM_t PWM);

/* pmu_pwm_cycle_start */
void pmu_pwm_cycle_start(e_PWM_t PWM);

/* pmu_pwm_stop */
void pmu_pwm_stop(e_PWM_t PWM);


#endif

