/*
  ******************************************************************************
  * @file    driver_keyscan.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   KeyScan module driver.
  *          This file provides firmware functions to manage the 
  *          Key Scan peripheral
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "driver_keyscan.h"

uint32_t KeyValue[5];

/*********************************************************************
 * @fn      keyscan_init
 *
 * @brief   Initialize the keyscan according to the specified parameters
 *          in the KeyScanParam 
 *
 * @param   KeyScanParam: a str_KeyScanParam_t structure that contains
 *                        the configuration information for KeyScan module
 *          
 * @return  None.
 */
void keyscan_init(str_KeyScanParam_t KeyScanParam)
{
    uint32_t i;
    uint32_t lu32_ColumnBit;
    uint32_t lu32_RowBit;
    uint32_t lu32_GPIO_PORT;

    /* KeyScan clock enable, reset disable */
    ool_write(PMU_REG_CLK_EN, ool_read(PMU_REG_CLK_EN) | PMU_KEYSCAN_CLK_EN);
    ool_write(PMU_REG_RST_CTRL, ool_read(PMU_REG_RST_CTRL) & (~PMU_KEYSCAN_SFT_RST));

    /* Config column pin */
    for (i = 0; i < KEYSCAN_MAX_COL; i++)
    {
        lu32_ColumnBit = 1 << i;

        /* GroupA have 22 Column */
        if (KeyScanParam.ColSelect_Pin_GroupA & lu32_ColumnBit) 
        {
            /* Config PA0 ~ PA7 */
            if (i < 8) 
            {
                pmu_set_pin_to_PMU(GPIO_PORT_A, lu32_ColumnBit);
                pmu_set_pin_pull(GPIO_PORT_A, lu32_ColumnBit, GPIO_PULL_UP);

                ool_pd_write16(PMU_REG_PD_PORTA_MUX, (ool_pd_read16(PMU_REG_PD_PORTA_MUX) & ~(0x03 << i * 2)) | (0x02 << i * 2));
            }
            /* Config PB0 ~ PB7 */
            else if (i < 16) 
            {
                pmu_set_pin_to_PMU(GPIO_PORT_B, lu32_ColumnBit >> 8);
                pmu_set_pin_pull(GPIO_PORT_B, lu32_ColumnBit >> 8, GPIO_PULL_UP);

                ool_pd_write16(PMU_REG_PD_PORTB_MUX, (ool_pd_read16(PMU_REG_PD_PORTB_MUX) & ~(0x03 << (i - 8) * 2)) | (0x02 << (i - 8) * 2));
            }
            /* Config PC0 ~ PC3 */
            else if (i < 20)
            {
                pmu_set_pin_to_PMU(GPIO_PORT_C, lu32_ColumnBit >> 16);
                pmu_set_pin_pull(GPIO_PORT_C, lu32_ColumnBit >> 16, GPIO_PULL_UP);

                ool_pd_write16(PMU_REG_PD_PORTC_MUX, (ool_pd_read16(PMU_REG_PD_PORTC_MUX) & ~(0x03 << (i - 16) * 2)) | (0x02 << (i - 16) * 2));
            }
        }
        /* GroupB have only 12 Column */
        else if (KeyScanParam.ColSelect_Pin_GroupB & lu32_ColumnBit) 
        {
            /* Config PC4 ~ PC7 */
            if (i < 4) 
            {
                pmu_set_pin_to_PMU(GPIO_PORT_C, lu32_ColumnBit << 4);
                pmu_set_pin_pull(GPIO_PORT_C, lu32_ColumnBit << 4, GPIO_PULL_UP);

                ool_pd_write16(PMU_REG_PD_PORTC_MUX, (ool_pd_read16(PMU_REG_PD_PORTC_MUX) & ~(0x03 << (i + 4) * 2)) | (0x02 << (i + 4) * 2));
            }
            else if (i < 12)
            {
                pmu_set_pin_to_PMU(GPIO_PORT_D, lu32_ColumnBit >> 4);
                pmu_set_pin_pull(GPIO_PORT_D, lu32_ColumnBit >> 4, GPIO_PULL_UP);

                ool_pd_write16(PMU_REG_PD_PORTD_MUX, (ool_pd_read16(PMU_REG_PD_PORTD_MUX) & ~(0x03 << (i - 4) * 2)) | (0x02 << (i - 4) * 2));
            }
        }
    }

    /* Config row pin */
    switch (KeyScanParam.RowSelect_Port)
    {
        case ROW_PORT_A: lu32_GPIO_PORT = GPIO_PORT_A; break;
        case ROW_PORT_B: lu32_GPIO_PORT = GPIO_PORT_B; break;
        case ROW_PORT_C: lu32_GPIO_PORT = GPIO_PORT_C; break;
        case ROW_PORT_D: lu32_GPIO_PORT = GPIO_PORT_D; break;
            
        default: break; 
    }

    pmu_set_pin_to_PMU((enum system_port_t)lu32_GPIO_PORT, KeyScanParam.RowSelect_Pin);
    pmu_set_pin_pull((enum system_port_t)lu32_GPIO_PORT, KeyScanParam.RowSelect_Pin, GPIO_PULL_UP);

    for (i = 0; i < KEYSCAN_MAX_ROW; i++)
    {
        lu32_RowBit = 1 << i;
        
        if (KeyScanParam.RowSelect_Pin & lu32_RowBit) 
        {
            ool_pd_write16(PMU_REG_PD_PORTA_MUX + lu32_GPIO_PORT * 2, ool_pd_read16(PMU_REG_PD_PORTA_MUX + lu32_GPIO_PORT * 2) & ~(0x03 << i * 2));
        }
    }


    /* keyscan Disable */ 
    ool_pd_write(PMU_REG_PD_KEYSCAN_CTRL, ool_pd_read(PMU_REG_PD_KEYSCAN_CTRL) & ~PMU_KEYSCAN_EN);

    /* select port, set filter value */
    ool_pd_write(PMU_REG_PD_GLITCH_CFG, KeyScanParam.RowSelect_Port | (KeyScanParam.FilterValue & 0x0F));
    
    /* select row */
    ool_pd_write(PMU_REG_PD_KEYSCAN_ROW_SEL, KeyScanParam.RowSelect_Pin);
    /* select column */
    ool_pd_write(PMU_REG_PD_KEYSCAN_COL_SEL_0,  (KeyScanParam.ColSelect_Pin_GroupA | KeyScanParam.ColSelect_Pin_GroupB) & 0xFF);
    ool_pd_write(PMU_REG_PD_KEYSCAN_COL_SEL_1, ((KeyScanParam.ColSelect_Pin_GroupA | KeyScanParam.ColSelect_Pin_GroupB) >> 8) & 0xFF);
    ool_pd_write(PMU_REG_PD_KEYSCAN_COL_SEL_2, (KeyScanParam.ColSelect_Pin_GroupA >> 16) & 0x0F);

    /* scan interval */
    ool_pd_write(PMU_REG_PD_KEYSCAN_INTERVAL_L, KeyScanParam.ScanInterval & 0xFF);
    ool_pd_write(PMU_REG_PD_KEYSCAN_INTERVAL_H, (KeyScanParam.ScanInterval) >> 8 & 0x03);

    /* keyscan enable */ 
    ool_pd_write(PMU_REG_PD_KEYSCAN_CTRL, ool_pd_read(PMU_REG_PD_KEYSCAN_CTRL) | PMU_KEYSCAN_GLITCH_EN | PMU_KEYSCAN_EN);

    pmu_enable_isr(PMU_KEYSCAN_INT_EN);
}

/*********************************************************************
 * @fn      keyscan_ReadKeyValue
 *
 * @brief   read key value.
 *
 * @param   None.
 * @return  None.
 */
void keyscan_ReadKeyValue(uint32_t *fp_KeyBuffer)
{
    uint32_t i;
    
    for (i = 0; i < 5; i++)
    {
        fp_KeyBuffer[i] = ool_pd_read32(PMU_REG_PD_KEYSCAN_VAL_00 + i * 4);
        
        co_printf("%08x\r\n", fp_KeyBuffer[i]);
    }
} 

/*********************************************************************
 * @fn      keyscan_ReadKeyValue
 *
 * @brief   read key value.
 *
 * @param   None.
 * @return  None.
 */
void keyscan_IRQHandler(void)
{
    uint8_t lu8_CTRL_REG;
    
    if (pmu_get_isr_state() & PMU_KEYSCAN_INT_STATUS) 
    {
        lu8_CTRL_REG = ool_pd_read(PMU_REG_PD_KEYSCAN_CTRL);
        
        ool_pd_write(PMU_REG_PD_KEYSCAN_CTRL, lu8_CTRL_REG | PMU_KEYSCAN_VAL_CLR);
        
        pmu_clear_isr_state(PMU_KEYSCAN_INT_CLR);
        
        keyscan_ReadKeyValue(KeyValue);

        ool_pd_write(PMU_REG_PD_KEYSCAN_CTRL, lu8_CTRL_REG & ~PMU_KEYSCAN_VAL_CLR);
    }
}

