/*
  ******************************************************************************
  * @file    driver_pdm.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   PDM module driver.
  *          This file provides firmware functions to manage the 
  *          pulse-duration modulation (PDM) peripheral
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "driver_pdm.h"

/************************************************************************************
 * @fn      pdm_IRQHandler
 *
 * @brief   Handle PDM interrupt request.
 *
 * @param   huart: PDM handle.
 */
void pdm_IRQHandler(PDM_HandleTypeDef *hpdm)
{
    if (PDM->FIFO_Status.FIFO_Almost_FULL) 
    {
        while (!PDM->FIFO_Status.FIFO_Empty) 
        {
            hpdm->p_RxData[hpdm->u32_RxCount++] = PDM->DATA;
            
            if (hpdm->u32_RxCount >= hpdm->u32_RxSize) 
            {
                /* disable fifo almost full interrupt */
                PDM->FIFO_INTE.FIFO_Almost_FULL_INT = 0;

                hpdm->b_RxBusy = false;

                break;
            }
        }
    }
}

/*********************************************************************
 * @fn      pdm_init
 *
 * @brief   Initialize the PDM.
 *
 * @param   hpdm: PDM handle.
 */
void pdm_init(PDM_HandleTypeDef *hpdm)
{
    /* Sampling Rate */
    PDM->CONTROL.CLK_CFG = hpdm->Init.SamplingRate;
    PDM->CONTROL.SR_Mode = 0;
    /* Sampling Edge */
    PDM->CONTROL.EDGE_Select = hpdm->Init.SamplingEdge;
    /* FIFO enable */
    PDM->CONTROL.FIFO_Enable = 1;

    PDM->CONTROL.ZERO_DET = 1;
    /*  high-pass filtering */
    PDM->CONTROL.HPF_EN = 1;

    /* All interrupt disable */
    PDM->FIFO_INTE.FIFO_FULL_INT        = 0;
    PDM->FIFO_INTE.FIFO_Almost_FULL_INT = 0;
    PDM->FIFO_INTE.FIFO_Empty_INT       = 0;

    /* Direct volume adjustment */
    PDM->VolumeCTRL.VOL_Direct = 1;
    
    PDM_FIFO_CLEAR();
}

/*********************************************************************
 * @fn      pdm_set_volume
 *
 * @brief   set the volume.
 *
 * @param   fu16_Volume : target volume
 */
void pdm_set_volume(uint16_t fu16_Volume)
{
    PDM->Volume = fu16_Volume;
}

/************************************************************************************
 * @fn      pdm_enable
 *
 * @brief   PDM enable.
 */
void pdm_enable(void)
{
    PDM->CONTROL.PDM_EN = 1;
}

/************************************************************************************
 * @fn      pdm_disable
 *
 * @brief   PDM disable.
 */
void pdm_disable(void)
{
    PDM->CONTROL.PDM_EN = 0;
}

/************************************************************************************
 * @fn      pdm_read_data
 *
 * @brief   receive an amount of data in blocking mode.
 */
void pdm_read_data(uint16_t *fp_Data, uint32_t fu32_Size)
{
    while (fu32_Size) 
    {
        if (!PDM->FIFO_Status.FIFO_Empty) 
        {
            *fp_Data++ = PDM->DATA;

            fu32_Size--;
        }
    }
}

/************************************************************************************
 * @fn      pdm_read_data_IT
 *
 * @brief   receive an amount of data in interrupt mode.
 */
void pdm_read_data_IT(PDM_HandleTypeDef *hpdm, uint16_t *fp_Data, uint32_t fu32_Size)
{
    if (hpdm->b_RxBusy)
        return;

    hpdm->p_RxData    = fp_Data;
    hpdm->u32_RxCount = 0;
    hpdm->u32_RxSize  = fu32_Size;
    hpdm->b_RxBusy    = true;

    /* FIFO almost full level */
    PDM_FIFO_ALMOST_LEVEL(20);

    /* enable fifo almost full interrupt */
    PDM->FIFO_INTE.FIFO_Almost_FULL_INT = 1;
}

/************************************************************************************
 * @fn      pdm_read_data_DMA
 *
 * @brief   receive an amount of data in DMA mode.
 */
void pdm_read_data_DMA(void)
{
    PDM->DMA_CFG.DMA_EN = 1;

    PDM_DMA_ALMOST_LEVEL(20);
}

/*
    for example:
    
GPIO_InitTypeDef   GPIO_Handle;
DMA_HandleTypeDef  DMA_Chan0;
PDM_HandleTypeDef  PDM_Handle;

uint16_t Buffer[512];

void main(void)
{
    GPIO_Handle.Pin       = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_Handle.Mode      = GPIO_MODE_AF_PP;
    GPIO_Handle.Pull      = GPIO_PULLUP;
    GPIO_Handle.Alternate = GPIO_FUNCTION_7;
    gpio_init(GPIO_A, &GPIO_Handle);

    __SYSTEM_PDM_CLK_ENABLE();
    __SYSTEM_DMA_CLK_ENABLE();
    
    PDM_Handle.Init.SamplingEdge = SAMPLING_RISING_EDGE;
    PDM_Handle.Init.SamplingRate = SAMPLING_RATE_16000;
    pdm_init(&PDM_Handle);

    NVIC_EnableIRQ(PDM_IRQn);

    pdm_enable();
    
    // use interrupt
    pdm_read_data_IT(&PDM_Handle, Buffer, 512);
    while(PDM_Handle.b_RxBusy);
    
    // use DMA
    __DMA_REQ_ID_PDM(1);
    
    DMA_Chan0.Channel               = DMA_Channel0;
    DMA_Chan0.Init.Data_Flow        = DMA_P2M_DMAC;
    DMA_Chan0.Init.Request_ID       = 1;
    DMA_Chan0.Init.Source_Inc       = DMA_ADDR_INC_NO_CHANGE;
    DMA_Chan0.Init.Desination_Inc   = DMA_ADDR_INC_INC;
    DMA_Chan0.Init.Source_Width     = DMA_TRANSFER_WIDTH_16;
    DMA_Chan0.Init.Desination_Width = DMA_TRANSFER_WIDTH_16;
    dma_init(&DMA_Chan0);

    pdm_read_data_DMA();

    dma_start(&DMA_Chan0, (uint32_t)&PDM->DATA , (uint32_t)Buffer, 256, DMA_BURST_LEN_16, DMA_BURST_LEN_16);
    while(!dma_get_tfr_Status(DMA_Channel0));
    dma_clear_tfr_Status(DMA_Channel0);
}

*/
