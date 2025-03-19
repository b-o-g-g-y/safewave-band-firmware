/*
  ******************************************************************************
  * @file    driver_adc.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   ADC module driver.
  *          This file provides firmware functions to manage the 
  *          Analog to Digital Converter (ADC) peripheral
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "driver_adc.h"

/*********************************************************************
 * @fn      adc_init
 *
 * @brief   Initialize the ADC according to the specified parameters
 *          in the adc_InitParameter_t 
 *
 * @param   InitParam : adc_InitParameter_t structure that contains
 *                      the configuration information for ADC module
 */
void adc_init(adc_InitParameter_t InitParam)
{
    /* power on ADC module */
    ool_write(PMU_REG_ADC_CFG, ool_read(PMU_REG_ADC_CFG) | 0x80);

    /* ADC FIFO Config */
    ADC->CONTROL.FIFO_EN                = InitParam.FIFO_Enable;
    ADC->CONTROL.FIFO_Almost_Full_Level = InitParam.FIFO_AlmostFullLevel;
    /* ADC DMA  Config */
    ADC->CONTROL.DMA_EN    = InitParam.DMA_Enable;
    ADC->CONTROL.DMA_Level = InitParam.DMA_Level;
    /* Fixed use Software trigger */
    ADC->CONTROL.ADC_TRIG_SW = 0;
    /* Fixed ADC clock enable */
    ADC->CONTROL.ADC_CLK_EN = 1;


    /* ADC Setup Delay、Clock Division */
    ADC->ANA_CTRL0.Delay_For_Setup = InitParam.ADC_SetupDelay;
    ADC->ANA_CTRL0.CLK_DIV         = InitParam.ADC_CLK_DIV;
    /* Fixed ADC internal clock enable */
    ADC->ANA_CTRL0.SACLK_CTL = 1;
    /* Disable isolation */
    ADC->ANA_CTRL0.ISON = 1;
    /* Disable reset */
    ADC->ANA_CTRL0.RSTN = 1;
    /* Power On */
    ADC->ANA_CTRL0.ADC_POWER = 0;


    /* ADC Reference */
    if (InitParam.ADC_Reference == ADC_REF_1P2V) 
    {
        ADC->ANA_CTRL1.REF1P2V_BUF_EN = 1;
    }
    ADC->ANA_CTRL1.REFH_MODE = InitParam.ADC_Reference;

    /* Fixed POLL mode */
    ADC->ANA_CTRL2.POLL_MODE = 0;
    /* ERR Mask disable */
    ADC->ANA_CTRL2.TOUT_MASK = 1;
}

/*********************************************************************
 * @fn      adc_Channel_ConvertConfig
 *
 * @brief   Channel Convert Config
 * 
 * @param   fu8_ChannelBit: 1bit ~ 1 Channel. bit0 ~ Channel0
 */
void adc_Channel_ConvertConfig(uint8_t fu8_ChannelBit)
{
    /* external IO Channel */
    ADC->CONTROL.ADC_MODE = 0;
    /* Channel Enable */
    ADC->CONTROL.Channel_EN = fu8_ChannelBit;
    
    /* sample source */
    ADC->ANA_CTRL0.SAMPLE_SIG_SEL = IO_CHANNEL;
}

/*********************************************************************
 * @fn      adc_VBAT_ConvertConfig
 *
 * @brief   1/4 VBAT Convert Config
 */
void adc_VBAT_ConvertConfig(void)
{
    /* clear Legacy data */
    uint32_t TempValue = ADC->DATA;

    /* Internal signal */
    ADC->CONTROL.ADC_MODE   = 1;
    ADC->CONTROL.Channel_EN = 1;

    /* 1/4VBAT DIV Enable */
    ADC->ANA_CTRL1.VBAT_DIV_EN = 1;

    /* sample source */
    ADC->ANA_CTRL0.SAMPLE_SIG_SEL = VBAT;
}

/*********************************************************************
 * @fn      adc_VBE_ConvertConfig
 *
 * @brief   VBE(core temperature) Convert Config
 */
void adc_VBE_ConvertConfig(void)
{
    /* clear Legacy data */
    uint32_t TempValue = ADC->DATA;

    /* Internal signal */
    ADC->CONTROL.ADC_MODE   = 1;
    ADC->CONTROL.Channel_EN = 1;

    /* BUF Config */
    ADC->ANA_CTRL1.BUF_EN     = 1;
    ADC->ANA_CTRL1.BUF_IN_SEL = 0;

    /* sample source */
    ADC->ANA_CTRL0.SAMPLE_SIG_SEL = VBE;
}

/*********************************************************************
 * @fn      adc_get_channel_data
 *
 * @brief   get Channel convert Data(Not use FIFO)
 * 
 * @param   fu8_Channel : channel number(0 ~ 7)
 */
uint32_t adc_get_channel_data(uint8_t fu8_Channel)
{
    return ADC->Channel_DATA[fu8_Channel];
}

/*********************************************************************
 * @fn      adc_get_channel_data_FT
 *
 * @brief   get Channel convert Data(Not use FIFO).
 *          Calibrate ADC results using FT test param.
 * 
 * @param   fu8_Channel : channel number(0 ~ 7)
 */
uint32_t adc_get_channel_data_FT(uint8_t fu8_Channel)
{
    uint32_t lu32_ADC;
    uint32_t lu32_Result;
    
    struct_ADC_Cal_Param_t *ADC_Cal = pmu_get_adc_cal_param();

    lu32_ADC = ADC->Channel_DATA[fu8_Channel];

    lu32_ADC *= 1000;

    switch(ADC->ANA_CTRL1.REFH_MODE)
    {
        /* reference select IDOIO */
        case ADC_REF_LDOIO:
        {
            lu32_Result = (lu32_ADC - ADC_Cal->s16_constantB) / ADC_Cal->u16_slopeB;
            lu32_Result = (3103 * lu32_Result) / 1000;
        }break;
        
        /* reference select 1.2V */
        case ADC_REF_1P2V:
        {
            lu32_Result = (lu32_ADC - ADC_Cal->s16_constantA) / ADC_Cal->u16_slopeA;
            lu32_Result = (8533 * lu32_Result) / 1000;
        }break;
    }
 
    return lu32_Result;
}

/*********************************************************************
 * @fn      adc_get_data
 *
 * @brief   use FIFO or use 1/4VBAT、VBE、Channel voltage divider. 
 *          read convert Data from Data register
 */
uint32_t adc_get_data(void)
{
    return ADC->DATA & 0x3FF;
}

/*********************************************************************
 * @fn      adc_get_data_FT
 *
 * @brief   use FIFO or use 1/4VBAT、VBE、Channel voltage divider. 
 *          read convert Data from Data register
 *          Calibrate ADC results using FT test param.
 */
uint32_t adc_get_data_FT(void)
{
    uint32_t lu32_ADC;
    uint32_t lu32_Result;

    struct_ADC_Cal_Param_t *ADC_Cal = pmu_get_adc_cal_param();

    lu32_ADC = ADC->DATA & 0x3FF;

    lu32_ADC *= 1000;

    switch(ADC->ANA_CTRL1.REFH_MODE)
    {
        /* reference select IDOIO */
        case ADC_REF_LDOIO:
        {
            lu32_Result = (lu32_ADC - ADC_Cal->s16_constantB) / ADC_Cal->u16_slopeB;
            lu32_Result = (3103 * lu32_Result) / 1000;
        }break;
        
        /* reference select 1.2V */
        case ADC_REF_1P2V:
        {
            lu32_Result = (lu32_ADC - ADC_Cal->s16_constantA) / ADC_Cal->u16_slopeA;
            lu32_Result = (8533 * lu32_Result) / 1000;
        }break;
    }

    return lu32_Result;
}

/*********************************************************************
 * @fn      adc_int_enable
 *
 * @brief   interrupt enable
 */
void adc_int_enable(enum_int_t fe_INT)
{
    switch (fe_INT)
    {
        case FIFO_FULL_INT:        ADC->INT.FIFO_FULL_INT        = 1; break;
        case FIFO_EMPTY_INT:       ADC->INT.FIFO_EMPTY_INT       = 1; break;
        case FIFO_ALMOST_FULL_INT: ADC->INT.FIFO_ALMOST_FULL_INT = 1; break;
        case FIFO_OVER_INT:        ADC->INT.FIFO_OVER_INT        = 1; break;
        case FIFO_UNDER_INT:       ADC->INT.FIFO_UNDER_INT       = 1; break;
        case ERR_INT:              ADC->INT.ERR_INT              = 1; break;
        case SW_INT:               ADC->INT.SW_INT               = 1; break;
            
        default: break; 
    }
}

/*********************************************************************
 * @fn      adc_int_disable
 *
 * @brief   interrupt disable
 */
void adc_int_disable(enum_int_t fe_INT)
{
    switch (fe_INT)
    {
        case FIFO_FULL_INT:        ADC->INT.FIFO_FULL_INT        = 0; break;
        case FIFO_EMPTY_INT:       ADC->INT.FIFO_EMPTY_INT       = 0; break;
        case FIFO_ALMOST_FULL_INT: ADC->INT.FIFO_ALMOST_FULL_INT = 0; break;
        case FIFO_OVER_INT:        ADC->INT.FIFO_OVER_INT        = 0; break;
        case FIFO_UNDER_INT:       ADC->INT.FIFO_UNDER_INT       = 0; break;
        case ERR_INT:              ADC->INT.ERR_INT              = 0; break;
        case SW_INT:               ADC->INT.SW_INT               = 0; break;
            
        default: break; 
    }
}

/*********************************************************************
 * @fn      adc_get_int_status
 *
 * @brief   get interrupt status
 */
bool adc_get_int_status(enum_int_t fe_INT)
{
    bool lb_Status;

    switch (fe_INT)
    {
        case FIFO_FULL_INT:        lb_Status = ADC->INT.FIFO_FULL_STATUS;        break;
        case FIFO_EMPTY_INT:       lb_Status = ADC->INT.FIFO_EMPTY_STATUS;       break;
        case FIFO_ALMOST_FULL_INT: lb_Status = ADC->INT.FIFO_ALMOST_FULL_STATUS; break;
        case FIFO_OVER_INT:        lb_Status = ADC->INT.FIFO_OVER_STATUS;        break;
        case FIFO_UNDER_INT:       lb_Status = ADC->INT.FIFO_UNDER_STATUS;       break;
        case ERR_INT:              lb_Status = ADC->INT.ERR_STATUS;              break;
        case SW_INT:               lb_Status = ADC->INT.SW_STATUS;               break;
            
        default: break; 
    }

    return lb_Status;
}

/*********************************************************************
 * @fn      adc_clr_int_status
 *
 * @brief   clear interrupt status
 */
void adc_clr_int_status(enum_int_t fe_INT)
{
    switch (fe_INT)
    {
        case FIFO_FULL_INT:        ADC->INTC.FIFO_FULL_CLR        = 1; break;
        case FIFO_EMPTY_INT:       ADC->INTC.FIFO_EMPTY_CLR       = 1; break;
        case FIFO_ALMOST_FULL_INT: ADC->INTC.FIFO_ALMOST_FULL_CLR = 1; break;
        case FIFO_OVER_INT:        ADC->INTC.FIFO_OVER_CLR        = 1; break;
        case FIFO_UNDER_INT:       ADC->INTC.FIFO_UNDER_CLR       = 1; break;
        case ERR_INT:              ADC->INTC.ERR_CLR              = 1; break;
        case SW_INT:               ADC->INTC.SW_CLR               = 1; break;

        default: break; 
    }
}

/*********************************************************************
 * @fn      adc_get_fifo_Status
 *
 * @brief   get FIFO status
 */
uint32_t adc_get_fifo_Status(void)
{
    return ADC->FIFO_STATUS.FIFO_Statsu_DWORD;
}

/*********************************************************************
 * @fn      adc_DataValid_status
 *
 * @brief   Data Valid status
 *
 * @return  true:  Valid
 *          false: Invalid
 */
bool adc_DataValid_status(void)
{
    return (bool)(ADC->CONTROL.DATA_VALID);
}

/*********************************************************************
 * @fn      adc_fifo_clear
 *
 * @brief   Clear ADC FIFO 
 */
void adc_fifo_clear(void)
{
    /* Start Clearing FIFO */
    ADC->CONTROL.FIFO_CLR = 1;
    /* Stop */
    ADC->CONTROL.FIFO_CLR = 0;
}

/*********************************************************************
 * @fn      adc_convert_enable
 *
 * @brief   adc convert start
 */
void adc_convert_enable(void)
{
    adc_fifo_clear();
    
    ADC->CONTROL.ADC_EN = 1;
}

/*********************************************************************
 * @fn      adc_convert_disable
 *
 * @brief   adc convert stop
 */
void adc_convert_disable(void)
{
    ADC->CONTROL.ADC_EN = 0;
}
