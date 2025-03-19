/*
  ******************************************************************************
  * @file    driver_if8080.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   IF8080 module driver.
  *          This file provides firmware functions to manage the 
  *          Liquid Crystal Display (IF8080) peripheral.
  *          Support 8080/6800 interface.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "driver_if8080.h"

/*********************************************************************
 * @fn      if8080_init
 *
 * @brief   Initialize the IF8080 according to the specified parameters
 *          in the str_LCDParam_t 
 *
 * @param   hdma : pointer to a str_LCDParam_t structure that contains
 *                 the configuration information for IF8080 module
 *          
 * @return  None.
 */
void if8080_init(str_LCDParam_t LcdInit)
{
    /* DMA Config */
    IF8080->DMA_CFG.DMA_ENABLE   = 1;
    IF8080->DMA_CFG.DMA_TX_LEVEL = 20;
    /* select 8080 or 6800 */
    IF8080->INTF_CFG.MODE = LcdInit.LcdMode;
    /* select 8bit or 16bit */
    IF8080->DATA_CFG.DATA_BUS_BIT = LcdInit.DataBusSelect;
    /* Write Clock DIV */
    IF8080->CRM.WRITE_CLK_CFG = LcdInit.WriteClock;
    /* Read Clock DIV */
    IF8080->CRM.READ_CLK_CFG  = LcdInit.ReadClock;
}

/*********************************************************************
 * @fn      if8080_write_cmd
 *
 * @brief   Sending command
 *
 * @param   fp8_CMD : command data
 *  
 * @return  None.
 */
void if8080_write_cmd(uint8_t fp8_CMD)
{
    /* 6800 mode */
    if (IF8080->INTF_CFG.MODE == MODE_6800) 
    {
        IF8080->INTF_CFG.RD = 0;
    }

    /* wait bus idle */
    if8080_wait_bus_idle();

    /* Write, CMD */
    IF8080->CFG = (uint32_t)1 << 31 | fp8_CMD;

    while(!(IF8080->CFGWR_STATUS.STATUS));
}

/*********************************************************************
 * @fn      if8080_write_param
 *
 * @brief   Sending parameter.
 *
 * @param   fu16_Data : parameter. Can be 8 bit or 16 bit, depend on BUS bit.
 *
 * @return  None.
 */
void if8080_write_param(uint16_t fu16_Data)
{
    /* 6800 mode */
    if (IF8080->INTF_CFG.MODE == MODE_6800) 
    {
        IF8080->INTF_CFG.RD = 0;
    }

    /* Write, CMD */
    IF8080->CFG = (uint32_t)3 << 30 | fu16_Data;
    
    while(!(IF8080->CFGWR_STATUS.STATUS));
}

/*********************************************************************
 * @fn      if8080_write_data
 *
 * @brief   Sending data or parameters
 *
 * @param   fp32_WriteBuffer : Write data buffer
 *          fu32_WriteNum    : transmit number.
 *                             1. select DATA_BUS_8_BIT,  1 count sent 1 byte
 *                             2. select DATA_BUS_16_BIT, 1 count sent 2 byte
 * @return  None.
 */
void if8080_write_data(uint32_t *fp32_WriteBuffer, uint32_t fu32_WriteNum)
{
    uint32_t i;
    uint32_t lu32_Num;

    /* wait bus idle */
    //if8080_wait_bus_idle();

    /* 6800 mode */
    if (IF8080->INTF_CFG.MODE == MODE_6800) 
    {
        IF8080->INTF_CFG.RD = 0;
    }

    IF8080->DATA_WR_LEN.LEN = fu32_WriteNum;

    /* 8 bit bus */
    if (IF8080->DATA_CFG.DATA_BUS_BIT == DATA_BUS_8_BIT) 
    {
        lu32_Num = fu32_WriteNum / 4;
        
        if (fu32_WriteNum % 4) 
        {
            lu32_Num++;
        }
    }
    /* 16 bit bus */
    else 
    {
        lu32_Num = fu32_WriteNum / 2;
        
        if (fu32_WriteNum % 2) 
        {
            lu32_Num++;
        }
    }

    for (i = 0; i < lu32_Num; i++)
    {
        while (IF8080->INT.TX_FULL_INT_STATUS);

        IF8080->TX_FIFO = *fp32_WriteBuffer++;
    }
}

/*********************************************************************
 * @fn      if8080_read_data_8bit
 *
 * @brief   read data. select DATA_BUS_8_BIT, 1 count receive 1 byte.
 *
 * @param   fu8_Param      : read Param
 *          fp8_ReadBuffer : read data buffer.
 *          fu32_ReadNum   : receive number.
 * 
 * @return  None.
 */
void if8080_read_data_8bit(uint8_t fu8_Param, uint8_t *fp8_ReadBuffer, uint32_t fu32_ReadNum)
{
    uint32_t i;

    /* wait bus idle */
    if8080_wait_bus_idle();
    
    /* 6800 mode */
    if (IF8080->INTF_CFG.MODE == MODE_6800) 
    {
        IF8080->INTF_CFG.RD = 1;
    }

    /* Read Length */
    IF8080->CFG = (fu32_ReadNum << 8) | fu8_Param;

    for (i = 0; i < fu32_ReadNum; i++)
    {
        while(!(IF8080->CFGRD_STATUS.STATUS));

        fp8_ReadBuffer[i] = IF8080->APB_RD.RD_DATA;
    }
}

/*********************************************************************
 * @fn      if8080_read_data_16bit
 *
 * @brief   read data. select DATA_BUS_16_BIT, 1 count receive 2 byte.
 *
 * @param   fu8_Param       : read Param
            fp16_ReadBuffer : read data buffer.
 *          fu32_ReadNum    : receive number.
 * 
 * @return  None.
 */
void if8080_read_data_16bit(uint8_t fu8_Param, uint16_t *fp16_ReadBuffer, uint32_t fu32_ReadNum)
{
    uint32_t i;

    /* wait bus idle */
    if8080_wait_bus_idle();

    /* 6800 mode */
    if (IF8080->INTF_CFG.MODE == MODE_6800) 
    {
        IF8080->INTF_CFG.RD = 1;
    }

    /* Read Length */
    IF8080->CFG = (fu32_ReadNum << 8) | fu8_Param;

    for (i = 0; i < fu32_ReadNum; i++)
    {
        while(!(IF8080->CFGRD_STATUS.STATUS));
        
        fp16_ReadBuffer[i] = IF8080->APB_RD.RD_DATA;
    }
}



/*
    Use case:
    
    
    int main(void)
    {
    
        system_set_port_mux(GPIO_PORT_B, GPIO_BIT_0, PORTB0_FUNC_LCD_DAT0);    // 8080_BIT0
        system_set_port_mux(GPIO_PORT_B, GPIO_BIT_1, PORTB1_FUNC_LCD_DAT1);    // 8080_BIT1
        system_set_port_mux(GPIO_PORT_B, GPIO_BIT_2, PORTB2_FUNC_LCD_DAT2);    // 8080_BIT2
        system_set_port_mux(GPIO_PORT_B, GPIO_BIT_3, PORTB3_FUNC_LCD_DAT3);    // 8080_BIT3
        system_set_port_mux(GPIO_PORT_B, GPIO_BIT_4, PORTB4_FUNC_LCD_DAT4);    // 8080_BIT4
        system_set_port_mux(GPIO_PORT_B, GPIO_BIT_5, PORTB5_FUNC_LCD_DAT5);    // 8080_BIT5
        system_set_port_mux(GPIO_PORT_B, GPIO_BIT_6, PORTB6_FUNC_LCD_DAT6);    // 8080_BIT6
        system_set_port_mux(GPIO_PORT_B, GPIO_BIT_7, PORTB7_FUNC_LCD_DAT7);    // 8080_BIT7
        
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_LCD_WRX);     // 8080_WR
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_LCD_RDX);     // 8080_RD
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_LCD_CSX);     // 8080_CS
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA7_FUNC_LCD_DCX);     // 8080_DC
        
        system_set_port_mux(GPIO_PORT_D, GPIO_BIT_0, PORTD0_FUNC_D0);          // 8080_Reset  software simulation
        
        str_LCDParam_t str_LCDParam;
        
        str_LCDParam.LcdMode       = MODE_8080;
        str_LCDParam.DataBusSelect = DATA_BUS_8_BIT;
        str_LCDParam.ReadClock     = RDCLK_DIV_8;
        str_LCDParam.WriteClock    = WDCLK_DIV_8;

        if8080_init(str_LCDParam);
        
        if8080_cs_set();
        
        if8080_write_cmd(...);
        if8080_write_param(...);
        
        if8080_write_cmd(...);
        if8080_write_param(...);
        
        if8080_write_data(..., ...);
        
        if8080_cs_release();
    }
*/








