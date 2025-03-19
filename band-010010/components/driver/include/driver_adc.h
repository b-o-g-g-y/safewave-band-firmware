/*
  ******************************************************************************
  * @file    driver_adc.h
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Header file of ADC HAL module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __DRIVER_ADC_H__
#define __DRIVER_ADC_H__

#include "stdint.h"
#include "stdbool.h"

#include "driver_pmu.h"

/** @addtogroup ADC_Registers_Section
  * @{
  */
/* ################################ Register Section Start ################################ */

/* Control Register for ADC */
typedef struct
{
    uint32_t ADC_CLK_EN             : 1;
    uint32_t FIFO_EN                : 1;
    uint32_t ADC_MODE               : 1;
    uint32_t DATA_VALID             : 1;
    uint32_t rsv_0                  : 3;
    uint32_t ADC_EN                 : 1;
    uint32_t Channel_EN             : 8;
    uint32_t FIFO_Almost_Full_Level : 6;
    uint32_t rsv_1                  : 1;
    uint32_t DMA_EN                 : 1;
    uint32_t DMA_Level              : 6;
    uint32_t ADC_TRIG_SW            : 1;
    uint32_t FIFO_CLR               : 1;
}REG_CONTROL_t;

/* FIFO Status Register */
typedef union
{
    struct
    {
        uint32_t FIFO_FULL        : 1;
        uint32_t FIFO_EMPTY       : 1;
        uint32_t FIFO_ALMOST_FULL : 1;
        uint32_t FIFO_OVER        : 1;
        uint32_t FIFO_UNDER       : 1;
        uint32_t rsv_0            : 27;
    }FIFO_Status_BIT;
    
    uint32_t FIFO_Statsu_DWORD;
}REG_FIFO_STATUS_t;

/* INT Register */
typedef struct
{
    uint32_t FIFO_FULL_INT           : 1;
    uint32_t FIFO_EMPTY_INT          : 1;
    uint32_t FIFO_ALMOST_FULL_INT    : 1;
    uint32_t FIFO_OVER_INT           : 1;
    uint32_t FIFO_UNDER_INT          : 1;
    uint32_t ERR_INT                 : 1;
    uint32_t SW_INT                  : 1;
    uint32_t rsv_0                   : 9;
    uint32_t FIFO_FULL_STATUS        : 1;
    uint32_t FIFO_EMPTY_STATUS       : 1;
    uint32_t FIFO_ALMOST_FULL_STATUS : 1;
    uint32_t FIFO_OVER_STATUS        : 1;
    uint32_t FIFO_UNDER_STATUS       : 1;
    uint32_t ERR_STATUS              : 1;
    uint32_t SW_STATUS               : 1;
    uint32_t rsv_1                   : 9;
}REG_INT_t;


/* INT Clear Register */
typedef struct
{
    uint32_t FIFO_FULL_CLR        : 1;
    uint32_t FIFO_EMPTY_CLR       : 1;
    uint32_t FIFO_ALMOST_FULL_CLR : 1;
    uint32_t FIFO_OVER_CLR        : 1;
    uint32_t FIFO_UNDER_CLR       : 1;
    uint32_t ERR_CLR              : 1;
    uint32_t SW_CLR               : 1;
    uint32_t rsv_0                : 25;
}REG_INTC_t;


/* ADC Analog Control0 Register */
typedef struct
{
    uint32_t ADC_POWER       : 1;
    uint32_t RSTN            : 1;
    uint32_t ISON            : 1;
    uint32_t CLK_EDGE        : 1;
    uint32_t SACLK_CTL       : 1;
    uint32_t SAMPLE_SIG_SEL  : 3;
    uint32_t CLK_DIV         : 8;
    uint32_t Delay_For_Setup : 16;
}REG_ANA_CTRL0_t;


/* ADC Analog Control1 Register */
typedef struct
{
    uint32_t rsv_0            : 1;
    uint32_t BUF_EN           : 1;
    uint32_t BUF_IN_SEL       : 1;
    uint32_t REFH_MODE        : 1;
    uint32_t VBAT_DIV_EN      : 1;
    uint32_t rsv_1            : 4;
    uint32_t REF1P2V_BUF_EN   : 1;
    uint32_t rsv_2            : 22;
}REG_ANA_CTRL1_t;


/* ADC Analog Control2 Register */
typedef struct
{
    uint32_t POLL_MODE    : 1;
    uint32_t TOUT_MASK    : 1;
    uint32_t CONV_TIME    : 5;
    uint32_t rsv_0        : 9;
    uint32_t SAMPLE_POINT : 8;
    uint32_t rsv_1        : 8;
}REG_ANA_CTRL2_t;


/* ------------------------------------------------*/
/*                   ADC Register                 */
/* ------------------------------------------------*/
typedef struct 
{
    volatile REG_CONTROL_t     CONTROL;
    volatile uint32_t          Channel_DATA[8];
    volatile uint32_t          DATA;
    volatile REG_FIFO_STATUS_t FIFO_STATUS;
    volatile REG_INT_t         INT;
    volatile REG_INTC_t        INTC;
    volatile REG_ANA_CTRL0_t   ANA_CTRL0;
    volatile REG_ANA_CTRL1_t   ANA_CTRL1;
    volatile REG_ANA_CTRL2_t   ANA_CTRL2;
}struct_ADC_t;

#define ADC_ADDR_BASE    (0x50090000)

#define ADC              ((struct_ADC_t *)ADC_ADDR_BASE)

/* ################################ Register Section END ################################ */
/**
  * @}
  */



/** @addtogroup ADC_Initialization_Config_Section
  * @{
  */
/* ################################ Initialization¡¢Config Section Start ################################ */

enum adc_channel_t 
{
    ADC_CHANNEL_0 = 0x01,
    ADC_CHANNEL_1 = 0x02,
    ADC_CHANNEL_2 = 0x04,
    ADC_CHANNEL_3 = 0x08,
    ADC_CHANNEL_4 = 0x10,
    ADC_CHANNEL_5 = 0x20,
    ADC_CHANNEL_6 = 0x40,
    ADC_CHANNEL_7 = 0x80,
};

enum adc_fifo_Status_t 
{
    FIFO_FULL        = 0x01,
    FIFO_EMPTY       = 0x02,
    FIFO_ALMOST_FULL = 0x04,
    FIFO_OVER        = 0x08,
    FIFO_UNDER       = 0x10,
};

typedef enum
{
    FIFO_FULL_INT,
    FIFO_EMPTY_INT,
    FIFO_ALMOST_FULL_INT,
    FIFO_OVER_INT,
    FIFO_UNDER_INT,
    ERR_INT,
    SW_INT,
}enum_int_t;

enum adc_sample_sel_t 
{
    IO_CHANNEL = 3,
    VBE        = 6,
    VBAT       = 7,
};

enum adc_fifo_enable_t 
{
    FIFO_DISABLE,
    FIFO_ENABLE,
};

enum adc_dma_enable_t 
{
    DMA_DISABLE,
    DMA_ENABLE,
};

enum adc_reference_t 
{
    ADC_REF_LDOIO,
    ADC_REF_1P2V,
};

typedef enum
{
    RES_120K = 1,
    RES_240K = 2,
    RES_80K  = 3,
}enum_res_t;

typedef enum
{
    DIV_1_4 = 0,    // 1/4
    DIV_1_3 = 1,    // 1/3
    DIV_1_2 = 2,    // 1/2
    DIV_2_3 = 3,    // 2/3
}enum_div_t;

/**
  * @brief  ADC Initialization Structure definition
  */
typedef struct 
{
    uint32_t ADC_CLK_DIV;             /* This parameter can be a 8-bit Size, and The minimum value is 1 */

    uint32_t ADC_SetupDelay;          /* This parameter can be a 16-bit Size */

    uint32_t ADC_Reference;           /* This parameter can be a 16-bit Size */
    
    uint32_t FIFO_Enable;             /* This parameter can be a value of @ref adc_fifo_enable_t */

    uint32_t FIFO_AlmostFullLevel;    /* This parameter can be a 6-bit Size */
    
    uint32_t DMA_Enable;              /* This parameter can be a value of @ref adc_dma_enable_t */
    
    uint32_t DMA_Level;               /* This parameter can be a 6-bit Size */

}adc_InitParameter_t;

/* ################################ Initialization¡¢Config Section END ################################ */
/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/

/* adc_init */ 
void adc_init(adc_InitParameter_t InitParam);

/* adc_Channel_ConvertConfig */ 
/* adc_VBAT_ConvertConfig */
/* adc_VBE_ConvertConfig */
void adc_Channel_ConvertConfig(uint8_t fu8_ChannelBit);
void adc_VBAT_ConvertConfig(void);
void adc_VBE_ConvertConfig(void);

/* adc_get_channel_data */ 
/* adc_get_channel_data_FT */
uint32_t adc_get_channel_data(uint8_t fu8_Channel);
uint32_t adc_get_channel_data_FT(uint8_t fu8_Channel);

/* adc_get_data */ 
/* adc_get_data_FT */ 
uint32_t adc_get_data(void);
uint32_t adc_get_data_FT(void);

/* adc_int_enable  */
/* adc_int_disable */
/* adc_get_int_status */
/* adc_clr_int_status */
void adc_int_enable(enum_int_t fe_INT);
void adc_int_disable(enum_int_t fe_INT);
bool adc_get_int_status(enum_int_t fe_INT);
void adc_clr_int_status(enum_int_t fe_INT);

/* adc_get_fifo_Status */
uint32_t adc_get_fifo_Status(void);

/* adc_DataValid_status */ 
bool adc_DataValid_status(void);

/* adc_fifo_clear */ 
void adc_fifo_clear(void);

/* adc_convert_enable */ 
void adc_convert_enable(void);

/* adc_convert_disable */ 
void adc_convert_disable(void);

#endif
