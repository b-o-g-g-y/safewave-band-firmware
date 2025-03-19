/*
  ******************************************************************************
  * @file    driver_pwm.h
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Header file of PWM HAL module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __DRIVER_PWM_H__
#define __DRIVER_PWM_H__

#include <stdint.h>
#include <stdbool.h>

#include "plf.h"

/** @addtogroup PWM_Registers_Section
  * @{
  */
/* ################################ Register Section Start ################################ */

/* Posedge、Negedeg register */
typedef struct 
{
    uint32_t Posedge;
    uint32_t Negedeg;
}REG_PWMedge_t;

/* capture control register */
typedef struct 
{
    uint32_t Capture_Mode : 8;
    uint32_t Capture_EN   : 8;
    uint32_t rsv_0        : 16;
}REG_CaptureCtrl_t;

/* PWM Update control register */
typedef struct 
{
    uint32_t PWM_Update : 8;
    uint32_t rsv_0      : 8;
    uint32_t PWM_Status : 8;
    uint32_t rsv_1      : 8;
}REG_PWMUpdate_t;

/* Prescale、Period register */
typedef struct 
{
    uint32_t Prescale;
    uint32_t Period;
}REG_PWMFrequency_t;

/* -----------------------------------------------*/
/*                   PWM Register                 */
/* -----------------------------------------------*/
typedef struct 
{
    volatile uint32_t           rsv_0;
    volatile uint32_t           OutputEN;           /* Offset 0x04 */
    volatile uint32_t           ChannelEN;          /* Offset 0x08 */
    volatile uint32_t           rsv_1;
    volatile REG_PWMedge_t      Edge[8];            /* Offset 0x10 ~ 0x4C */
    volatile uint32_t           rsv_2[16];
    volatile uint32_t           OutputSelect;       /* Offset 0x90 */
    volatile uint32_t           CapturePrescale;    /* Offset 0x94 */
    volatile uint32_t           CaptureStatus;      /* Offset 0x98 */
    volatile uint32_t           CaptureINTEN;       /* Offset 0x9C */
    volatile REG_CaptureCtrl_t  CaptureCtrl;        /* Offset 0xA0 */
    volatile uint32_t           CaptureValue[8];    /* Offset 0xA4 ~ 0xC0 */
    volatile uint32_t           rsv_3[8];
    volatile REG_PWMUpdate_t    Update;             /* Offset 0xE4 */
    volatile uint32_t           DACMode;            /* Offset 0xE8 */
    volatile uint32_t           rsv_4;
    volatile uint32_t           OutputValue;        /* Offset 0xF0 */
    volatile uint32_t           rsv_5;
    volatile uint32_t           InverterEN;         /* Offset 0xF8 */
    volatile uint32_t           CNT_EN;             /* Offset 0xFC */
    volatile REG_PWMFrequency_t Frequency[8];       /* Offset 0x100 ~ 0x13C */
    volatile uint32_t           rsv_6[16];
    volatile uint32_t           DAC_DIV[8];         /* Offset 0x180 ~ 0x19C */
}struct_PWM_t;

#define PWM       ((struct_PWM_t *)PWM_BASE)


/* ################################ Register Section END ################################## */
/**
  * @}
  */


/** @addtogroup PWM_Initialization_Config_Section
  * @{
  */
/* ################################ Initialization、Config Section Start ################################ */

/* PWM Channel */
typedef enum
{
    PWM_CHANNEL_0 = 0x01,
    PWM_CHANNEL_1 = 0x02,
    PWM_CHANNEL_2 = 0x04,
    PWM_CHANNEL_3 = 0x08,
    PWM_CHANNEL_4 = 0x10,
    PWM_CHANNEL_5 = 0x20,
    PWM_CHANNEL_6 = 0x40,
    PWM_CHANNEL_7 = 0x80,
}enum_PWMChannel_t;

/* Inverter Enable */
typedef enum
{
    Inverter_Enable,     /* output Inverter enable  */
    Inverter_Disable,    /* output Inverter disable */
}enum_InverterEN_t;

/* Capture_Prescale */
typedef enum
{
    CAPTURE_DIV_1,
    CAPTURE_DIV_2,
    CAPTURE_DIV_4,
    CAPTURE_DIV_8,
    CAPTURE_DIV_16,
    CAPTURE_DIV_32,
    CAPTURE_DIV_64,
    CAPTURE_DIV_128,
    CAPTURE_DIV_256,
    CAPTURE_DIV_512,
    CAPTURE_DIV_1024,
    CAPTURE_DIV_2048,
}enum_CapturePrescale_t;

/* Capture Mode */
typedef enum
{
    MODE_LOOP,      /* Loop    trigger */
    MODE_SINGLE,    /* Single  trigger */
}enum_CaptureMode_t;

/*
 * @brief  PWM mode config parameter
 */
typedef struct 
{
    uint32_t Prescale;      /* Clock = APB_Clock / Prescale */
    uint32_t Period;        /* Period = Clock * PWM_Period  */

    uint32_t Posedge;       /* The position of the rising  edge in the period */
    uint32_t Negedge;       /* The position of the falling edge in the period */
}struct_PWM_Config_t;

/*
 * @brief  PWM_DAC mode config parameter
 */
typedef struct 
{
    uint32_t PulseDIV;    /* Pulse = APB_Clock / 2*( PulseDIV + 1) */
    uint32_t Duty;        /* 0% ~ 100% */
}struct_PWM_DAC_Config_t;

/*
 * @brief  PWM complementary config parameter
 */
typedef struct 
{
    uint32_t Prescale;         /* Clock = APB_Clock / Prescale */
    uint32_t Period;           /* Period = Clock * PWM_Period  */

    uint32_t DutyCycle;        /* Low level hold time in one period */
    
    uint32_t MianDeadTime;     /* Mian Channel dead-time */
    uint32_t CompDeadTime;     /* complementary Channel dead-time */
}struct_PWM_Complementary_Config_t;

/*
 * @brief  Capture mode config parameter
 */
typedef struct 
{
    uint32_t CapturePrescale;    /* This parameter can be a value of @ref enum_CapturePrescale_t*/
    uint32_t CaptureMode;        /* This parameter can be a value of @ref enum_CaptureMode_t */
}struct_Capture_Config_t;

/* ################################ Initialization、Config Section END ################################## */
/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/

/* PWM mode. output inverter Enable/Disable */
#define __PWM_INVERTER_ENABLE(__CHANNEL__)                   (PWM->InverterEN |=  (__CHANNEL__))
#define __PWM_INVERTER_DISABLE(__CHANNEL__)                  (PWM->InverterEN &= ~(__CHANNEL__))

/* Capture mode. Capture interrupt Enable/Disable/Clear */
#define __PWM_CAPTURE_INT_ENABLE(__CHANNEL__)                (PWM->CaptureINTEN  |=  (__CHANNEL__))
#define __PWM_CAPTURE_INT_DISABLE(__CHANNEL__)               (PWM->CaptureINTEN  &= ~(__CHANNEL__))
#define __PWM_CAPTURE_INT_CLEAR(__CHANNEL__)                 (PWM->CaptureStatus |=  (__CHANNEL__))


/* Exported functions --------------------------------------------------------*/

/*------------------------------------------------------------*/
/*------------------     PWM Mode functions ------------------*/
/*------------------ PWM_DAC Mode functions ------------------*/
/*------------------------------------------------------------*/
/* pwm_config */
void pwm_config(uint8_t fu8_channel, struct_PWM_Config_t fstr_Config);

/* pwm_dac_config */
void pwm_dac_config(uint8_t fu32_channel, struct_PWM_DAC_Config_t fsr_Config);

/* pwm_complementary_config */
bool pwm_complementary_config(uint8_t fu8_MainChannel, uint8_t ComplementaryChannel, struct_PWM_Complementary_Config_t fstr_Config);

/* pwm_output_enable/disable */
void pwm_output_enable(uint8_t fu8_channel);
void pwm_output_disable(uint8_t fu8_channel);

/* pwm_output_status */
bool pwm_output_status(enum_PWMChannel_t fe_channel);

/* pwm_output_updata */
void pwm_output_updata(uint8_t fu8_channel);
	
/*------------------------------------------------------------*/
/*------------------ Capture Mode functions ------------------*/
/*------------------------------------------------------------*/
/* pwm_capture_config */
void pwm_capture_config(uint8_t fu8_channel, struct_Capture_Config_t fstr_Config);

/* capture_enable/disable */
void pwm_capture_enable(uint8_t fu8_channel);
void pwm_capture_disable(uint8_t fu8_channel);

/* pwm_capture_status */
bool pwm_capture_status(enum_PWMChannel_t fe_channel);
/* pwm_capture_status_clear */
void pwm_capture_status_clear(enum_PWMChannel_t fe_channel);

/* pwm_capture_value */
uint32_t pwm_capture_value(enum_PWMChannel_t fe_channel);

#endif
