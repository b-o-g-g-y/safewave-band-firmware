/*
  ******************************************************************************
  * @file    driver_wdt.h
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Header file of WDT HAL module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __DRIVER_WDT_H__
#define __DRIVER_WDT_H__

#include "stdint.h"
#include "stdbool.h"

#include "driver_pmu.h"

#include "sys_utils.h"

enum wdt_IRQ_t 
{
    WDT_IRQ_ENABLE  = 0x04,
    WDT_IRQ_DISABLE = 0x00,
};

/**
  * @brief  DMA Initialization Structure definition
  */
typedef struct 
{
    uint32_t WdtCount;      /* This parameter can be a 24-bit Size */

    uint32_t Timeout;       /* This parameter can be a 8-bit Size */
    
    uint32_t IRQ_Enable;    /* This parameter can be a value of @ref wdt_IRQ_t */
}wdt_Init_t;


/* Exported functions --------------------------------------------------------*/

/* wdt_init */
void wdt_init(wdt_Init_t Init);

/* wdt_IRQ_handler */
void wdt_IRQ_handler(void);

/* wdt_Refresh */
void wdt_Refresh(void);

/* wdt_Interrupt_Enable */
/* wdt_Interrupt_Disable */
void wdt_Interrupt_Enable(void);
void wdt_Interrupt_Disable(void);

/* wdt_Enable */
/* wdt_Disable */
void wdt_Enable(void);
void wdt_Disable(void);

/* wdt_Set_Count */
void wdt_Set_Count(uint32_t WdtCount);

/* wdt_Set_Timeout */
void wdt_Set_Timeout(uint8_t WdtTimeout);

#endif


