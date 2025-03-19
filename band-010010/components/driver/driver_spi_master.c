/*
  ******************************************************************************
  * @file    driver_spi_master.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   SPI module driver.
  *          This file provides firmware functions to manage the 
  *          Serial Peripheral Interface (SPI) peripheral
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "driver_spi.h"

/************************************************************************************
 * @fn      spi_master_IRQHandler
 *
 * @brief   Handle SPI interrupt request.
 *
 * @param   hspi: SPI handle.
 */
__attribute__((section("ram_code"))) void spi_master_IRQHandler(SPI_HandleTypeDef *hspi)
{
    /* Tx FIFO Threshold EMPTY */
    if (__SPI_TxFIFO_EMPTY_INT_STATUS(hspi->SPIx))
    {
        while(!__SPI_IS_TxFIFO_FULL(hspi->SPIx))
        {
            switch (hspi->SPIx->CTRL0.DFS)
            {
                case SPI_FRAME_SIZE_4BIT: 
                case SPI_FRAME_SIZE_5BIT: 
                case SPI_FRAME_SIZE_6BIT: 
                case SPI_FRAME_SIZE_7BIT: 
                case SPI_FRAME_SIZE_8BIT: 
                {
                    hspi->SPIx->DR = hspi->p8_TxData[hspi->u32_TxCount++];
                }break;
                
                case SPI_FRAME_SIZE_9BIT: 
                case SPI_FRAME_SIZE_10BIT: 
                case SPI_FRAME_SIZE_11BIT: 
                case SPI_FRAME_SIZE_12BIT: 
                case SPI_FRAME_SIZE_13BIT: 
                case SPI_FRAME_SIZE_14BIT: 
                case SPI_FRAME_SIZE_15BIT: 
                case SPI_FRAME_SIZE_16BIT: 
                {
                    hspi->SPIx->DR = hspi->p16_TxData[hspi->u32_TxCount++];
                }break;
                
                default: break; 
            }

            if (hspi->u32_TxCount >= hspi->u32_TxSize) 
            {
                __SPI_TxFIFO_EMPTY_INT_DISABLE(hspi->SPIx);

                hspi->b_TxBusy = false;

                break;
            }
        }
    }
    /* Rx FIFO Threshold FULL */
    else if (__SPI_RxFIFO_FULL_INT_STATUS(hspi->SPIx))
    {
        while (!__SPI_IS_RxFIFO_EMPTY(hspi->SPIx)) 
        {
            switch (hspi->SPIx->CTRL0.DFS)
            {
                case SPI_FRAME_SIZE_4BIT: 
                case SPI_FRAME_SIZE_5BIT: 
                case SPI_FRAME_SIZE_6BIT: 
                case SPI_FRAME_SIZE_7BIT: 
                case SPI_FRAME_SIZE_8BIT: 
                {
                    hspi->p8_RxData[hspi->u32_RxCount++] = hspi->SPIx->DR;
                }break;
                
                case SPI_FRAME_SIZE_9BIT: 
                case SPI_FRAME_SIZE_10BIT: 
                case SPI_FRAME_SIZE_11BIT: 
                case SPI_FRAME_SIZE_12BIT: 
                case SPI_FRAME_SIZE_13BIT: 
                case SPI_FRAME_SIZE_14BIT: 
                case SPI_FRAME_SIZE_15BIT: 
                case SPI_FRAME_SIZE_16BIT: 
                {
                    hspi->p16_RxData[hspi->u32_RxCount++] = hspi->SPIx->DR;
                }break;
                
                default: break; 
            }

            if (hspi->u32_RxCount >= hspi->u32_RxSize) 
            {
                __SPI_RxFIFO_FULL_INT_DISABLE(hspi->SPIx);

                hspi->b_RxBusy = false;

                break;
            }
        }
    }
}

/************************************************************************************
 * @fn      spi_master_init
 *
 * @brief   Initialize the SPI according to the specified parameters in the struct_SPIInit_t
 *
 * @param   hspi: SPI handle.
 */
void spi_master_init(SPI_HandleTypeDef *hspi)
{
    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Work mode */
    hspi->SPIx->CTRL0.SCPOL = hspi->Init.Work_Mode & 0x2 ? 1 : 0;
    hspi->SPIx->CTRL0.SCPH  = hspi->Init.Work_Mode & 0x1 ? 1 : 0;
    /* Frame Size */
    hspi->SPIx->CTRL0.DFS = hspi->Init.Frame_Size;
    /* BaudRate Prescaler */
    hspi->SPIx->BAUDR = hspi->Init.BaudRate_Prescaler;

    /* FIFO Threshold */
    hspi->SPIx->TXFTLR = hspi->Init.TxFIFOEmpty_Threshold;
    hspi->SPIx->RXFTLR = hspi->Init.RxFIFOFull_Threshold;

    /* Disable all interrupt */
    hspi->SPIx->IMR.MSTIM = 0;
    hspi->SPIx->IMR.TXEIM = 0;
    hspi->SPIx->IMR.TXOIM = 0;
    hspi->SPIx->IMR.RXUIM = 0;
    hspi->SPIx->IMR.RXOIM = 0;
    hspi->SPIx->IMR.RXFIM = 0;

    __SPI_CS_TOGGLE_DISABLE(hspi->SPIx);

    /* CS Set */
    hspi->SPIx->SER  = 1;
}

/************************************************************************************
 * @fn      spi_master_transmit_X1
 *
 * @brief   Send an amount of data in blocking mode.(Standard mode)
 *
 * @param   hspi: SPI handle.
 *          fp_Data: pointer to data buffer
 *          fu32_Size: amount of data to be sent
 */
void spi_master_transmit_X1(SPI_HandleTypeDef *hspi, uint16_t *fp_Data, uint32_t fu32_Size)
{
    uint8_t *lp8_Data;

    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Select RxTx mode */
    __SPI_TMODE_RxTx(hspi->SPIx);
    /* Select Standard mode */
    __SPI_SET_MODE_X1(hspi->SPIx);

    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);

    /* Frame Size <= 8 */
    if (hspi->SPIx->CTRL0.DFS <= SPI_FRAME_SIZE_8BIT) 
    {
        lp8_Data = (uint8_t *)fp_Data;

        while (fu32_Size--) 
        {
            while(__SPI_IS_TxFIFO_FULL(hspi->SPIx));
            /* write data to tx FIFO */
            hspi->SPIx->DR = *lp8_Data++;
        }
    }
    else 
    {
        while (fu32_Size--) 
        {
            while(__SPI_IS_TxFIFO_FULL(hspi->SPIx));
            /* write data to tx FIFO */
            hspi->SPIx->DR = *fp_Data++;
        }
    }

    while(!__SPI_IS_TxFIFO_EMPTY(hspi->SPIx));
    while(__SPI_IS_BUSY(hspi->SPIx));
}

/************************************************************************************
 * @fn      spi_master_transmit_X1_IT
 *
 * @brief   Send an amount of data in interrupt mode.(Standard mode)
 *
 * @param   hspi: SPI handle.
 *          fp_Data: pointer to data buffer
 *          fu32_Size: amount of data to be sent
 */
void spi_master_transmit_X1_IT(SPI_HandleTypeDef *hspi, uint16_t *fp_Data, uint32_t fu32_Size)
{
    if (hspi->b_TxBusy)
        return;

    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Select RxTx mode */
    __SPI_TMODE_RxTx(hspi->SPIx);

    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);

    hspi->u32_TxSize  = fu32_Size;
    hspi->u32_TxCount = 0;
    hspi->b_TxBusy    = true;
    hspi->p8_TxData  = (uint8_t *)fp_Data;
    hspi->p16_TxData = fp_Data;

    while(!__SPI_IS_TxFIFO_FULL(hspi->SPIx))
    {
        hspi->SPIx->DR = (hspi->SPIx->CTRL0.DFS <= SPI_FRAME_SIZE_8BIT) ? hspi->p8_TxData[hspi->u32_TxCount++] : hspi->p16_TxData[hspi->u32_TxCount++];

        if (hspi->u32_TxCount >= fu32_Size) 
        {
            hspi->b_TxBusy = false;
            return;
        }
    }

    /* TxFIFO empty interrupt enable */
    __SPI_TxFIFO_EMPTY_INT_ENABLE(hspi->SPIx);
}

/************************************************************************************
 * @fn      spi_master_transmit_X1_DMA
 *
 * @brief   Send an amount of data in DMA mode.(Standard mode)
 *
 * @param   hspi: SPI handle.
 */
void spi_master_transmit_X1_DMA(SPI_HandleTypeDef *hspi)
{
    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Select RxTx mode */
    __SPI_TMODE_RxTx(hspi->SPIx);
    /* Select Standard mode */
    __SPI_SET_MODE_X1(hspi->SPIx);

    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);

    /* DMA Config */
    __SPI_DMA_TX_ENABLE(hspi->SPIx);
    __SPI_DMA_TX_LEVEL(hspi->SPIx, hspi->Init.TxFIFOEmpty_Threshold);
}

/************************************************************************************
 * @fn      spi_master_receive_X1
 *
 * @brief   Receive an amount of data in blocking mode.(Standard mode)
 *
 * @param   hspi: SPI handle.
 *          fp_Data: pointer to data buffer
 *          fu32_Size: amount of data to be Receive
 */
void spi_master_receive_X1(SPI_HandleTypeDef *hspi, uint16_t *fp_Data, uint32_t fu32_Size)
{
    uint8_t *lp8_Data;
    uint32_t RxCount = fu32_Size;
    uint32_t TxCount = fu32_Size;

    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Select RxTx mode */
    __SPI_TMODE_RxTx(hspi->SPIx);
    /* Select Standard mode */
    __SPI_SET_MODE_X1(hspi->SPIx);

    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);

    /* Frame Size <= 8 */
    if (hspi->SPIx->CTRL0.DFS <= SPI_FRAME_SIZE_8BIT) 
    {
        lp8_Data = (uint8_t *)fp_Data;

        while (RxCount) 
        {
            /* write data to tx FIFO */
            if (!__SPI_IS_TxFIFO_FULL(hspi->SPIx) && TxCount) 
            {
                hspi->SPIx->DR = 0x00;
                TxCount--;
            }

            while(!__SPI_IS_RxFIFO_EMPTY(hspi->SPIx))
            {
                *lp8_Data++ = hspi->SPIx->DR;

                RxCount--;
            }
        }
    }
    else 
    {
        while (RxCount) 
        {
            /* write data to tx FIFO */
            if (!__SPI_IS_TxFIFO_FULL(hspi->SPIx) && TxCount) 
            {
                hspi->SPIx->DR = 0x0000;
                TxCount--;
            }

            while(!__SPI_IS_RxFIFO_EMPTY(hspi->SPIx))
            {
                *fp_Data++ = hspi->SPIx->DR;

                RxCount--;
            }
        }
    }

    while(__SPI_IS_BUSY(hspi->SPIx));
}

/************************************************************************************
 * @fn      spi_master_readflash_X1
 *
 * @brief   ReadFlash mode. First send the command address, And then receive the data
 *
 * @param   hspi: SPI handle.
 *          fp_CMD_ADDR    : Command and Address.
 *          fu32_CMDLegnth : Command and Address Length.
 *          fp_Data    : pointer to data buffer.
 *          fu16_Size  : amount of data to be Receive.
 */
void spi_master_readflash_X1(SPI_HandleTypeDef *hspi, uint16_t *fp_CMD_ADDR, uint32_t fu32_CMDLegnth, uint16_t *fp_Data, uint16_t fu16_Size)
{
    uint8_t *lp8_Data;

    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Select flash read mode */
    __SPI_TMODE_FLASH_READ(hspi->SPIx);

    /* Select Standard mode */
    __SPI_SET_MODE_X1(hspi->SPIx);

    /* Config receive data size */ 
    hspi->SPIx->CTRL1.NDF = fu16_Size - 1;

    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);

    /* Frame Size <= 8 */
    if (hspi->SPIx->CTRL0.DFS <= SPI_FRAME_SIZE_8BIT) 
    {
        lp8_Data = (uint8_t *)fp_CMD_ADDR;

        while (fu32_CMDLegnth--) 
        {
            /* write data to tx FIFO */
            hspi->SPIx->DR = *lp8_Data++;
        }

        lp8_Data = (uint8_t *)fp_Data;

        while(!__SPI_IS_TxFIFO_EMPTY(hspi->SPIx));
        while (__SPI_IS_BUSY(hspi->SPIx)) 
        {
            if (__SPI_IS_RxFIFO_NOT_EMPTY(hspi->SPIx)) 
            {
                *lp8_Data++ = hspi->SPIx->DR;
            }
        }

        while (__SPI_IS_RxFIFO_NOT_EMPTY(hspi->SPIx)) 
        {
            *lp8_Data++ = hspi->SPIx->DR;
        }
    }
    else 
    {
        while (fu32_CMDLegnth--) 
        {
            /* write data to tx FIFO */
            hspi->SPIx->DR = *fp_CMD_ADDR++;
        }

        while(!__SPI_IS_TxFIFO_EMPTY(hspi->SPIx));
        while (__SPI_IS_BUSY(hspi->SPIx)) 
        {
            if (__SPI_IS_RxFIFO_NOT_EMPTY(hspi->SPIx)) 
            {
                *fp_Data++ = hspi->SPIx->DR;
            }
        }

        while (__SPI_IS_RxFIFO_NOT_EMPTY(hspi->SPIx)) 
        {
            *fp_Data++ = hspi->SPIx->DR;
        }
    }
}

/************************************************************************************
 * @fn      spi_master_readflash_X1_IT
 *
 * @brief   ReadFlash mode. First send the command address, And then receive the data
 *
 * @param   hspi: SPI handle.
 *          fp_CMD_ADDR   : Command and Address.
 *          fu32_CMDLegnth: Command and Address Length.
 *          fp_RxData  : pointer to data buffer.
 *          fu16_Size  : amount of data to be Receive.
 */
void spi_master_readflash_X1_IT(SPI_HandleTypeDef *hspi, uint16_t *fp_CMD_ADDR, uint32_t fu32_CMDLegnth, uint16_t *fp_Data, uint16_t fu16_Size)
{
    uint8_t *lp8_Data;

    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Select flash read mode */
    __SPI_TMODE_FLASH_READ(hspi->SPIx);
    /* Select Standard mode */
    __SPI_SET_MODE_X1(hspi->SPIx);

    /* Config receive data size */ 
    hspi->SPIx->CTRL1.NDF = fu16_Size - 1;

    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);


    hspi->u32_RxSize  = fu16_Size;
    hspi->u32_RxCount = 0;
    hspi->b_RxBusy    = true;
    hspi->p8_RxData  = (uint8_t *)fp_Data;
    hspi->p16_RxData = fp_Data;

    /* Frame Size <= 8 */
    if (hspi->SPIx->CTRL0.DFS <= SPI_FRAME_SIZE_8BIT) 
    {
        lp8_Data = (uint8_t *)fp_CMD_ADDR;

        while (fu32_CMDLegnth--) 
        {
            /* write data to tx FIFO */
            hspi->SPIx->DR = *lp8_Data++;
        }
    }
    else 
    {
        while (fu32_CMDLegnth--) 
        {
            /* write data to tx FIFO */
            hspi->SPIx->DR = *fp_CMD_ADDR++;
        }
    }

    /* RxFIFO full interrupt enable */
    __SPI_RxFIFO_FULL_INT_ENABLE(hspi->SPIx);
}

/************************************************************************************
 * @fn      spi_master_readflash_X1_DMA
 *
 * @brief   ReadFlash mode. First send the command address, And then receive the data
 *
 * @param   hspi: SPI handle.
 *          fp_CMD_ADDR   : Command and Address.
 *          fu32_CMDLegnth: Command and Address Length.
 *          fu16_Size  : amount of data to be Receive.
 */
void spi_master_readflash_X1_DMA(SPI_HandleTypeDef *hspi, uint16_t *fp_CMD_ADDR, uint32_t fu32_CMDLegnth, uint16_t fu16_Size)
{
    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Select flash read mode */
    __SPI_TMODE_FLASH_READ(hspi->SPIx);
    /* Select Standard mode */
    __SPI_SET_MODE_X1(hspi->SPIx);

    /* DMA Config */
    __SPI_DMA_RX_ENABLE(hspi->SPIx);
    __SPI_DMA_RX_LEVEL(hspi->SPIx, hspi->Init.RxFIFOFull_Threshold);

    /* Config receive data size */ 
    hspi->SPIx->CTRL1.NDF = fu16_Size - 1;

    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);

    while (fu32_CMDLegnth--) 
    {
        /* write data to tx FIFO */
        hspi->SPIx->DR = *fp_CMD_ADDR++;
    }
}

/************************************************************************************
 * @fn      spi_master_transmit_X2X4
 *
 * @brief   Send an amount of data in blocking mode.(Dual、Quad mode)
 *
 * @param   hspi: SPI handle.
 *          fp_Data: pointer to data buffer
 *          fu32_Size: amount of data to be sent
 */
void spi_master_transmit_X2X4(SPI_HandleTypeDef *hspi, uint16_t *fp_Data, uint32_t fu32_Size)
{
    uint8_t *lp8_Data;

    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* config Mult Wire Transfer parameters */
    spi_master_MultWireConfig(hspi, Wire_Write);

    /* Select Only Tx mode */
    __SPI_TMODE_Tx_ONLY(hspi->SPIx);
    
    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);

    /* Frame Size <= 8 */
    if (hspi->SPIx->CTRL0.DFS <= SPI_FRAME_SIZE_8BIT) 
    {
        lp8_Data = (uint8_t *)fp_Data;

        while (fu32_Size--) 
        {
            while(__SPI_IS_TxFIFO_FULL(hspi->SPIx));
            /* write data to tx FIFO */
            hspi->SPIx->DR = *lp8_Data++;
        }
    }
    else 
	{
        while (fu32_Size--) 
        {
            while(__SPI_IS_TxFIFO_FULL(hspi->SPIx));
            /* write data to tx FIFO */
            hspi->SPIx->DR = *fp_Data++;
        }
    }

    while(__SPI_IS_BUSY(hspi->SPIx));
}

/************************************************************************************
 * @fn      spi_master_transmit_X2X4_IT
 *
 * @brief   Send an amount of data in interrupt mode.(Dual、Quad mode)
 *
 * @param   hspi: SPI handle.
 *          fp_Data: pointer to data buffer
 *          fu32_Size: amount of data to be sent
 */
void spi_master_transmit_X2X4_IT(SPI_HandleTypeDef *hspi, uint16_t *fp_Data, uint32_t fu32_Size)
{
    uint8_t *lp8_Data;

    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* config Mult Wire Transfer parameters */
    spi_master_MultWireConfig(hspi, Wire_Write);

    /* Select Only Tx mode */
    __SPI_TMODE_Tx_ONLY(hspi->SPIx);
    
    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);

    hspi->u32_TxSize  = fu32_Size;
    hspi->u32_TxCount = 0;
    hspi->b_TxBusy    = true;
    hspi->p8_TxData   = (uint8_t *)fp_Data;
    hspi->p16_TxData  = fp_Data;

    /* Frame Size <= 8 */
    if (hspi->SPIx->CTRL0.DFS <= SPI_FRAME_SIZE_8BIT) 
    {
        lp8_Data = (uint8_t *)fp_Data;

        while(!__SPI_IS_TxFIFO_FULL(hspi->SPIx))
        {
            hspi->SPIx->DR = lp8_Data[hspi->u32_TxCount++];

            if (hspi->u32_TxCount >= fu32_Size)
            {
                hspi->b_TxBusy = false;
                return;
            }
        }
    }
    else 
    {
        while(!__SPI_IS_TxFIFO_FULL(hspi->SPIx))
        {
            hspi->SPIx->DR = fp_Data[hspi->u32_TxCount++];

            if (hspi->u32_TxCount >= fu32_Size) 
            {
                hspi->b_TxBusy = false;
                return;
            }
        }
    }

    /* TxFIFO empty interrupt enable */
    __SPI_TxFIFO_EMPTY_INT_ENABLE(hspi->SPIx);
}

/************************************************************************************
 * @fn      spi_master_transmit_X2X4_IT
 *
 * @brief   Send an amount of data in DMA mode.(Dual、Quad mode)
 *
 * @param   hspi: SPI handle.
 */
void spi_master_transmit_X2X4_DMA(SPI_HandleTypeDef *hspi)
{
    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* config Mult Wire Transfer parameters */
    spi_master_MultWireConfig(hspi, Wire_Write);

    /* Select Only Tx mode */
    __SPI_TMODE_Tx_ONLY(hspi->SPIx);
    
    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);

    /* DMA Config */
    __SPI_DMA_TX_ENABLE(hspi->SPIx);
    __SPI_DMA_TX_LEVEL(hspi->SPIx, hspi->Init.TxFIFOEmpty_Threshold);
}

/************************************************************************************
 * @fn      spi_master_receive_X2X4
 *
 * @brief   Receive an amount of data in blocking mode.(Dual、Quad mode)
 *
 * @param   hspi: SPI handle.
 *          fp_Data: pointer to data buffer
 *          fu16_Size: amount of data to be Receive
 */
void spi_master_receive_X2X4(SPI_HandleTypeDef *hspi, uint16_t *fp_Data, uint16_t fu16_Size)
{
    uint8_t *lp8_Data;

    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Config receive data size */ 
    hspi->u32_RxSize = fu16_Size - 1;

    /* config Mult Wire Transfer parameters */
    spi_master_MultWireConfig(hspi, Wire_Read);

    /* Frame Size <= 8 */
    if (hspi->SPIx->CTRL0.DFS <= SPI_FRAME_SIZE_8BIT) 
    {
        lp8_Data = (uint8_t *)fp_Data;

        while(!__SPI_IS_TxFIFO_EMPTY(hspi->SPIx));
        while (__SPI_IS_BUSY(hspi->SPIx)) 
        {
            if (__SPI_IS_RxFIFO_NOT_EMPTY(hspi->SPIx)) 
            {
                *lp8_Data++ = hspi->SPIx->DR;
            }
        }

        while (__SPI_IS_RxFIFO_NOT_EMPTY(hspi->SPIx)) 
        {
            *lp8_Data++ = hspi->SPIx->DR;
        }
    }
    else 
    {
        while(!__SPI_IS_TxFIFO_EMPTY(hspi->SPIx));
        while (__SPI_IS_BUSY(hspi->SPIx)) 
        {
            if (__SPI_IS_RxFIFO_NOT_EMPTY(hspi->SPIx)) 
            {
                *fp_Data++ = hspi->SPIx->DR;
            }
        }

        while (__SPI_IS_RxFIFO_NOT_EMPTY(hspi->SPIx)) 
        {
            *fp_Data++ = hspi->SPIx->DR;
        }
    }
}

/************************************************************************************
 * @fn      spi_master_receive_X2X4_IT
 *
 * @brief   Receive an amount of data in interrupt mode.(Dual、Quad mode)
 *
 * @param   hspi: SPI handle.
 *          fp_Data: pointer to data buffer
 *          fu16_Size: amount of data to be Receive
 */
void spi_master_receive_X2X4_IT(SPI_HandleTypeDef *hspi, uint16_t *fp_Data, uint16_t fu16_Size)
{
    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Config receive data size */ 
    hspi->u32_RxSize = fu16_Size - 1;

    /* config Mult Wire Transfer parameters */
    spi_master_MultWireConfig(hspi, Wire_Read);

    hspi->u32_RxSize  = fu16_Size;
    hspi->u32_RxCount = 0;
    hspi->b_RxBusy    = true;
    hspi->p8_RxData  = (uint8_t *)fp_Data;
    hspi->p16_RxData = fp_Data;
    
    /* RxFIFO full interrupt enable */
    __SPI_RxFIFO_FULL_INT_ENABLE(hspi->SPIx);
}

/************************************************************************************
 * @fn      spi_master_receive_X2X4_DMA
 *
 * @brief   Receive an amount of data in DMA mode.(Dual、Quad mode)
 *
 * @param   hspi: SPI handle.
 *          fp_Data: pointer to data buffer
 *          fu16_Size: amount of data to be Receive
 */
void spi_master_receive_X2X4_DMA(SPI_HandleTypeDef *hspi, uint16_t fu16_Size)
{
    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* DMA Config */
    __SPI_DMA_RX_ENABLE(hspi->SPIx);
    __SPI_DMA_RX_LEVEL(hspi->SPIx, hspi->Init.RxFIFOFull_Threshold);

    /* Config receive data size */ 
    hspi->u32_RxSize = fu16_Size - 1;

    /* config Mult Wire Transfer parameters */
    spi_master_MultWireConfig(hspi, Wire_Read);
}

/************************************************************************************
 * @fn      spi_master_MultWireConfig
 *
 * @brief   config Mult Wire Transfer parameters.(Dual、Quad mode)
 *
 * @param   hspi: SPI handle.
 */
void spi_master_MultWireConfig(SPI_HandleTypeDef *hspi, enum_Wire_Type_t fe_type)
{
    uint8_t lu8_Buffer[8];
    uint8_t lu8_Count = 0, i = 0;

    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    if (fe_type == Wire_Write) 
    {
        /* Frame Size */
        __SPI_DATA_FRAME_SIZE(hspi->SPIx, SPI_FRAME_SIZE_8BIT);
        /* Select RxTx mode */
        __SPI_TMODE_RxTx(hspi->SPIx);
        /* Select Standard mode */
        __SPI_SET_MODE_X1(hspi->SPIx);

        __SPI_ENABLE(hspi->SPIx);

        if (hspi->MultWireParam.InstructLength == INST_16BIT) 
        {
            lu8_Buffer[lu8_Count++] = hspi->MultWireParam.Instruct >> 8 & 0xFF;
            lu8_Buffer[lu8_Count++] = hspi->MultWireParam.Instruct & 0xFF;
        }
        else if (hspi->MultWireParam.InstructLength == INST_8BIT) 
        {
            lu8_Buffer[lu8_Count++] = hspi->MultWireParam.Instruct & 0xFF;
        }

        if (hspi->MultWireParam.AddressLength >= ADDR_32BIT)
        {
            lu8_Buffer[lu8_Count++] = hspi->MultWireParam.Address >> 24 & 0xFF;
        }
        if (hspi->MultWireParam.AddressLength >= ADDR_24BIT) 
        {
            lu8_Buffer[lu8_Count++] = hspi->MultWireParam.Address >> 16 & 0xFF;
        }
        if (hspi->MultWireParam.AddressLength >= ADDR_16BIT) 
        {
            lu8_Buffer[lu8_Count++] = hspi->MultWireParam.Address >> 8 & 0xFF;
        }
        if (hspi->MultWireParam.AddressLength >= ADDR_8BIT) 
        {
            lu8_Buffer[lu8_Count++] = hspi->MultWireParam.Address & 0xFF;
        }

        while (lu8_Count--) 
        {
            /* write data to tx FIFO */
            hspi->SPIx->DR = lu8_Buffer[i++];
        }

        while(!__SPI_IS_TxFIFO_EMPTY(hspi->SPIx));
        while(__SPI_IS_BUSY(hspi->SPIx));

        __SPI_DISABLE(hspi->SPIx);

        /* Frame Size */
        __SPI_DATA_FRAME_SIZE(hspi->SPIx, hspi->Init.Frame_Size);

        if (hspi->MultWireParam.Wire_X2X4 == Wire_X2) 
        {   /* Select Dual mode */
            __SPI_SET_MODE_X2(hspi->SPIx);
        }
        else if (hspi->MultWireParam.Wire_X2X4 == Wire_X4) 
        {   /* Select Quad mode */
            __SPI_SET_MODE_X4(hspi->SPIx);
        }

        hspi->SPIx->CTRL2.INST_L      = (hspi->SPIx->CTRL0.DFS <= SPI_FRAME_SIZE_8BIT) ? INST_8BIT : INST_16BIT;
        hspi->SPIx->CTRL2.WAIT_CYCLES = 0;
        hspi->SPIx->CTRL2.TRANS_TYPE  = INST_ADDR_XX;
        hspi->SPIx->CTRL2.ADDR_L      = 0;
    }
    else 
    {
        if (hspi->MultWireParam.Wire_X2X4 == Wire_X2) 
        {   /* Select Dual mode */
            __SPI_SET_MODE_X2(hspi->SPIx);
        }
        else if (hspi->MultWireParam.Wire_X2X4 == Wire_X4) 
        {   /* Select Quad mode */
            __SPI_SET_MODE_X4(hspi->SPIx);
        }

        /* Select Only Rx mode */
        __SPI_TMODE_Rx_ONLY(hspi->SPIx);
    
        /* config Transfer Type、Instruct Length、Address Length */
        hspi->SPIx->CTRL2.TRANS_TYPE  = INST_ADDR_X1;
        hspi->SPIx->CTRL2.INST_L      = hspi->MultWireParam.InstructLength;
        hspi->SPIx->CTRL2.ADDR_L      = hspi->MultWireParam.AddressLength;
        hspi->SPIx->CTRL2.WAIT_CYCLES = hspi->MultWireParam.ReceiveWaitCycles;

        hspi->SPIx->CTRL1.NDF = hspi->u32_RxSize;

        /* Enable SPI */
        __SPI_ENABLE(hspi->SPIx);

        /* write Instruct */
        if (hspi->MultWireParam.InstructLength > INST_0BIT) 
        {
            hspi->SPIx->DR = hspi->MultWireParam.Instruct;
        }

        /* write address */
        if (hspi->MultWireParam.AddressLength > ADDR_24BIT) 
        {
            hspi->SPIx->DR = (hspi->MultWireParam.Address >> 16) & 0xFFFF;
            hspi->SPIx->DR =  hspi->MultWireParam.Address & 0xFFFF;
        }
        else if (hspi->MultWireParam.AddressLength > ADDR_16BIT) 
        {
            hspi->SPIx->DR = (hspi->MultWireParam.Address >> 8) & 0xFFFF;
            hspi->SPIx->DR =  hspi->MultWireParam.Address & 0xFF;
        }
        else if (hspi->MultWireParam.AddressLength > ADDR_0BIT)
        {
            hspi->SPIx->DR = hspi->MultWireParam.Address & 0xFFFF;
        }
    }
}

/*
    for example:

    uint16_t  TxBuffer16[256];
    uint16_t  RxBuffer16[256];
    uint8_t   CMDBuffer[256];

    SPI_HandleTypeDef  SPI0_Handle;
    DMA_HandleTypeDef  DMA_Chan0;

void spim0_isr(void)
{
    spi_master_IRQHandler(&SPI0_Handle);
}

void main()
{
    system_regs->misc.ssim0_apb_clk_en = 1;
    system_regs->clk_gate.ssim0_clk_en = 1;

    system_regs->clk_gate.dma_clk_en   = 1;

    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_SSIM0_IO0);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_SSIM0_IO1);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_SSIM0_IO2);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_SSIM0_IO3);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_SSIM0_CLK);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA7_FUNC_SSIM0_CSN);


    SPI0_Handle.SPIx                       = SPIM0;
    SPI0_Handle.Init.Work_Mode             = SPI_WORK_MODE_0;
    SPI0_Handle.Init.Frame_Size            = SPI_FRAME_SIZE_16BIT;
    SPI0_Handle.Init.BaudRate_Prescaler    = 10;
    SPI0_Handle.Init.TxFIFOEmpty_Threshold = 20;
    SPI0_Handle.Init.RxFIFOFull_Threshold  = 0;

    spi_master_init(&SPI0_Handle);

    NVIC_EnableIRQ(SPIM0_IRQn);
    NVIC_SetPriority(SPIM0_IRQn, 0);

    for (i = 0; i < 256; i++)
    {
        TxBuffer16[i]  = i;
        TxBuffer16[i] |= i << 8;
    }

    //---------------------------------------//
    //     transmit Test (Standard mode)     //
    //---------------------------------------//
    spi_master_transmit_X1_IT(&SPI0_Handle, TxBuffer16, 256);
    while(SPI0_Handle.b_TxBusy);

    //---------------------------------------//
    //     receive Test (Standard mode)      //
    //---------------------------------------//
    // 读取flash模式，先发指令地址，然后开始读取。
    CMDBuffer[0] = 0xAA11;
    CMDBuffer[1] = 0x2233;
    spi_master_readflash_X1(&SPI0_Handle, CMDBuffer, 2, RxBuffer16, 256);
    while(__SPI_IS_BUSY(SPI0_Handle.SPIx));

    //------------------------------------------//
    //     transmit Test (Dual、Quad mode)      //
    //------------------------------------------//
    // 先通过单线模式发指令地址，然后多线模式开始传输数据。
    SPI0_Handle.MultWireParam.Wire_X2X4      = Wire_X4;
    SPI0_Handle.MultWireParam.InstructLength = INST_8BIT;
    SPI0_Handle.MultWireParam.Instruct       = 0xAA;
    SPI0_Handle.MultWireParam.AddressLength  = ADDR_24BIT;
    SPI0_Handle.MultWireParam.Address        = 0x112233;
    spi_master_transmit_X2X4(&SPI0_Handle, TxBuffer16, 256);

    // USE DMA mode
    system_regs->dma_req[0] |= 1 << 20;

    DMA_Chan0.Channel               = DMA_Channel0;
    DMA_Chan0.Init.Data_Flow        = DMA_M2P_DMAC;
    DMA_Chan0.Init.Request_ID       = 1;
    DMA_Chan0.Init.Source_Inc       = DMA_ADDR_INC_INC;
    DMA_Chan0.Init.Desination_Inc   = DMA_ADDR_INC_NO_CHANGE;
    DMA_Chan0.Init.Source_Width     = DMA_TRANSFER_WIDTH_16;
    DMA_Chan0.Init.Desination_Width = DMA_TRANSFER_WIDTH_16;
    dma_init(&DMA_Chan0);

    spi_master_transmit_X2X4_DMA(&SPI0_Handle);

    dma_start(&DMA_Chan0, (uint32_t)TxBuffer16, (uint32_t)&SPI0_Handle.SPIx->DR, 256, DMA_BURST_LEN_1, DMA_BURST_LEN_1);
    while(!dma_get_tfr_Status(DMA_Channel0));
    dma_clear_tfr_Status(DMA_Channel0);

    //------------------------------------------//
    //     receive Test (Dual、Quad mode)       //
    //------------------------------------------//
    // 先通过单线模式发指令，地址，dummy周期，然后多线模式开始传输数据。
    SPI0_Handle.MultWireParam.Wire_X2X4         = Wire_X4;
    SPI0_Handle.MultWireParam.ReceiveWaitCycles = 8;
    SPI0_Handle.MultWireParam.InstructLength    = INST_8BIT;
    SPI0_Handle.MultWireParam.Instruct          = 0xAA;
    SPI0_Handle.MultWireParam.AddressLength     = ADDR_24BIT;
    SPI0_Handle.MultWireParam.Address           = 0x112233;
    spi_master_receive_X2X4(&SPI0_Handle, RxBuffer16, 256);
}

*/
