/*
  ******************************************************************************
  * @file    driver_keyscan.h
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Header file of KeyScan HAL module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __DRIVER_KEY_SCAN_H__
#define __DRIVER_KEY_SCAN_H__

#include "stdint.h"
#include "stdbool.h"

#include "driver_pmu.h"

#define KEYSCAN_MAX_ROW    (8)
#define KEYSCAN_MAX_COL    (20)

enum KeyScan_Row
{
    ROW_PORT_A = 0x10,
    ROW_PORT_B = 0x20,
    ROW_PORT_C = 0x30,
    ROW_PORT_D = 0x40,
};

typedef struct 
{
    uint16_t  FilterValue;       /* This parameter can be a 4-bit Size */
    uint16_t  ScanInterval;      /* This parameter can be a 10-bit Size */
    
    uint16_t  RowSelect_Port;    /* This parameter can be a value of @ref KeyScan_Row */
    uint16_t  RowSelect_Pin;     /* This parameter can be a 8-bit Size. 1 BIT for 1 ROW */
                                 /* Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0 */ 
                                 /*  |    |    |    |    |    |    |    |   */
                                 /* Row7 Row6 Row5 Row4 Row3 Row2 Row1 Row0 */ 

    uint32_t  ColSelect_Pin_GroupA;     /* This parameter can be a 20-bit Size. 1 BIT for 1 COL */
    
    /* Bit9  Bit8  Bit7  Bit6  Bit5  Bit4  Bit3  Bit2  Bit1  Bit0 */ 
    /*  |     |     |     |     |     |     |     |     |     |   */
    /* Col9  Col8  Col7  Col6  Col5  Col4  Col3  Col2  Col1  Col0 */ 
    /*  |     |     |     |     |     |     |     |     |     |   */
    /* PB1   PB0   PA7   PA6   PA5   PA4   PA3   PA2   PA1   PA0  */          

    /* Bit19  Bit18  Bit17  Bit16  Bit15  Bit14  Bit13  Bit12  Bit11  Bit10 */ 
    /*   |      |      |      |      |      |      |      |      |      |   */
    /* Col19  Col18  Col17  Col16  Col15  Col14  Col13  Col12  Col11  Col10 */ 
    /*   |      |      |      |      |      |      |      |      |      |   */
    /*  PC3    PC2    PC1    PC0    PB7    PB6    PB5    PB4    PB3    PB2  */ 


    uint32_t  ColSelect_Pin_GroupB;     /* This parameter can be a 12-bit Size. 1 BIT for 1 COL */
    
    /* Bit11  Bit10  Bit9  Bit8  Bit7  Bit6  Bit5  Bit4  Bit3  Bit2  Bit1  Bit0 */ 
    /*   |      |     |     |     |     |     |     |     |     |     |     |   */
    /* Col11  Col10  Col9  Col8  Col7  Col6  Col5  Col4  Col3  Col2  Col1  Col0 */ 
    /*   |      |     |     |     |     |     |     |     |     |     |     |   */
    /*  PD7    PD6   PD5   PD4   PD3   PD2   PD1   PD0   PC7   PC6   PC5   PC4  */
    
    /*
        Attention: Three ways to use column.
                    1. only use GroupA.
                    2. only use GroupB.
                    3. use GroupA and GroupB, the selected column cannot be duplicated.
                       For example, you can select GroupA->Bit0(Col0) and GroupB->Bit1(Col1).
                                    you cannot select GroupA->Bit0(Col0) and GroupB->Bit0(Col0).
    */
}str_KeyScanParam_t;


/* Exported functions --------------------------------------------------------*/

/* keyscan_init */
void keyscan_init(str_KeyScanParam_t KeyScanParam);

/* keyscan_ReadKeyValue */
void keyscan_ReadKeyValue(uint32_t *fp_KeyBuffer);

/* keyscan_IRQHandler */
void keyscan_IRQHandler(void);

#endif
