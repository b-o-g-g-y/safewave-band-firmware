/*
  ******************************************************************************
  * @file    driver_i2s.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   I2S module driver.
  *          This file provides firmware functions to manage the 
  *          Inter¡ªIC Sound (I2S) peripheral
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "driver_i2s.h"

/************************************************************************************
 * @fn      i2s_IRQHandler
 *
 * @brief   Handle I2S interrupt request.
 *
 * @param   hi2s: I2S handle.
 */
void i2s_IRQHandler(I2S_HandleTypeDef *hi2s)
{
    /* FIFO empty interrupt enable */
    if (i2s_int_is_enable(I2S_TX_FIFO_EMPTY)) 
    {
        /* FIFO empty */
        if (i2s_int_get_status(I2S_TX_FIFO_EMPTY)) 
        {
            while(!i2s_int_get_status(I2S_TX_FIFO_FULL))
            {
                I2S->DATA = hi2s->p_TxData[hi2s->u32_TxCount++];

                if (hi2s->u32_TxCount >= hi2s->u32_TxSize) 
                {
                    i2s_int_disable(I2S_TX_FIFO_EMPTY);

                    hi2s->b_TxBusy = false;

                    break;
                }
            }
        }
    }

    /* Rx FIFO full interrupt enable */
    if (i2s_int_is_enable(I2S_RX_FIFO_FULL)) 
    {
        /* FIFO full */
        if (i2s_int_get_status(I2S_RX_FIFO_FULL)) 
        {
            while(!i2s_int_get_status(I2S_RX_FIFO_EMPTY))
            {
                hi2s->p_RxData[hi2s->u32_RxCount++] = I2S->DATA;

                if (hi2s->u32_RxCount >= hi2s->u32_RxSize) 
                {
                    i2s_int_disable(I2S_RX_FIFO_FULL);
                    
                    hi2s->b_RxBusy = false;

                    break;
                }
            }
        }
    }
}

/*********************************************************************
 * @fn      i2s_init
 *
 * @brief   Initialize the I2S according to the specified parameters
 *          in the struct_I2SInit_t 
 *
 * @param   hi2s : I2S_HandleTypeDef structure that contains
 *                 the configuration information for I2S module
 */
void i2s_init(I2S_HandleTypeDef *hi2s)
{
    /* Rx channel select */
    I2S->CONTROL.HLSEL = hi2s->Init.RxDataFormat;
    /* I2S format */
    I2S->CONTROL.FMT = hi2s->Init.Format;
    /* WS/clock inversion */
    I2S->CONTROL.WS_INV  = hi2s->Init.WS_InvEN;
    I2S->CONTROL.CLK_INV = hi2s->Init.CLK_InvEN;
    /* I2S interrupt enable */
    I2S->CONTROL.INT_EN    = 1;
    I2S->CONTROL.RX_INT_EN = 1;
    I2S->CONTROL.TX_INT_EN = 1;
    
    /* Format */
    if (hi2s->Init.Format == Philip) 
    {
        I2S->CONTROL.DLY_EN = 1;
        I2S->CONTROL.FMT    = 1;
    }
    else if (hi2s->Init.Format == Left_Justified) 
    {
        I2S->CONTROL.DLY_EN = 0;
        I2S->CONTROL.FMT    = 1;
    }
    else if (hi2s->Init.Format == PCM_Short) 
    {
        I2S->CONTROL.DLY_EN = 0;
        I2S->CONTROL.FMT    = 0;
    }

    /* clock  divider */
    I2S->CLK_DIV = hi2s->Init.Clock_DIV;
    /* Channel length */
    I2S->WS_DIV  = hi2s->Init.Channel_Length - 1;

    I2S_RX_INT_FULL_LEVEL(24);
    I2S_TX_INT_EMPTY_LEVEL(8);
}

/************************************************************************************
 * @fn      i2s_transmit
 *
 * @brief   send an amount of data in blocking mode.
 */
void i2s_transmit(uint32_t *fp_Data, uint32_t fu32_Size)
{
    while (fu32_Size--) 
    {
        while(i2s_int_get_status(I2S_TX_FIFO_FULL));

        I2S->DATA = *fp_Data++;
    }
}

/************************************************************************************
 * @fn      i2s_receive
 *
 * @brief   receive an amount of data in blocking mode.
 */
void i2s_receive(uint32_t *fp_Data, uint32_t fu32_Size)
{
    while (fu32_Size) 
    {
        while(!i2s_int_get_status(I2S_RX_FIFO_EMPTY))
        {
            *fp_Data++ = I2S->DATA;

            fu32_Size--;

            if (fu32_Size == 0) 
            {
                break;
            }
        }
    }
}

/************************************************************************************
 * @fn      i2s_transmit_receive
 *
 * @brief   transmit and receive an amount of data in blocking mode.
 */
void i2s_transmit_receive(uint32_t *fp_TxData, uint32_t *fp_RxData, uint32_t fu32_Size)
{
    uint32_t Tx_Size = fu32_Size;
    uint32_t Rx_Size = fu32_Size;

    while (Tx_Size | Rx_Size) 
    {
        if (Tx_Size) 
        {
            while(!i2s_int_get_status(I2S_TX_FIFO_FULL))
            {
                I2S->DATA = *fp_TxData++;
                
                Tx_Size--;
            }
        }

        if (Rx_Size) 
        {
            while(!i2s_int_get_status(I2S_RX_FIFO_EMPTY))
            {
                *fp_RxData++ = I2S->DATA;

                Rx_Size--;

                if (Rx_Size == 0) 
                {
                    break;
                }
            }
        }
    }
}

/************************************************************************************
 * @fn      i2s_transmit_IT
 *
 * @brief   send an amount of data in interrupt mode.
 *
 * @param   hi2c: I2C handle.
 */
void i2s_transmit_IT(I2S_HandleTypeDef *hi2s, uint32_t *fp_Data, uint32_t fu32_Size)
{
    if (hi2s->b_TxBusy)
        return;

    hi2s->p_TxData    = fp_Data;
    hi2s->u32_TxCount = 0;
    hi2s->u32_TxSize  = fu32_Size;
    hi2s->b_TxBusy    = true;

    while(!i2s_int_get_status(I2S_TX_FIFO_FULL))
    {
        I2S->DATA = hi2s->p_TxData[hi2s->u32_TxCount++];

        if (hi2s->u32_TxCount >= hi2s->u32_TxSize) 
        {
            hi2s->b_TxBusy = false;

            return;
        }
    }

    i2s_int_enable(I2S_TX_FIFO_EMPTY);
}

/************************************************************************************
 * @fn      i2s_receive_IT
 *
 * @brief   receive an amount of data in interrupt mode.
 */
void i2s_receive_IT(I2S_HandleTypeDef *hi2s, uint32_t *fp_Data, uint32_t fu32_Size)
{
    if (hi2s->b_RxBusy)
        return;

    hi2s->p_RxData    = fp_Data;
    hi2s->u32_RxCount = 0;
    hi2s->u32_RxSize  = fu32_Size;
    hi2s->b_RxBusy    = true;

    i2s_int_enable(I2S_RX_FIFO_FULL);
}

/************************************************************************************
 * @fn      i2s_transmit_DMA
 *
 * @brief   send an amount of data in DMA mode.
 */
void i2s_transmit_DMA(void)
{
    /* I2S DMA Enable */
    I2S->DMA_CFG.DMA_TX_EN = 1;
    /* Tx empty dma level */
    I2S_TX_DMA_EMPTY_LEVEL(8);
}

/************************************************************************************
 * @fn      i2s_receive_DMA
 *
 * @brief   receive an amount of data in DMA mode.
 */
void i2s_receive_DMA(void)
{
    /* I2S DMA Enable */
    I2S->DMA_CFG.DMA_RX_EN = 1;
    /* Rx full dma level */
    I2S_RX_DMA_FULL_LEVEL(24);
}

/************************************************************************************
 * @fn      i2s_enable
 *
 * @brief   i2s enable.
 */
void i2s_enable(void)
{
    I2S->CONTROL.I2S_EN = 1;
}

/************************************************************************************
 * @fn      i2s_disable
 *
 * @brief   i2s disable.
 */
void i2s_disable(void)
{
    I2S->CONTROL.I2S_EN = 0;
}

/*********************************************************************
 * @fn      i2s_int_enable
 *
 * @brief   I2s interrupt enable
 */
void i2s_int_enable(enum_I2S_INT_t fe_INT)
{
    I2S->INTE |= fe_INT;
}

/*********************************************************************
 * @fn      i2s_int_disable
 *
 * @brief   I2s interrupt disable
 */
void i2s_int_disable(enum_I2S_INT_t fe_INT)
{
    I2S->INTE &= ~fe_INT;
}

/*********************************************************************
 * @fn      i2s_int_is_enable
 *
 * @brief   I2s interrupt is enable
 */
bool i2s_int_is_enable(enum_I2S_INT_t fe_INT)
{
    return (I2S->INTE & fe_INT) ? true : false;
}

/*********************************************************************
 * @fn      i2s_int_get_status
 *
 * @brief   I2s get interrupt status
 */
bool i2s_int_get_status(enum_I2S_INT_t fe_INT)
{
    return (I2S->Status & fe_INT) ? true : false;
}

/*
    for example:
    
GPIO_InitTypeDef   GPIO_Handle;
DMA_HandleTypeDef  DMA_Chan0;
I2S_HandleTypeDef  I2S_Handle;

uint32_t TxBuffer32[256];
uint32_t RxBuffer32[256];

void main(void)
{
    for (i = 0; i < 256; i++)
    {
        TxBuffer32[i]  = i << 24;
        TxBuffer32[i] |= i << 16;
        TxBuffer32[i] |= i << 8;
        TxBuffer32[i] |= i << 0;
    }

    GPIO_Handle.Pin       = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_Handle.Mode      = GPIO_MODE_AF_PP;
    GPIO_Handle.Pull      = GPIO_PULLUP;
    GPIO_Handle.Alternate = GPIO_FUNCTION_A;
    gpio_init(GPIO_A, &GPIO_Handle);

    __SYSTEM_I2S_CLK_ENABLE();
    
    I2S_Handle.Init.Format         = Philip;
    I2S_Handle.Init.RxDataFormat   = LEFT_DATA_IN_HIGH_BYTE;
    I2S_Handle.Init.Clock_DIV      = 5;
    I2S_Handle.Init.Channel_Length = 16;
    I2S_Handle.Init.WS_InvEN       = INV_DISABLE;
    I2S_Handle.Init.CLK_InvEN      = INV_DISABLE;

    i2s_init(&I2S_Handle);

    NVIC_EnableIRQ(I2S_IRQn);
    NVIC_SetPriority(I2S_IRQn, 0);

    i2s_enable();

    // interrupt mode
    i2s_transmit_IT(&I2S_Handle, (uint32_t *)RxBuffer32, 256);
    i2s_receive_IT(&I2S_Handle, (uint32_t *)RxBuffer32, 256);

    while(I2S_Handle.b_RxBusy);
    while(I2S_Handle.b_TxBusy);

    for (i = 0; i < 256; i++)
    {
        co_printf("%08X \r\n", RxBuffer32[i]);
    }

    // DMA Mode
    __DMA_REQ_ID_I2S_TX(1);
    
    DMA_Chan0.Channel               = DMA_Channel0;
    DMA_Chan0.Init.Data_Flow        = DMA_M2P_DMAC;
    DMA_Chan0.Init.Request_ID       = 1;
    DMA_Chan0.Init.Source_Inc       = DMA_ADDR_INC_INC;
    DMA_Chan0.Init.Desination_Inc   = DMA_ADDR_INC_NO_CHANGE;
    DMA_Chan0.Init.Source_Width     = DMA_TRANSFER_WIDTH_32;
    DMA_Chan0.Init.Desination_Width = DMA_TRANSFER_WIDTH_32;
    dma_init(&DMA_Chan0);
    
	i2s_transmit_DMA();

    dma_start(&DMA_Chan0, (uint32_t)TxBuffer32, (uint32_t)&I2S->DATA, 256, DMA_BURST_LEN_4, DMA_BURST_LEN_4);
    while(!dma_get_tfr_Status(DMA_Channel0));
    dma_clear_tfr_Status(DMA_Channel0);
}

*/
