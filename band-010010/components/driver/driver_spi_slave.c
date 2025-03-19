/*
  ******************************************************************************
  * @file    driver_spi_slave.c
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
 * @fn      spi_slave_IRQHandler
 *
 * @brief   Handle SPI interrupt request.
 *
 * @param   hspi: SPI handle.
 */
__attribute__((section("ram_code"))) void spi_slave_IRQHandler(SPI_HandleTypeDef *hspi)
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
                case SPI_FRAME_SIZE_8BIT: hspi->SPIx->DR = hspi->p8_TxData[hspi->u32_TxCount++]; break;
                
                case SPI_FRAME_SIZE_9BIT: 
                case SPI_FRAME_SIZE_10BIT: 
                case SPI_FRAME_SIZE_11BIT: 
                case SPI_FRAME_SIZE_12BIT: 
                case SPI_FRAME_SIZE_13BIT: 
                case SPI_FRAME_SIZE_14BIT: 
                case SPI_FRAME_SIZE_15BIT: 
                case SPI_FRAME_SIZE_16BIT: hspi->SPIx->DR = hspi->p16_TxData[hspi->u32_TxCount++]; break;

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
                case SPI_FRAME_SIZE_8BIT: hspi->p8_RxData[hspi->u32_RxCount++] = hspi->SPIx->DR; break;
                
                case SPI_FRAME_SIZE_9BIT: 
                case SPI_FRAME_SIZE_10BIT: 
                case SPI_FRAME_SIZE_11BIT: 
                case SPI_FRAME_SIZE_12BIT: 
                case SPI_FRAME_SIZE_13BIT: 
                case SPI_FRAME_SIZE_14BIT: 
                case SPI_FRAME_SIZE_15BIT: 
                case SPI_FRAME_SIZE_16BIT: hspi->p16_RxData[hspi->u32_RxCount++] = hspi->SPIx->DR; break;

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
 * @fn      spi_slave_init
 *
 * @brief   Initialize the SPI according to the specified parameters in the struct_SPIInit_t
 *
 * @param   hspi: SPI handle.
 */
void spi_slave_init(SPI_HandleTypeDef *hspi)
{
    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Work mode */
    hspi->SPIx->CTRL0.SCPOL = hspi->Init.Work_Mode & 0x2 ? 1 : 0;
    hspi->SPIx->CTRL0.SCPH  = hspi->Init.Work_Mode & 0x1 ? 1 : 0;
    /* Frame Size */
    hspi->SPIx->CTRL0.DFS = hspi->Init.Frame_Size;
    /* slave output enable */
    hspi->SPIx->CTRL0.SLV_OE = 0;

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
 * @fn      spi_slave_transmit_X1
 *
 * @brief   Send an amount of data in blocking mode.(Standard mode)
 *
 * @param   hspi: SPI handle.
 *          fp_Data: pointer to data buffer
 *          fu32_Size: amount of data to be sent
 */
void spi_slave_transmit_X1(SPI_HandleTypeDef *hspi, uint16_t *fp_Data, uint32_t fu32_Size)
{
    uint8_t *lp8_Data;
        
    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

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
 * @fn      spi_slave_transmit_X1_IT
 *
 * @brief   Send an amount of data in interrupt mode.(Standard mode)
 *
 * @param   hspi: SPI handle.
 *          fp_Data: pointer to data buffer
 *          fu32_Size: amount of data to be sent
 */
void spi_slave_transmit_X1_IT(SPI_HandleTypeDef *hspi, uint16_t *fp_Data, uint32_t fu32_Size)
{
    if (hspi->b_TxBusy)
        return;

    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Select Only Tx mode */
    __SPI_TMODE_Tx_ONLY(hspi->SPIx);

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
void spi_slave_transmit_X1_DMA(SPI_HandleTypeDef *hspi)
{
    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Select Only Tx mode */
    __SPI_TMODE_Tx_ONLY(hspi->SPIx);

    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);

    /* DMA Config */
    __SPI_DMA_TX_ENABLE(hspi->SPIx);
    __SPI_DMA_TX_LEVEL(hspi->SPIx, hspi->Init.TxFIFOEmpty_Threshold);
}

/************************************************************************************
 * @fn      spi_slave_receive_X1
 *
 * @brief   Receive an amount of data in blocking mode.(Standard mode)
 *
 * @param   hspi: SPI handle.
 *          fp_Data: pointer to data buffer
 *          fu32_Size: amount of data to be Receive
 */
void spi_slave_receive_X1(SPI_HandleTypeDef *hspi, uint16_t *fp_Data, uint32_t fu32_Size)
{
    uint8_t *lp8_Data;

    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Select Only Rx mode */
    __SPI_TMODE_Rx_ONLY(hspi->SPIx);

    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);

    /* Frame Size <= 8 */
    if (hspi->SPIx->CTRL0.DFS <= SPI_FRAME_SIZE_8BIT) 
    {
        lp8_Data = (uint8_t *)fp_Data;
        
        while (fu32_Size) 
        {
            while(!__SPI_IS_RxFIFO_EMPTY(hspi->SPIx))
            {
                *lp8_Data++ = hspi->SPIx->DR;

                fu32_Size--;
            }
        }
    }
    else 
    {
        while (fu32_Size) 
        {
            while(!__SPI_IS_RxFIFO_EMPTY(hspi->SPIx))
            {
                *fp_Data++ = hspi->SPIx->DR;
                
                fu32_Size--;
            }
        }
    }

    while(__SPI_IS_BUSY(hspi->SPIx));
}

/************************************************************************************
 * @fn      spi_slave_receive_X1_IT
 *
 * @brief   Receive an amount of data in interrupt mode.(Standard mode)
 *
 * @param   hspi: SPI handle.
 *          fp_Data: pointer to data buffer
 *          fu32_Size: amount of data to be Receive
 */
void spi_slave_receive_X1_IT(SPI_HandleTypeDef *hspi, uint16_t *fp_Data, uint32_t fu32_Size)
{
    if (hspi->b_RxBusy)
        return;

    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Select Only Rx mode */
    __SPI_TMODE_Rx_ONLY(hspi->SPIx);

    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);

    hspi->u32_RxSize  = fu32_Size;
    hspi->u32_RxCount = 0;
    hspi->b_RxBusy    = true;
    hspi->p8_RxData  = (uint8_t *)fp_Data;
    hspi->p16_RxData = fp_Data;

    /* RxFIFO full interrupt enable */
    __SPI_RxFIFO_FULL_INT_ENABLE(hspi->SPIx);
}

/************************************************************************************
 * @fn      spi_slave_receive_X1_DMA
 *
 * @brief   Receive an amount of data in interrupt mode.(Standard mode)
 *
 * @param   hspi: SPI handle.
 *          fp_Data: pointer to data buffer
 *          fu32_Size: amount of data to be Receive
 */
void spi_slave_receive_X1_DMA(SPI_HandleTypeDef *hspi)
{
    /* Disable SPI, reset FIFO */
    __SPI_DISABLE(hspi->SPIx);

    /* Select Only Rx mode */
    __SPI_TMODE_Rx_ONLY(hspi->SPIx);

    /* Enable SPI */
    __SPI_ENABLE(hspi->SPIx);

    /* DMA Config */
    __SPI_DMA_RX_ENABLE(hspi->SPIx);
    __SPI_DMA_RX_LEVEL(hspi->SPIx, hspi->Init.RxFIFOFull_Threshold);
}

/*
    for example:

    uint8_t  TxBuffer8[256];
    uint8_t  RxBuffer8[256];
    uint16_t TxBuffer16[256];
    uint16_t RxBuffer16[256];

    SPI_HandleTypeDef  SPIS_Handle;
    DMA_HandleTypeDef  DMA_Chan0;

void spis_isr(void)
{
	spi_slave_IRQHandler(&SPIS_Handle);
}

void main(void)
{
    system_regs->misc.ssis_apb_clk_en = 1;
    system_regs->clk_gate.ssis_clk_en = 1;
    system_regs->clk_gate.dma_clk_en  = 1;

    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_SSIS_MOSI);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_SSIS_MISO);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_SSIS_CLK);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_SSIS_CSN);

    SPIS_Handle.SPIx                       = SPIS0;
    SPIS_Handle.Init.Work_Mode             = SPI_WORK_MODE_0;
    SPIS_Handle.Init.Frame_Size            = SPI_FRAME_SIZE_8BIT;
    SPIS_Handle.Init.TxFIFOEmpty_Threshold = 8;
    SPIS_Handle.Init.RxFIFOFull_Threshold  = 0;

    spi_slave_init(&SPIS_Handle);

    NVIC_EnableIRQ(SPIS_IRQn);
    NVIC_SetPriority(SPIS_IRQn, 0);

    for (i = 0; i < 256; i++)
    {
        TxBuffer8[i] = i;
    }

    //--------------------------//
    //     transmit Test        //
    //--------------------------//
    
    // USE blocking mode¡¢INT mode 
    spi_slave_transmit_X1(&SPIS_Handle, (uint16_t *)TxBuffer8, 256);

    spi_slave_transmit_X1_IT(&SPIS_Handle, (uint16_t *)TxBuffer8, 256);
    while(SPIS_Handle.b_TxBusy);

    // USE DMA mode
    system_regs->dma_req[1] |= 1 << 20;
    
    DMA_Chan0.Channel               = DMA_Channel0;
    DMA_Chan0.Init.Data_Flow        = DMA_M2P_DMAC;
    DMA_Chan0.Init.Request_ID       = 1;
    DMA_Chan0.Init.Source_Inc       = DMA_ADDR_INC_INC;
    DMA_Chan0.Init.Desination_Inc   = DMA_ADDR_INC_NO_CHANGE;
    DMA_Chan0.Init.Source_Width     = DMA_TRANSFER_WIDTH_8;
    DMA_Chan0.Init.Desination_Width = DMA_TRANSFER_WIDTH_8;
    dma_init(&DMA_Chan0);

    spi_slave_transmit_X1_DMA(&SPIS_Handle);

    dma_start(&DMA_Chan0, (uint32_t)TxBuffer8, (uint32_t)&SPIS_Handle.SPIx->DR, 256, DMA_BURST_LEN_1, DMA_BURST_LEN_1);
    while(!dma_get_tfr_Status(DMA_Channel0));
    dma_clear_tfr_Status(DMA_Channel0);


    //-------------------------//
    //     receive Test        //
    //-------------------------//
    
    co_printf("---------------------------------------\r\n");
    co_printf("--------------blocking mode------------\r\n");
    co_printf("---------------------------------------\r\n");
    
    // USE blocking mode¡¢INT mode
    spi_slave_receive_X1(&SPIS_Handle, (uint16_t *)RxBuffer8, 256);

    for (i = 0; i < 256; i++)
    {
        co_printf("0x%02X \r\n", RxBuffer8[i]);
        RxBuffer8[i] = 0;
    }


    co_printf("---------------------------------------\r\n");
    co_printf("----------------INT mode---------------\r\n");
    co_printf("---------------------------------------\r\n");

    spi_slave_receive_X1_IT(&SPIS_Handle, (uint16_t *)RxBuffer8, 256);
    while(SPIS_Handle.b_RxBusy);

    for (i = 0; i < 256; i++)
    {
        co_printf("0x%02X \r\n", RxBuffer8[i]);
        RxBuffer8[i] = 0;
    }


    co_printf("---------------------------------------\r\n");
    co_printf("---------------DMA mode----------------\r\n");
    co_printf("---------------------------------------\r\n");

    // USE DMA mode
    system_regs->dma_req[1] |= 2 << 16;
    
    DMA_Chan0.Channel = DMA_Channel0;
    DMA_Chan0.Init.Data_Flow        = DMA_P2M_DMAC;
    DMA_Chan0.Init.Request_ID       = 2;
    DMA_Chan0.Init.Source_Inc       = DMA_ADDR_INC_NO_CHANGE;
    DMA_Chan0.Init.Desination_Inc   = DMA_ADDR_INC_INC;
    DMA_Chan0.Init.Source_Width     = DMA_TRANSFER_WIDTH_8;
    DMA_Chan0.Init.Desination_Width = DMA_TRANSFER_WIDTH_8;
    dma_init(&DMA_Chan0);

    spi_slave_receive_X1_DMA(&SPIS_Handle);

    dma_start(&DMA_Chan0, (uint32_t)&SPIS_Handle.SPIx->DR, (uint32_t)RxBuffer8, 256, DMA_BURST_LEN_1, DMA_BURST_LEN_1);
    while(!dma_get_tfr_Status(DMA_Channel0));
    dma_clear_tfr_Status(DMA_Channel0);

    for (i = 0; i < 256; i++)
    {
        co_printf("0x%02X \r\n", RxBuffer8[i]);
        RxBuffer8[i] = 0;
    }
}

*/
