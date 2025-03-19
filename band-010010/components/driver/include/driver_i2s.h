/*
  ******************************************************************************
  * @file    driver_i2s.h
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Header file of I2S HAL module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __DRIVER_I2S_H__
#define __DRIVER_I2S_H__

#include <stdint.h>
#include <stdbool.h>

#include "plf.h"

/** @addtogroup I2S_Registers_Section
  * @{
  */
/* ################################ Register Section Start ################################ */

/* I2S control Register */
typedef struct
{
    uint32_t I2S_EN    : 1;
    uint32_t FMT       : 1;
    uint32_t DLY_EN    : 1;
    uint32_t WS_INV    : 1;
    uint32_t CLK_INV   : 1;
    uint32_t rsv_0     : 2;
    uint32_t RX_INT_EN : 1;    
    uint32_t TX_INT_EN : 1;    
    uint32_t INT_EN    : 1;
    uint32_t HLSEL     : 1;   
    uint32_t rsv_1     : 21;
}REG_I2S_Control_t;

/* I2S FIFO control Register */
typedef struct
{
    uint32_t TX_FIFO_CFG : 8;
    uint32_t rsv_0       : 16;
    uint32_t RX_FIFO_CFG : 8;
}REG_I2S_FIFO_CFG_t;

/* I2S control Register */
typedef struct
{
    uint32_t DMA_TX_EN    : 1;
    uint32_t DMA_RX_EN    : 1;
    uint32_t DMA_RX_LEVEL : 5;
    uint32_t DMA_TX_LEVEL : 5;
    uint32_t rsv_0        : 20;
}REG_I2S_DMA_CFG_t;

typedef enum
{
    I2S_TX_FIFO_EMPTY      = 0x20,
    I2S_TX_FIFO_HALF_EMPTY = 0x10,
    I2S_TX_FIFO_FULL       = 0x08,
    I2S_RX_FIFO_EMPTY      = 0x04,
    I2S_RX_FIFO_HALF_FULL  = 0x02,
    I2S_RX_FIFO_FULL       = 0x01,
}enum_I2S_INT_t;

/* ------------------------------------------------*/
/*                   I2S Register                  */
/* ------------------------------------------------*/
typedef struct 
{
    volatile REG_I2S_Control_t     CONTROL;            /* Offset 0x00 */
    volatile uint32_t              CLK_DIV;            /* Offset 0x04 */
    volatile uint32_t              WS_DIV;             /* Offset 0x08 */
    volatile uint32_t              DATA;               /* Offset 0x0C */
    volatile uint32_t              Status;             /* Offset 0x10 */
    volatile uint32_t              INTE;               /* Offset 0x14 */
    volatile uint32_t              rsv_0[2];
    volatile REG_I2S_FIFO_CFG_t    FIFO_CFG;           /* Offset 0x20 */
    volatile REG_I2S_DMA_CFG_t     DMA_CFG;            /* Offset 0x24 */
}struct_I2S_t;

#define I2S              ((struct_I2S_t *)I2S_BASE)

/* ################################ Register Section END ################################## */
/**
  * @}
  */


/** @addtogroup I2S_Initialization_Config_Section
  * @{
  */
/* ################################ Initialization¡¢Config Section Start ################################ */

/* Rx Data Format */
enum enum_RxDataFormat_t
{
    RIGHT_DATA_IN_HIGH_BYTE,
    LEFT_DATA_IN_HIGH_BYTE,
};

/* Format select */
enum enum_Format_t
{
    Philip,
    Left_Justified,
    PCM_Short,
};

/* inversion enable/disable */
enum enum_INV_t
{
    INV_DISABLE,
    INV_ENABLE,
};

/**
  * @brief  I2S Initialization Structure definition
  */
typedef struct 
{
    uint32_t RxDataFormat;      /* This parameter can be a value of @ref enum_RxDataFormat_t */

    uint32_t Format;            /* This parameter can be a value of @ref enum_Format_t */

    uint32_t Clock_DIV;         /* This parameter can be a 16-bit Size */

    uint32_t Channel_Length;    /* This parameter can be a 16-bit Size */

    uint32_t WS_InvEN;          /* WS    inversion enable. This parameter can be a value of @ref enum_INV_t */
    uint32_t CLK_InvEN;         /* clock inversion enable. This parameter can be a value of @ref enum_INV_t */
}struct_I2SInit_t;

/*
 * @brief  I2S handle Structure definition
 */
typedef struct I2S_HandleTypeDef
{
    struct_I2SInit_t         Init;               /*!< I2S communication parameters      */

    volatile uint32_t        u32_TxSize;         /*!< I2S Transmit parameters in interrupt  */
    volatile uint32_t        u32_TxCount;
    volatile uint32_t       *p_TxData;
    volatile bool            b_TxBusy;

    volatile uint32_t        u32_RxSize;         /*!< I2S Receive parameters in interrupt  */
    volatile uint32_t        u32_RxCount;
    volatile uint32_t       *p_RxData;
    volatile bool            b_RxBusy;
}I2S_HandleTypeDef;

/* ################################ Initialization¡¢Config Section END ################################## */
/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
#define I2S_RX_INT_FULL_LEVEL(__LEVEL__)        (I2S->FIFO_CFG.RX_FIFO_CFG = __LEVEL__)
#define I2S_TX_INT_EMPTY_LEVEL(__LEVEL__)       (I2S->FIFO_CFG.TX_FIFO_CFG = __LEVEL__)

#define I2S_RX_DMA_FULL_LEVEL(__LEVEL__)        (I2S->DMA_CFG.DMA_RX_LEVEL = __LEVEL__)
#define I2S_TX_DMA_EMPTY_LEVEL(__LEVEL__)       (I2S->DMA_CFG.DMA_TX_LEVEL = __LEVEL__)

/* Exported functions --------------------------------------------------------*/

/* i2s_IRQHandler */
void i2s_IRQHandler(I2S_HandleTypeDef *hi2s);

/* i2s_init */
void i2s_init(I2S_HandleTypeDef *hi2s);

/* i2s_enable  */
/* i2s_disable */
void i2s_enable(void);
void i2s_disable(void);

/* i2s_int_enable */
/* i2s_int_disable */
/* i2s_int_get_status */
void i2s_int_enable(enum_I2S_INT_t fe_INT);
void i2s_int_disable(enum_I2S_INT_t fe_INT);
bool i2s_int_is_enable(enum_I2S_INT_t fe_INT);
bool i2s_int_get_status(enum_I2S_INT_t fe_INT);

/* i2s_transmit     */
/* i2s_transmit_IT  */
/* i2s_transmit_DMA */
/* i2s_receive      */
/* i2s_receive_IT   */
/* i2s_receive_DMA  */
void i2s_transmit(uint32_t *fp_Data, uint32_t fu32_Size);
void i2s_transmit_IT(I2S_HandleTypeDef *hi2s, uint32_t *fp_Data, uint32_t fu32_Size);
void i2s_transmit_DMA(void);

void i2s_receive(uint32_t *fp_Data, uint32_t fu32_Size);
void i2s_receive_IT(I2S_HandleTypeDef *hi2s, uint32_t *fp_Data, uint32_t fu32_Size);
void i2s_receive_DMA(void);

void i2s_transmit_receive(uint32_t *fp_TxData, uint32_t *fp_RxData, uint32_t fu32_Size);

#endif
