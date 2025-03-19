/*
  ******************************************************************************
  * @file    driver_if8080.h
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Header file of IF8080 HAL module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __DRIVER_LCD_H__
#define __DRIVER_LCD_H__

#include "stdint.h"
#include "stdbool.h"

#include "sys_utils.h"

/** @addtogroup DMA_Registers_Section
  * @{
  */
/* ################################ Register Section Start ################################ */

/* Configuration Register */
typedef struct 
{
    uint32_t DATA : 8;
    uint32_t LEN  : 22;
    uint32_t CDX  : 1;     /* 1:Param, 0:CMD */
    uint32_t WR   : 1;     /* 1:Write, 0:Read */
}REG_CFG_t;

/* Configuration write status Register */
typedef struct 
{
    uint32_t STATUS : 1;
    uint32_t rsv_0  : 31;
}REG_CFGWR_STATUS_t;

/* Configuration read status Register */
typedef struct 
{
    uint32_t STATUS : 1;
    uint32_t rsv_0  : 31;
}REG_CFGRD_STATUS_t;

/* read Receive data Register */ 
typedef struct 
{
    uint32_t RD_DATA : 16;
    uint32_t rsv_0   : 16;
}REG_APB_RD_t;

/* tx fifo almost empty level */
typedef struct 
{
    uint32_t LEVEL : 5;
    uint32_t rsv_0 : 27;
}REG_TXFF_AEMP_LV_t;

/* Write data length */
typedef struct 
{
    uint32_t LEN   : 24;
    uint32_t rsv_0 : 8;
}REG_DATA_WR_LEN_t;

/* Data transmission Configuration */
typedef struct 
{
    uint32_t WR_L_LEN         : 3;
    uint32_t rsv_0            : 1;
    uint32_t WR_H_LEN         : 3;
    uint32_t DATA_BUS_BIT     : 1;
    uint32_t DATA_TRANS_SEQ_0 : 2;
    uint32_t DATA_TRANS_SEQ_1 : 2;
    uint32_t DATA_TRANS_SEQ_2 : 2;
    uint32_t DATA_TRANS_SEQ_3 : 2;
    uint32_t rsv_1:16;
}REG_DATA_CFG_t;

/* DMA Configuration */
typedef struct 
{
    uint32_t DMA_TX_LEVEL : 5;
    uint32_t DMA_ENABLE   : 1;
    uint32_t rsv_0        : 26;
}REG_DMA_t;

/* CS */
typedef struct 
{
    uint32_t LCD_CS : 1;
    uint32_t rsv_0  : 31;
}REG_CS_t;

/* Write/Read clock Config */
typedef struct 
{
    uint32_t WRITE_CLK_CFG : 3;
    uint32_t rsv_0         : 1;
    uint32_t READ_CLK_CFG  : 4;
    uint32_t rsv_1         : 24;
}REG_WR_CLK_t;

/* Tx FIFO Clear */
typedef struct 
{
    uint32_t POP_CLR  : 1;
    uint32_t PUSH_CLR : 1;
    uint32_t rsv_0    : 30;
}REG_TXFF_CLR_t;

/* IF8080 interface config */
typedef struct 
{
    uint32_t DC_POLARITY : 1;
    uint32_t CS_POLARITY : 1;
    uint32_t MODE        : 1;
    uint32_t RD          : 1;
    uint32_t rsv_0       : 28;
}REG_INTERFACE_CFG_t;

/* Interrupt control */
typedef struct 
{
    uint32_t TX_FULL_INT_ENABLE : 1;
    uint32_t TX_FULL_INT_STATUS : 1;

    uint32_t TX_EMPT_INT_ENABLE : 1;
    uint32_t TX_EMPT_INT_STATUS : 1;
        
    uint32_t TX_AEMPT_INT_ENABLE : 1;
    uint32_t TX_AEMPT_INT_STATUS : 1;

    uint32_t TE_VSYNC_INT_ENABLE : 1; 
    uint32_t TE_VSYNC_INT_STATUS : 1; 
    uint32_t TE_VSYNC_INT_CLEAR  : 1; 

    uint32_t TE_HSYNC_INT_ENABLE : 1; 
    uint32_t TE_HSYNC_INT_STATUS : 1; 
    uint32_t TE_HSYNC_INT_CLEAR  : 1; 

    uint32_t rsv_0 : 24;
}REG_INT_CONTROL_t;

/* VSYNC config */
typedef struct 
{
    uint32_t TE_VSYNC_WIDTH_NUM : 24;
    uint32_t rsv_0              : 8;
}REG_TE_VSYNC_CFG_t;

/* HSYNC config */
typedef struct 
{
    uint32_t TE_HSYNC_LINE_NUM  : 12;
    uint32_t TE_HSYNC_POLARITY  : 1;
    uint32_t rsv_0              : 19;
}REG_TE_HSYNC_CFG_t;


/* -------------------------------------------*/
/*                IF8080 Register                */
/* -------------------------------------------*/
typedef struct 
{
    volatile uint32_t               TX_FIFO;         // offset 0x00
    volatile uint32_t               CFG;             // offset 0x04
    volatile REG_CFGWR_STATUS_t     CFGWR_STATUS;    // offset 0x08
    volatile REG_CFGRD_STATUS_t     CFGRD_STATUS;    // offset 0x0C
    volatile REG_APB_RD_t           APB_RD;          // offset 0x10
    volatile REG_TXFF_AEMP_LV_t     TXFF_AEMP_LV;    // offset 0x14
    volatile REG_DATA_WR_LEN_t      DATA_WR_LEN;     // offset 0x18
    volatile REG_DATA_CFG_t         DATA_CFG;        // offset 0x1C
    volatile REG_DMA_t              DMA_CFG;         // offset 0x20
    volatile REG_CS_t               CSX;             // offset 0x24
    volatile REG_WR_CLK_t           CRM;             // offset 0x28
    volatile REG_TXFF_CLR_t         TXFF_CLR;        // offset 0x2C
    volatile REG_INTERFACE_CFG_t    INTF_CFG;        // offset 0x30
    volatile REG_INT_CONTROL_t      INT;             // offset 0x34
    volatile REG_TE_VSYNC_CFG_t     TE_VSYNC_CFG;    // offset 0x38
    volatile REG_TE_HSYNC_CFG_t     TE_HSYNC_CFG;    // offset 0x3C
}struct_LCD_t;

#define LCD_ADDR_BASE    (0x500D0000)

#define IF8080              ((volatile struct_LCD_t *)LCD_ADDR_BASE)

/* ################################ Register Section END ################################ */
/**
  * @}
  */



/** @addtogroup DMA_Initialization_Config_Section
  * @{
  */
/* ################################ Initialization、Config Section Start ################################ */

typedef enum
{
    MODE_8080,
    MODE_6800,
}enum_LCD_MODE_t;

typedef enum
{
    DATA_BUS_8_BIT,
    DATA_BUS_16_BIT,
}enum_DATA_BUS_t;

typedef enum
{
    RDCLK_DIV_4 = 0x03,
    RDCLK_DIV_6,
    RDCLK_DIV_8,
    RDCLK_DIV_16,
    RDCLK_DIV_32,
    RDCLK_DIV_64,
}enum_RDCLK_DIV_t;

typedef enum
{
    WDCLK_DIV_1,
    WDCLK_DIV_2,
    WDCLK_DIV_3,
    WDCLK_DIV_4,
    WDCLK_DIV_6,
    WDCLK_DIV_8,
}enum_WDCLK_DIV_t;

typedef enum
{
    INT_HSYNC,
    INT_VSYNC,
    INT_AEMPT,
    INT_EMPTY,
    INT_FULL,
}enum_INT_t;

/**
  * @brief  IF8080 Initialization Structure definition
  */
typedef struct 
{
    uint32_t LcdMode;                   /* This parameter can be a value of @ref enum_LCD_MODE_t */
    uint32_t DataBusSelect;             /* This parameter can be a value of @ref enum_DATA_BUS_t */
    uint32_t ReadClock;                 /* This parameter can be a value of @ref enum_RDCLK_DIV_t */
    uint32_t WriteClock;                /* This parameter can be a value of @ref enum_WDCLK_DIV_t */
}str_LCDParam_t;

/* ################################ Initialization、Config Section END ################################ */
/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/

/* if8080_init */
void if8080_init(str_LCDParam_t LcdInit);

/* if8080_write_cmd */
void if8080_write_cmd(uint8_t fp8_CMD);

/* if8080_write_param */
void if8080_write_param(uint16_t fu16_Data);

/* if8080_write_data */
void if8080_write_data(uint32_t *fp32_WriteBuffer, uint32_t fu32_WriteNum);

/* if8080_read_data_8bit */
void if8080_read_data_8bit(uint8_t fu8_Param, uint8_t *fp8_ReadBuffer, uint32_t fu32_ReadNum);

/* if8080_read_data_16bit */
void if8080_read_data_16bit(uint8_t fu8_Param, uint16_t *fp16_ReadBuffer, uint32_t fu32_ReadNum);




/* Exported inline functions --------------------------------------------------------*/


/* if8080_tx_fifo_level */
static inline void if8080_tx_fifo_almost_empty_level(uint8_t fu8_level)
{
    /* fu8_level: 0 ~ 31 */
    IF8080->TXFF_AEMP_LV.LEVEL = fu8_level;
}

/* if8080_tx_fifo_reset */
static inline void if8080_tx_fifo_reset(void)
{
    IF8080->TXFF_CLR.POP_CLR  = 1;
    IF8080->TXFF_CLR.PUSH_CLR = 1;
    IF8080->TXFF_CLR.POP_CLR  = 0;
    IF8080->TXFF_CLR.PUSH_CLR = 0;
}

/* if8080_wait_bus_idle */
static inline void if8080_wait_bus_idle(void)
{
    /* wait FIFO empty */
    while (!(IF8080->INT.TX_EMPT_INT_STATUS));

    co_delay_10us(1);
}

/* if8080_dma_requset_level */
static inline void if8080_dma_requset_level(uint8_t fu8_level)
{
    /* fu8_level: 0 ~ 31 */
    IF8080->DMA_CFG.DMA_TX_LEVEL = fu8_level;
}

/* if8080_interrupt_enable */
/* if8080_interrupt_disable */
/* if8080_interrupt_get_status */
/* if8080_interrupt_clear_status */
static inline void if8080_interrupt_enable(enum_INT_t fe_INT_Index)
{
    switch (fe_INT_Index)
    {
        case INT_HSYNC: IF8080->INT.TE_HSYNC_INT_ENABLE = 1; break;
        case INT_VSYNC: IF8080->INT.TE_VSYNC_INT_ENABLE = 1; break;
        case INT_AEMPT: IF8080->INT.TX_AEMPT_INT_ENABLE = 1; break;
        case INT_EMPTY: IF8080->INT.TX_EMPT_INT_ENABLE  = 1; break;
        case INT_FULL:  IF8080->INT.TX_FULL_INT_ENABLE  = 1; break;

        default: break; 
    }
}
static inline void if8080_interrupt_disable(enum_INT_t fe_INT_Index)
{
    switch (fe_INT_Index)
    {
        case INT_HSYNC: IF8080->INT.TE_HSYNC_INT_ENABLE = 0; break;
        case INT_VSYNC: IF8080->INT.TE_VSYNC_INT_ENABLE = 0; break;
        case INT_AEMPT: IF8080->INT.TX_AEMPT_INT_ENABLE = 0; break;
        case INT_EMPTY: IF8080->INT.TX_EMPT_INT_ENABLE  = 0; break;
        case INT_FULL:  IF8080->INT.TX_FULL_INT_ENABLE  = 0; break;

        default: break; 
    }
}
static inline bool if8080_interrupt_get_status(enum_INT_t fe_INT_Index)
{
    switch (fe_INT_Index)
    {
        case INT_HSYNC: return IF8080->INT.TE_HSYNC_INT_STATUS;
        case INT_VSYNC: return IF8080->INT.TE_VSYNC_INT_STATUS;
        case INT_AEMPT: return IF8080->INT.TX_AEMPT_INT_STATUS;
        case INT_EMPTY: return IF8080->INT.TX_EMPT_INT_STATUS;
        case INT_FULL:  return IF8080->INT.TX_FULL_INT_STATUS;

        default: break; 
    }
    return false;
}
static inline void if8080_interrupt_clear_status(enum_INT_t fe_INT_Index)
{
    switch (fe_INT_Index)
    {
        case INT_HSYNC: IF8080->INT.TE_HSYNC_INT_CLEAR = 1;
        case INT_VSYNC: IF8080->INT.TE_VSYNC_INT_CLEAR = 1;

        default: break; 
    }
}

/* if8080_cs_set */
/* if8080_cs_release */
static inline void if8080_cs_set(void)
{
    IF8080->CSX.LCD_CS = 0;
}
static inline void if8080_cs_release(void)
{
    IF8080->CSX.LCD_CS = 1;
}


/* if8080_set_bus_8bit */
/* if8080_set_bus_16bit */
static inline void if8080_set_bus_8bit(void)
{
    IF8080->DATA_CFG.DATA_BUS_BIT = DATA_BUS_8_BIT;
}
static inline void if8080_set_bus_16bit(void)
{
    IF8080->DATA_CFG.DATA_BUS_BIT = DATA_BUS_16_BIT;
}


/* if8080_wrclk_div */
/* if8080_rdclk_div */
static inline void if8080_wrclk_div(enum_WDCLK_DIV_t fe_WDCLK_DIV)
{
    IF8080->CRM.WRITE_CLK_CFG = fe_WDCLK_DIV;
}
static inline void if8080_rdclk_div(enum_RDCLK_DIV_t fe_RDCLK_DIV)
{
    IF8080->CRM.READ_CLK_CFG = fe_RDCLK_DIV;
}


/* if8080_wrclk_HighLength */
/* if8080_wrclk_LowLength  */
static inline void if8080_wrclk_HighLength(uint8_t fu8_Length)
{
    /* fu8_ClockCount: 0 ~ 7 */
    IF8080->DATA_CFG.WR_H_LEN = fu8_Length;
}
static inline void if8080_wrclk_LowLength(uint8_t fu8_Length)
{
    /* fu8_ClockCount: 0 ~ 7 */
    IF8080->DATA_CFG.WR_L_LEN = fu8_Length;
}


/* if8080_CS_Polarity */
/* if8080_DC_Polarity */
static inline void if8080_CS_Polarity(bool fb_Polarity)
{
    /* fb_Polarity: 1: high active */
    /*              0: low  active */
    IF8080->INTF_CFG.CS_POLARITY = fb_Polarity;
}
static inline void if8080_DC_Polarity(bool fb_Polarity)
{
    /* fb_Polarity: 1: high active */
    /*              0: low  active */
    IF8080->INTF_CFG.DC_POLARITY = fb_Polarity;
}


/* if8080_VSYNC_width */
/* if8080_HSYNC_length */
static inline void if8080_VSYNC_width(uint32_t fu32_width)
{
    IF8080->TE_VSYNC_CFG.TE_VSYNC_WIDTH_NUM = fu32_width;
}
static inline void if8080_HSYNC_length(uint32_t fu32_length, bool fb_polarity)
{
    /* 
        fb_polarity: 1: Rising edge count 
                     0: Falling edge count
    */
    IF8080->TE_HSYNC_CFG.TE_HSYNC_POLARITY = fb_polarity;
    IF8080->TE_HSYNC_CFG.TE_HSYNC_LINE_NUM = fu32_length;
}

#endif
