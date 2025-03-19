/*
  ******************************************************************************
  * @file    driver_pwm.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   SPI module driver.
  *          This file provides firmware functions to manage the 
  *          Pulse-Width Modulatio (PWM) peripheral
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "driver_pwm.h"

/************************************************************************************
 * @fn      pwm_config
 *
 * @brief   PWM mode. Config channel paramter.
 *
 * @param   fu8_channel: Select output channel.(1bit ~ 1channel)
 *          fstr_Config: Config paramter.
 */
void pwm_config(uint8_t fu8_channel, struct_PWM_Config_t fstr_Config)
{
    uint32_t lu32_Position = 0;
    uint32_t lu32_Current_Channel;

    PWM->ChannelEN    |=  fu8_channel;
    PWM->OutputSelect &= ~fu8_channel;
    PWM->DACMode      &= ~fu8_channel;

    /* Configure Select channel */
    while (fu8_channel >> lu32_Position != 0) 
    {
        /* Get current pin position */
        lu32_Current_Channel = fu8_channel & (1uL << lu32_Position);

        if (lu32_Current_Channel) 
        {
            /* stop  sync update */
            PWM->Update.PWM_Update &= ~lu32_Current_Channel;

            PWM->Edge[lu32_Position].Posedge = fstr_Config.Posedge;
            PWM->Edge[lu32_Position].Negedeg = fstr_Config.Negedge;

            PWM->Frequency[lu32_Position].Prescale = fstr_Config.Prescale - 1;
            PWM->Frequency[lu32_Position].Period   = fstr_Config.Period - 1;
        }

        lu32_Position++;
    }
}

/************************************************************************************
 * @fn      pwm_dac_config
 *
 * @brief   PWM_DAC mode. Config channel paramter.
 *
 * @param   fu8_channel: Select output channel.(1bit ~ 1channel)
 *          fstr_Config: Config paramter.
 */
void pwm_dac_config(uint8_t fu8_channel, struct_PWM_DAC_Config_t fstr_Config)
{
    uint32_t lu32_Position = 0;
    uint32_t lu32_Current_Channel;

    PWM->CNT_EN       |=  fu8_channel;
    PWM->ChannelEN    |=  fu8_channel;
    PWM->OutputSelect &= ~fu8_channel;
    PWM->DACMode      |=  fu8_channel;

    /* Configure Select channel */
    while (fu8_channel >> lu32_Position != 0) 
    {
        /* Get current pin position */
        lu32_Current_Channel = fu8_channel & (1uL << lu32_Position);

        if (lu32_Current_Channel) 
        {
            PWM->DAC_DIV[lu32_Position] = fstr_Config.PulseDIV;

            PWM->Edge[lu32_Position].Negedeg = ((uint64_t)0xFFFFFFFF * (uint64_t)fstr_Config.Duty) / 100;
        }

        lu32_Position++;
    }
}

/************************************************************************************
 * @fn      pwm_complementary_config
 *
 * @brief   Complementary outputs with dead-time insertion
 *
 * @param   fu8_MainChannel:      Select main          output channel.(1bit ~ 1channel)
 * @param   ComplementaryChannel: Select complementary output channel.(1bit ~ 1channel)
 *          fstr_Config: Config paramter.
 */
bool pwm_complementary_config(uint8_t fu8_MainChannel, uint8_t ComplementaryChannel, struct_PWM_Complementary_Config_t fstr_Config)
{
    struct_PWM_Config_t PWM_Config;
    
    PWM_Config.Prescale = fstr_Config.Prescale;
    PWM_Config.Period   = fstr_Config.Period;

    if (fstr_Config.MianDeadTime != 0) 
    {
        if (fstr_Config.MianDeadTime - 1 >= PWM_Config.Period - 1 - fstr_Config.DutyCycle - fstr_Config.MianDeadTime) 
            return false;

        PWM_Config.Posedge = fstr_Config.MianDeadTime - 1;
        PWM_Config.Negedge = PWM_Config.Period - 1 - fstr_Config.DutyCycle - fstr_Config.MianDeadTime;
    }
    else 
    {
        /* Error check */
        if (fstr_Config.DutyCycle >= PWM_Config.Period) 
            return false;
        if (fstr_Config.DutyCycle == 0)
            return false;

        PWM_Config.Posedge = PWM_Config.Period - 1;
        PWM_Config.Negedge = PWM_Config.Period - 1 - fstr_Config.DutyCycle;
    }

    pwm_config(fu8_MainChannel, PWM_Config);
    
    if (fstr_Config.CompDeadTime != 0) 
    {
        if (PWM_Config.Period - 1 - fstr_Config.DutyCycle + fstr_Config.MianDeadTime >= PWM_Config.Period - 1 - fstr_Config.CompDeadTime) 
            return false;

        PWM_Config.Posedge = PWM_Config.Period - 1 - fstr_Config.DutyCycle + fstr_Config.MianDeadTime;
        PWM_Config.Negedge = PWM_Config.Period - 1 - fstr_Config.CompDeadTime;
    }
    else 
    {
        /* Error check */
        if (fstr_Config.DutyCycle >= PWM_Config.Period) 
            return false;
        if (fstr_Config.DutyCycle == 0)
            return false;

        PWM_Config.Posedge = PWM_Config.Period - 1 - fstr_Config.DutyCycle;
        PWM_Config.Negedge = PWM_Config.Period - 1;
    }

    pwm_config(ComplementaryChannel, PWM_Config);
    
    return true;
}

/************************************************************************************
 * @fn      pwm_output_enable
 *
 * @brief   PWM_DAC mode. output enable.
 *
 * @param   fu8_channel: Select output channel.(1bit ~ 1channel)
 */
void pwm_output_enable(uint8_t fu8_channel)
{
    /* start sync update */
    PWM->Update.PWM_Update |= fu8_channel;

    PWM->OutputEN &= ~fu8_channel;
    PWM->CNT_EN   |= fu8_channel;
}

/************************************************************************************
 * @fn      pwm_output_disable
 *
 * @brief   PWM_DAC mode. output disable.
 *
 * @param   fu8_channel: Select output channel.(1bit ~ 1channel)
 */
void pwm_output_disable(uint8_t fu8_channel)
{
    /* stop sync update */
    PWM->Update.PWM_Update &= ~fu8_channel;

    PWM->OutputEN |= fu8_channel;
    PWM->CNT_EN   &= ~fu8_channel;
}

/************************************************************************************
 * @fn      pwm_output_inverter_disable
 *
 * @brief   PWM_DAC mode. output inverter disable.
 *
 * @param   fu8_channel: Select output channel.(1bit ~ 1channel)
 *  
 * @return  true : PWM runing.
 *          false: PWM stop.
 */
bool pwm_output_status(enum_PWMChannel_t fe_channel)
{
    if (PWM->Update.PWM_Status & fe_channel) 
    {
        return true;
    }
    else 
    {
        return false;
    }
}

/************************************************************************************
 * @fn      pwm_output_updata
 *
 * @brief   channel Posedge/Negedeg/Prescale/Period updata
 *
 * @param   fu8_channel: Select output channel.(1bit ~ 1channel)
 */
void pwm_output_updata(uint8_t fu8_channel)
{
    /* start sync update */
    PWM->Update.PWM_Update |= fu8_channel;
}

/************************************************************************************
 * @fn      pwm_capture_config
 *
 * @brief   Capture mode. Config channel paramter.
 *
 * @param   fu8_channel: Select capture channel.
 *          fstr_Config: Config paramter.
 */
void pwm_capture_config(uint8_t fu8_channel, struct_Capture_Config_t fstr_Config)
{
    uint32_t lu32_Position = 0;
    uint32_t lu32_Current_Channel;

    /* Set Capture Prescale */
    PWM->CapturePrescale = 0x10 | fstr_Config.CapturePrescale; 

    /* Configure Select channel */
    while (fu8_channel >> lu32_Position != 0) 
    {
        /* Get current pin position */
        lu32_Current_Channel = fu8_channel & (1uL << lu32_Position);

        if (lu32_Current_Channel) 
        {
            /* Set Capture mode */
            if (fstr_Config.CaptureMode == MODE_LOOP) 
            {
                PWM->CaptureCtrl.Capture_Mode &= ~lu32_Current_Channel;
            }
            else 
            {
                PWM->CaptureCtrl.Capture_Mode |= lu32_Current_Channel;
            }
        }

        lu32_Position++;
    }
}

/************************************************************************************
 * @fn      pwm_capture_enable
 *
 * @brief   capture enable.
 *
 * @param   fu8_channel: Select capture channel.(1bit ~ 1channel)
 */
void pwm_capture_enable(uint8_t fu8_channel)
{
    PWM->CaptureCtrl.Capture_EN |= fu8_channel;
}

/************************************************************************************
 * @fn      pwm_capture_disable
 *
 * @brief   capture disable.
 *
 * @param   fu8_channel: Select capture channel.(1bit ~ 1channel)
 */
void pwm_capture_disable(uint8_t fu8_channel)
{
    PWM->CaptureCtrl.Capture_EN &= ~fu8_channel;
}

/************************************************************************************
 * @fn      pwm_capture_status
 *
 * @brief   capture disable.
 *
 * @param   fu8_channel: Select capture channel.(1bit ~ 1channel)
 * @return  true : capture result value ready.
 *          false: capture result value not ready.
 */
bool pwm_capture_status(enum_PWMChannel_t fe_channel)
{
    return (PWM->CaptureStatus & fe_channel) ? true : false;
}

/************************************************************************************
 * @fn      pwm_capture_status_clear
 *
 * @brief   capture status claear.
 *
 * @param   fu8_channel: Select capture channel.(1bit ~ 1channel)
 */
void pwm_capture_status_clear(enum_PWMChannel_t fe_channel)
{
    PWM->CaptureStatus |= fe_channel;
}

/************************************************************************************
 * @fn      pwm_capture_value
 *
 * @brief   capture disable.
 *
 * @param   fu8_channel: Select capture channel.(1bit ~ 1channel)
 */
uint32_t pwm_capture_value(enum_PWMChannel_t fe_channel)
{
    uint8_t i;
    
    for (i = 0; i < 8; i++)
    {
        if (fe_channel & 1 << i)
            break;
    }

    return PWM->CaptureValue[i];
}
