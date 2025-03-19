/*
  ******************************************************************************
  * @file    driver_pmu_pwm.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   PWM module driver.
  *          This file provides firmware functions to manage the 
  *          PMU Pulse-Width Modulation (PWM) peripheral
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "driver_pmu_pwm.h"

/*********************************************************************
 * @fn      pmu_pwm_config
 *
 * @brief   pwm start output.(single mode)
 * 
 *          PWM_0 can use PA0 and PA4  |  PWM_1 can use  PA1 and PA5  
 *                        PB0 and PB4  |                 PB1 and PA5  
 *                        PC0 and PC4  |                 PC1 and PA5  
 *                        PD0 and PD4  |                 PD1 and PA5  
 *          PWM_2 can use PA2 and PA6  |  PWM_3 can use  PA3 and PA7  
 *                        PB2 and PB6  |                 PB3 and PA7  
 *                        PC2 and PC6  |                 PC3 and PA7  
 *                        PD2 and PD6  |                 PD3 and PA7  
 * 
 *            period = str_PWMParam_t.Period + 1
 *    HIGH hold time = str_PWMParam_t.Period - str_PWMParam_t.HighCount
 *  
 * @param   PWM: select PWM.
 *          PWMParam: config parameter.
 * @return  None.
 */
void pmu_pwm_config(e_PWM_t PWM, str_PWMParam_t PWMParam)
{
    uint8_t lu8_PWM_CTRL_REG   = PMU_REG_PD_PTC0_CTRL;
    uint8_t lu8_PWM_LRC_REG    = PMU_REG_PD_PTC0_LRC_L;
    uint8_t lu8_PWM_HRC_REG    = PMU_REG_PD_PTC0_HRC_L;
    uint8_t lu8_PORT_IOMUX_REG = PMU_REG_PD_PORTA_MUX;
    
    lu8_PWM_CTRL_REG += PWM;
    lu8_PWM_LRC_REG  += PWM * 4;
    lu8_PWM_HRC_REG  += PWM * 4;

    /* PWM clock enable */
    if (!(ool_read(PMU_REG_CLK_EN) & PMU_PWM_CLK_EN))
    {
        ool_write(PMU_REG_CLK_EN, ool_read(PMU_REG_CLK_EN) | PMU_PWM_CLK_EN);
    }
    
    /* PWM Reset disable */
    if (ool_read(PMU_REG_RST_CTRL) & PMU_PWM_SFT_RST)
    {
        ool_write(PMU_REG_RST_CTRL, ool_read(PMU_REG_RST_CTRL) & ~PMU_PWM_CLK_EN);
    }

    /* Invert */
    if (PWMParam.Invert) 
    {
        ool_pd_write(lu8_PWM_CTRL_REG, ool_pd_read(lu8_PWM_CTRL_REG) | PMU_PTC_CTRL_PTC_INV);
    }
    else 
    {
        ool_pd_write(lu8_PWM_CTRL_REG, ool_pd_read(lu8_PWM_CTRL_REG) & ~PMU_PTC_CTRL_PTC_INV);
    }

    /* PWM Period */
    ool_pd_write(lu8_PWM_LRC_REG, PWMParam.Period & 0xFF);
    ool_pd_write(lu8_PWM_LRC_REG + 1, (PWMParam.Period >> 8) & 0xFF);

    /* PWM HIGH hold time */
    ool_pd_write(lu8_PWM_HRC_REG, PWMParam.HighCount & 0xFF);
    ool_pd_write(lu8_PWM_HRC_REG + 1, (PWMParam.HighCount >> 8) & 0xFF);

    /* Port Pin config */
    lu8_PORT_IOMUX_REG += PWMParam.Port * 2;
    
    pmu_set_pin_to_PMU(PWMParam.Port, 1 << PWMParam.Pin);
    
    if (PWMParam.Pin < 4) 
    {
        /* Clear */
        ool_pd_write(lu8_PORT_IOMUX_REG, ool_pd_read(lu8_PORT_IOMUX_REG) & ~(3 << PWMParam.Pin * 2));  
        /* Set */  
        ool_pd_write(lu8_PORT_IOMUX_REG, ool_pd_read(lu8_PORT_IOMUX_REG) | (1 << PWMParam.Pin * 2));    
    }
    else 
    {
        /* Clear */
        ool_pd_write(lu8_PORT_IOMUX_REG + 1, ool_pd_read(lu8_PORT_IOMUX_REG + 1) & ~(3 << (PWMParam.Pin-4) * 2));  
        /* Set */  
        ool_pd_write(lu8_PORT_IOMUX_REG + 1, ool_pd_read(lu8_PORT_IOMUX_REG + 1) | (1 << (PWMParam.Pin-4) * 2)); 
    }
}

/*********************************************************************
 * @fn      pmu_pwm_single_start
 *
 * @brief   pwm start output.(single mode)
 *
 * @param   PWM: select PWM.
 * @return  None.
 */
void pmu_pwm_single_start(e_PWM_t PWM)
{
    uint8_t lu8_PWM_CTRL_REG = PMU_REG_PD_PTC0_CTRL;
    
    lu8_PWM_CTRL_REG += PWM;
    
    ool_pd_write(lu8_PWM_CTRL_REG, ool_pd_read(lu8_PWM_CTRL_REG) | PMU_PTC_CTRL_PTC_EN | 
                                                                   PMU_PTC_CTRL_PTC_OUT_EN |
                                                                   PMU_PTC_CTRL_PTC_MODE);
}

/*********************************************************************
 * @fn      pmu_pwm_cycle_start
 *
 * @brief   pwm start output.(cycle mode)
 *
 * @param   PWM: select PWM.
 * @return  None.
 */
void pmu_pwm_cycle_start(e_PWM_t PWM)
{
    uint8_t lu8_PWM_CTRL_REG = PMU_REG_PD_PTC0_CTRL;
    
    lu8_PWM_CTRL_REG += PWM;
    
    ool_pd_write(lu8_PWM_CTRL_REG, ool_pd_read(lu8_PWM_CTRL_REG) & ~PMU_PTC_CTRL_PTC_MODE);
    ool_pd_write(lu8_PWM_CTRL_REG, ool_pd_read(lu8_PWM_CTRL_REG) | PMU_PTC_CTRL_PTC_EN | PMU_PTC_CTRL_PTC_OUT_EN);
}

/*********************************************************************
 * @fn      pmu_pwm_stop
 *
 * @brief   pwm stop output.
 *
 * @param   PWM: select PWM.
 * @return  None.
 */
void pmu_pwm_stop(e_PWM_t PWM)
{
    uint8_t lu8_PWM_CTRL_REG = PMU_REG_PD_PTC0_CTRL;

    lu8_PWM_CTRL_REG += PWM;

    ool_pd_write(lu8_PWM_CTRL_REG, ool_pd_read(lu8_PWM_CTRL_REG) & ~(PMU_PTC_CTRL_PTC_MODE | PMU_PTC_CTRL_PTC_OUT_EN));
}

