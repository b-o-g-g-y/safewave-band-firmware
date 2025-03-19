/*
  ******************************************************************************
  * @file    driver_gc9c01.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   gc9c01 driver.
  *          This file provides firmware functions to manage gc9c01 
  *          with SPI and DMA. User should set GPIO, dma channel, spi
  *          channel according actual resource allocation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "driver_spi.h"
#include "driver_gpio.h"
#include "driver_dma.h"
#include "driver_gc9c01.h"

#include "sys_utils.h"

#define LCD_RELEASE_CS()            gpio_write_pin(GPIO_D, GPIO_PIN_7, GPIO_PIN_SET)
#define LCD_SET_CS()                gpio_write_pin(GPIO_D, GPIO_PIN_7, GPIO_PIN_CLEAR)

#define LCD_RELEASE_RESET()         gpio_write_pin(GPIO_E, GPIO_PIN_1, GPIO_PIN_SET)
#define LCD_SET_RESET()             gpio_write_pin(GPIO_E, GPIO_PIN_1, GPIO_PIN_CLEAR)

#define LCD_ENABLE_BACKLIGHT()      gpio_write_pin(GPIO_B, GPIO_PIN_3, GPIO_PIN_SET)
#define LCD_DISABLE_BACKLIGHT()     gpio_write_pin(GPIO_B, GPIO_PIN_3, GPIO_PIN_CLEAR)

static DMA_LLI_InitTypeDef Link_Channel[20];
static DMA_HandleTypeDef gc9c01_DMA_Channel;
static dma_LinkParameter_t LinkParameter;
static volatile bool dma_transfer_done = true;

SPI_HandleTypeDef GC9C01_SPI_Handle;

static void (*dma_trans_done_callback)(void) = NULL;

static void gc9c01_init_io(void)
{
    GPIO_InitTypeDef GPIO_Handle;

    // backlight
    GPIO_Handle.Pin       = GPIO_PIN_3;
    GPIO_Handle.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_init(GPIO_B, &GPIO_Handle);

    // reset
    GPIO_Handle.Pin       = GPIO_PIN_1;
    GPIO_Handle.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_init(GPIO_E, &GPIO_Handle);

    /* CS */
    GPIO_Handle.Pin       = GPIO_PIN_7;
    GPIO_Handle.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_init(GPIO_D, &GPIO_Handle);

    GPIO_Handle.Pin       = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
    GPIO_Handle.Mode      = GPIO_MODE_AF_PP;
    GPIO_Handle.Pull      = GPIO_PULLDOWN;
    GPIO_Handle.Alternate = GPIO_FUNCTION_2;
    gpio_init(GPIO_D, &GPIO_Handle);

    LCD_RELEASE_CS();
    LCD_ENABLE_BACKLIGHT();
}

static void gc9c01_init_spi(void)
{
    __SYSTEM_SPI1_MASTER_CLK_SELECT_96M();
    __SYSTEM_SPI1_MASTER_CLK_ENABLE();
    
    GC9C01_SPI_Handle.SPIx                       = SPIM1;
    GC9C01_SPI_Handle.Init.Work_Mode             = SPI_WORK_MODE_0;
    GC9C01_SPI_Handle.Init.Frame_Size            = SPI_FRAME_SIZE_8BIT;
    GC9C01_SPI_Handle.Init.BaudRate_Prescaler    = 2;
    GC9C01_SPI_Handle.Init.TxFIFOEmpty_Threshold = 20;
    GC9C01_SPI_Handle.Init.RxFIFOFull_Threshold  = 0;

    spi_master_init(&GC9C01_SPI_Handle);
}

static void gc9c01_init_dma(void)
{    
    __SYSTEM_DMA_CLK_ENABLE();
    __DMA_REQ_ID_SPI1_MASTER_TX(1);
    
    gc9c01_DMA_Channel.Channel = DMA_Channel0;
    gc9c01_DMA_Channel.Init.Data_Flow        = DMA_M2P_DMAC;
    gc9c01_DMA_Channel.Init.Request_ID       = 1;
    gc9c01_DMA_Channel.Init.Source_Inc       = DMA_ADDR_INC_INC;
    gc9c01_DMA_Channel.Init.Desination_Inc   = DMA_ADDR_INC_NO_CHANGE;
    gc9c01_DMA_Channel.Init.Source_Width     = DMA_TRANSFER_WIDTH_32;
    gc9c01_DMA_Channel.Init.Desination_Width = DMA_TRANSFER_WIDTH_16;
    dma_init(&gc9c01_DMA_Channel);

    NVIC_EnableIRQ(DMA_IRQn);
}

static void gc9c01_write_cmd(uint8_t cmd)
{
    uint8_t spi_data[4];

    spi_data[0]=0x02;
    spi_data[1]=0x00;
    spi_data[2]=cmd;
    spi_data[3]=0x00;
	
    LCD_SET_CS();
    spi_master_transmit_X1(&GC9C01_SPI_Handle, (uint16_t *)spi_data, 4);
    LCD_RELEASE_CS();
}

static void gc9c01_write_cmd_with_param(uint8_t cmd, uint8_t para)
{
    uint8_t spi_data[5];
    
    spi_data[0]=0x02;
    spi_data[1]=0x00;
    spi_data[2]=cmd;
    spi_data[3]=0x00;
    spi_data[4]=para;
	
    LCD_SET_CS();
    spi_master_transmit_X1(&GC9C01_SPI_Handle, (uint16_t *)spi_data, 5);
    LCD_RELEASE_CS();
}

static void gc9c01_write_buff(uint8_t *buffer, uint8_t len)
{
    LCD_SET_CS();
    spi_master_transmit_X1(&GC9C01_SPI_Handle, (uint16_t *)buffer, len);
    LCD_RELEASE_CS();
}

void gc9c01_init(void)
{
    uint8_t dat[45] = {0x02, 0x00};

//#define INIT_VERSION_NEW

    gc9c01_init_io();
    gc9c01_init_spi();
    gc9c01_init_dma();
    
    LCD_RELEASE_RESET();
    co_delay_100us(1200);
    LCD_SET_RESET();
    co_delay_100us(3500);
    LCD_RELEASE_RESET();
    co_delay_100us(1200);

    gc9c01_write_cmd(0xfe);
    gc9c01_write_cmd(0xef);// internal reg enable
    gc9c01_write_cmd_with_param(0x80, 0x11);
    gc9c01_write_cmd_with_param(0x81, 0x70);//reg_en for 7C\7D\7E
    gc9c01_write_cmd_with_param(0x82, 0x09);//reg_en for 90\93
    gc9c01_write_cmd_with_param(0x83, 0x03);//reg_en for 98\99
    gc9c01_write_cmd_with_param(0x84, 0x20);//reg_en for B5
#ifndef INIT_VERSION_NEW
    gc9c01_write_cmd_with_param(0x85, 0x42);//reg_en for B9\BE
    gc9c01_write_cmd_with_param(0x86, 0xfc);//reg_en for C2~7
    gc9c01_write_cmd_with_param(0x87, 0x09);//reg_en for C8\CB
#endif
    gc9c01_write_cmd_with_param(0x89, 0x10);//reg_en for EC
    gc9c01_write_cmd_with_param(0x8A, 0x4f);//reg_en for F0~3\F6
    gc9c01_write_cmd_with_param(0x8C, 0x59);//reg_en for 60\63\64\66
    gc9c01_write_cmd_with_param(0x8D, 0x51);//reg_en for 68\6C\6E
#ifndef INIT_VERSION_NEW
    gc9c01_write_cmd_with_param(0x8E, 0xae);//reg_en for A1~3\A5\A7
    gc9c01_write_cmd_with_param(0x8F, 0xf3);//reg_en for AC~F\A8\A9
#endif

    gc9c01_write_cmd_with_param(0x36, 0x00);//0xC0);
    gc9c01_write_cmd_with_param(0x3a, 0x05);// 565 frame
    gc9c01_write_cmd_with_param(0xEC, 0x77);//2COL

    dat[2] = 0x74;
    dat[3] = 0x00;
    dat[4] = 0x01;
    dat[5] = 0x80;
    dat[6] = 0x00;
    dat[7] = 0x00;
    dat[8] = 0x00;
    dat[9] = 0x00;
    gc9c01_write_buff(dat, 10);//rtn 60Hz

    gc9c01_write_cmd_with_param(0x98, 0x3E);//bvdd 3x
    gc9c01_write_cmd_with_param(0x99, 0x3E);//bvee -2x

    gc9c01_write_cmd_with_param(0xC3, 0x3A);//0x2A);//VBP
    gc9c01_write_cmd_with_param(0xC4, 0x16);//0x18);//VBN

    dat[2] = 0xA1;
    dat[3] = 0x00;
    dat[4] = 0x01;
    dat[5] = 0x04;
    gc9c01_write_buff(dat, 6);

    dat[2] = 0xA2;
    dat[3] = 0x00;
    dat[4] = 0x01;
    dat[5] = 0x04;
    gc9c01_write_buff(dat, 6);

    gc9c01_write_cmd_with_param(0xA9, 0x1C);//IREF

    dat[2] = 0xA5;
    dat[3] = 0x00;
    dat[4] = 0x11;//VDDMA
    dat[5] = 0x09;//VDDML
#ifndef INIT_VERSION_NEW
    gc9c01_write_buff(dat, 6);

    gc9c01_write_cmd_with_param(0xB9, 0x8A);//RTERM
    gc9c01_write_cmd_with_param(0xA8, 0x5E);//VBG_BUF, DVDD
    gc9c01_write_cmd_with_param(0xA7, 0x40);
    gc9c01_write_cmd_with_param(0xAF, 0x73);//VDDSOU ,VDDGM
    gc9c01_write_cmd_with_param(0xAE, 0x44);//VREE,VRDD
    gc9c01_write_cmd_with_param(0xAD, 0x38);//VRGL ,VDDSF
    gc9c01_write_cmd_with_param(0xA3, 0x77);
    gc9c01_write_cmd_with_param(0xC2, 0x02);//VREG_VREF
    gc9c01_write_cmd_with_param(0xC5, 0x11);//VREG1A
    gc9c01_write_cmd_with_param(0xC6, 0x0E);//VREG1B
    gc9c01_write_cmd_with_param(0xC7, 0x13);//VREG2A
    gc9c01_write_cmd_with_param(0xC8, 0x0D);//VREG2B
#endif

    gc9c01_write_cmd_with_param(0xCB, 0x02);//bvdd ref_ad

    dat[2] = 0x7C;
    dat[3] = 0x00;
    dat[4] = 0xB6;
    dat[5] = 0x26;
    gc9c01_write_buff(dat, 6);

    gc9c01_write_cmd_with_param(0xAC, 0x24);//VGLO

    gc9c01_write_cmd_with_param(0xF6, 0x80);//EPF=2
    //*********************Ð£×¼½áÊø*************************//
    //gip start
    dat[2] = 0xB5;
    dat[3] = 0x00;
    dat[4] = 0x09;//VFP
    dat[5] = 0x09;//VBP
    gc9c01_write_buff(dat, 6);

    dat[2] = 0x60;
    dat[3] = 0x00;
    dat[4] = 0x38;
    dat[5] = 0x0B;
    dat[6] = 0x5B;
    dat[7] = 0x56;
    gc9c01_write_buff(dat, 8);//STV1&2

    dat[2] = 0x63;
    dat[3] = 0x00;
    dat[4] = 0x3A;
    dat[5] = 0xE0;//DE
    dat[6] = 0x5B;//MAX=0x61
    dat[7] = 0x56;//MAX=0x61
    gc9c01_write_buff(dat, 8);

    dat[2] = 0x64;
    dat[3] = 0x00;
    dat[4] = 0x38;
    dat[5] = 0x0D;
    dat[6] = 0x72;
    dat[7] = 0xDD;
    dat[8] = 0x5B;
    dat[9] = 0x56;
    gc9c01_write_buff(dat, 10);//CLK_group1

    dat[2] = 0x66;
    dat[3] = 0x00;
    dat[4] = 0x38;
    dat[5] = 0x11;
    dat[6] = 0x72;
    dat[7] = 0xE1;
    dat[8] = 0x5B;
    dat[9] = 0x56;
    gc9c01_write_buff(dat, 10);//CLK_group1

    dat[2] = 0x68;
    dat[3] = 0x00;
    dat[4] = 0x3B;//FLC12 FREQ
    dat[5] = 0x08;
    dat[6] = 0x08;
    dat[7] = 0x00;
    dat[8] = 0x08;
    dat[9] = 0x29;
    dat[10] = 0x5B;
    gc9c01_write_buff(dat, 11);//FLC&FLV 1~2

    dat[2] = 0x6E;
    dat[3] = 0x00;
    dat[4] = 0x00;
    dat[5] = 0x00;
    dat[6] = 0x00;
    dat[7] = 0x07;
    dat[8] = 0x01;
    dat[9] = 0x13;
    dat[10] = 0x11;
    dat[11] = 0x0B;
    dat[12] = 0x09;
    dat[13] = 0x16;
    dat[14] = 0x15;
    dat[15] = 0x1D;
    dat[16] = 0x1E;
    dat[17] = 0x00;
    dat[18] = 0x00;
    dat[19] = 0x00;
    dat[20] = 0x00;
    dat[21] = 0x00;
    dat[22] = 0x00;
    dat[23] = 0x1E;
    dat[24] = 0x1D;
    dat[25] = 0x15;
    dat[26] = 0x16;
    dat[27] = 0x0A;
    dat[28] = 0x0C;
    dat[29] = 0x12;
    dat[30] = 0x14;
    dat[31] = 0x02;
    dat[32] = 0x08;
    dat[33] = 0x00;
    dat[34] = 0x00;
    dat[35] = 0x00;
    gc9c01_write_buff(dat, 36);//gip end

    gc9c01_write_cmd_with_param(0xBE, 0x11);//SOU_BIAS_FIX

    dat[2] = 0x6C;
    dat[3] = 0x00;
    dat[4] = 0xCC;
    dat[5] = 0x0C;
    dat[6] = 0xCC;
    dat[7] = 0x84;
    dat[8] = 0xCC;
    dat[9] = 0x04;
    dat[10] = 0x50;
    gc9c01_write_buff(dat, 11);//precharge GATE

    gc9c01_write_cmd_with_param(0x7D, 0x72);
    gc9c01_write_cmd_with_param(0x7E, 0x38);

    dat[2] = 0x70;
    dat[3] = 0x00;
    dat[4] = 0x02;
    dat[5] = 0x03;
    dat[6] = 0x09;
    dat[7] = 0x05;
    dat[8] = 0x0C;
    dat[9] = 0x06;
    dat[10] = 0x09;
    dat[11] = 0x05;
    dat[12] = 0x0C;
    dat[13] = 0x06;
    gc9c01_write_buff(dat, 14);

    dat[2] = 0x90;
    dat[3] = 0x00;
    dat[4] = 0x06;
    dat[5] = 0x06;
    dat[6] = 0x05;
    dat[7] = 0x06;
    gc9c01_write_buff(dat, 8);

    dat[2] = 0x93;
    dat[3] = 0x00;
    dat[4] = 0x45;
    dat[5] = 0xFF;
    dat[6] = 0x00;
    gc9c01_write_buff(dat, 7);

    dat[2] = 0xF0;
    dat[3] = 0x00;
#ifndef INIT_VERSION_NEW
    dat[4] = 0x45;
    dat[5] = 0x09;
    dat[6] = 0x08;
    dat[7] = 0x08;
    dat[8] = 0x26;
    dat[9] = 0x2A;
#else
    dat[4] = 0x46;//0x45;
    dat[5] = 0x0b;//0x09;
    dat[6] = 0x0F;//0x08;
    dat[7] = 0x0A;//0x08;
    dat[8] = 0x10;//0x26;
    dat[9] = 0x3F;//0x2A;
#endif
    gc9c01_write_buff(dat, 10);//gamma start

    dat[2] = 0xF1;
    dat[3] = 0x00;
#ifndef INIT_VERSION_NEW
    dat[4] = 0x43;
    dat[5] = 0x70;
    dat[6] = 0x72;
#else
    dat[4] = 0x52;//0x43;
    dat[5] = 0x9A;//0x70;
    dat[6] = 0x4F;//0x72;
#endif
    dat[7] = 0x36;
    dat[8] = 0x37;
#ifndef INIT_VERSION_NEW
    dat[9] = 0x6F;
#else
    dat[9] = 0xFF;//0x6F;
#endif
    gc9c01_write_buff(dat, 10);

    dat[2] = 0xF2;
    dat[3] = 0x00;
#ifndef INIT_VERSION_NEW
    dat[4] = 0x45;
    dat[5] = 0x09;
    dat[6] = 0x08;
    dat[7] = 0x08;
    dat[8] = 0x26;
    dat[9] = 0x2A;
#else
    dat[4] = 0x46;//0x45;
    dat[5] = 0x0b;//0x09;
    dat[6] = 0x0F;//0x08;
    dat[7] = 0x0A;//0x08;
    dat[8] = 0x10;//0x26;
    dat[9] = 0x3F;//0x2A;
#endif
    gc9c01_write_buff(dat, 10);

    dat[2] = 0xF3;
    dat[3] = 0x00;

#ifndef INIT_VERSION_NEW
    dat[4] = 0x43;
    dat[5] = 0x70;
    dat[6] = 0x72;
#else
    dat[4] = 0x52;//0x43;
    dat[5] = 0x9A;//0x70;
    dat[6] = 0x4F;//0x72;
#endif
    dat[7] = 0x36;
    dat[8] = 0x37;
#ifndef INIT_VERSION_NEW
    dat[9] = 0x6F;
#else
    dat[9] = 0xFF;//0x6F;
#endif
    gc9c01_write_buff(dat, 10);//gamma end

    //gc9c01_write_cmd_with_param(0x35, 0x00);

    gc9c01_write_cmd(0x11);
    co_delay_100us(120);
    gc9c01_write_cmd(0x29);
}

void gc9c01_set_window(uint16_t x_s, uint16_t x_e, uint16_t y_s, uint16_t y_e)
{
    uint8_t spi_data[8];

    spi_data[0] = 0x02;
    spi_data[1] = 0x00;
    spi_data[2] = 0x2a;
    spi_data[3] = 0x00;
    spi_data[4] = x_s>>8;
    spi_data[5] = x_s&0xff;
    spi_data[6] = x_e>>8;
    spi_data[7] = x_e&0xff;
    gc9c01_write_buff(spi_data, 8);

    spi_data[0] = 0x02;
    spi_data[1] = 0x00;
    spi_data[2] = 0x2b;
    spi_data[3] = 0x00;
    spi_data[4] = y_s>>8;
    spi_data[5] = y_s&0xff;
    spi_data[6] = y_e>>8;
    spi_data[7] = y_e&0xff;
    gc9c01_write_buff(spi_data, 8);

    gc9c01_write_cmd(0x2c);
}

void gc9c01_display_wait_transfer_done(void)
{
    while(dma_transfer_done == false);
}

void gc9c01_display(uint32_t pixel_count, uint16_t *data, void (*callback)(void))
{
    uint32_t i;
    uint32_t total_count = pixel_count / 2;   // accoding source width
    uint8_t link_count = total_count / 4000;
    
    if(dma_transfer_done == false)
    {
        return;
    }
    else
    {
        dma_transfer_done = false;
    }
    
    dma_trans_done_callback = callback;
    
    if(total_count % 4000)
    {
        link_count++;
    }
    
    for (i = 0; i < link_count; i++)
    {
        uint8_t all_set = (total_count <= 4000);
        
        LinkParameter.SrcAddr          = (uint32_t)&data[i * 8000];
        LinkParameter.DstAddr          = (uint32_t)&SPIM1->DR;
        if(all_set)
        {
            LinkParameter.NextLink     = 0;
        }
        else
        {
            LinkParameter.NextLink     = (uint32_t)&Link_Channel[i + 1];
        }
        LinkParameter.Data_Flow        = DMA_M2P_DMAC;
        LinkParameter.Request_ID       = gc9c01_DMA_Channel.Init.Request_ID;
        LinkParameter.Source_Inc       = DMA_ADDR_INC_INC;
        LinkParameter.Desination_Inc   = DMA_ADDR_INC_NO_CHANGE;
        LinkParameter.Source_Width     = DMA_TRANSFER_WIDTH_32;
        LinkParameter.Desination_Width = DMA_TRANSFER_WIDTH_16;
        LinkParameter.Burst_Len        = DMA_BURST_LEN_4;
        LinkParameter.Size             = all_set ? (total_count) : 4000;
        LinkParameter.gather_enable    = 0;
        LinkParameter.scatter_enable   = 0;
        total_count -= 4000;

        dma_linked_list_init(&Link_Channel[i], &LinkParameter);
    }

    GC9C01_SPI_Handle.Init.Frame_Size              = SPI_FRAME_SIZE_16BIT;
    GC9C01_SPI_Handle.MultWireParam.Wire_X2X4      = Wire_X4;
    GC9C01_SPI_Handle.MultWireParam.InstructLength = INST_8BIT;
    GC9C01_SPI_Handle.MultWireParam.Instruct       = 0x32;
    GC9C01_SPI_Handle.MultWireParam.AddressLength  = ADDR_24BIT;
    GC9C01_SPI_Handle.MultWireParam.Address        = 0x003C00;
    
    LCD_SET_CS();
    spi_master_transmit_X2X4_DMA(&GC9C01_SPI_Handle);
    dma_linked_list_start_IT(Link_Channel, &LinkParameter, gc9c01_DMA_Channel.Channel);
}

void gc9c01_display_gather(uint32_t pixel_count, uint16_t *data, uint16_t interval, uint16_t count)
{
#define DMA_TRANSFER_SIZE       4000
    uint32_t total_count = pixel_count / 2;   // accoding source width
    uint16_t line_count_in_single_list = DMA_TRANSFER_SIZE/(count/2);
    uint16_t src_width_count_in_single_list = line_count_in_single_list*(count/2);
    uint8_t link_count, i;
    
    if(dma_transfer_done == false)
    {
        return;
    }
    else
    {
        dma_transfer_done = false;
    }
    
    link_count = total_count / src_width_count_in_single_list;
    if(total_count % src_width_count_in_single_list)
    {
        link_count++;
    }
    
    for (i = 0; i < link_count; i++)
    {
        uint8_t all_set = (total_count <= src_width_count_in_single_list);
        
        LinkParameter.SrcAddr          = (uint32_t)&data[i * interval * line_count_in_single_list];
        LinkParameter.DstAddr          = (uint32_t)&SPIM1->DR;
        if(all_set)
        {
            LinkParameter.NextLink     = 0;
        }
        else
        {
            LinkParameter.NextLink     = (uint32_t)&Link_Channel[i + 1];
        }
        LinkParameter.Data_Flow        = DMA_M2P_DMAC;
        LinkParameter.Request_ID       = gc9c01_DMA_Channel.Init.Request_ID;
        LinkParameter.Source_Inc       = DMA_ADDR_INC_INC;
        LinkParameter.Desination_Inc   = DMA_ADDR_INC_NO_CHANGE;
        LinkParameter.Source_Width     = DMA_TRANSFER_WIDTH_32;
        LinkParameter.Desination_Width = DMA_TRANSFER_WIDTH_16;
        LinkParameter.Burst_Len        = DMA_BURST_LEN_4;
        LinkParameter.Size             = all_set ? (total_count) : src_width_count_in_single_list;
        LinkParameter.gather_enable    = 1;
        LinkParameter.scatter_enable   = 0;
        total_count -= src_width_count_in_single_list;

        dma_linked_list_init(&Link_Channel[i], &LinkParameter);
    }
    
    GC9C01_SPI_Handle.Init.Frame_Size              = SPI_FRAME_SIZE_16BIT;
    GC9C01_SPI_Handle.MultWireParam.Wire_X2X4      = Wire_X4;
    GC9C01_SPI_Handle.MultWireParam.InstructLength = INST_8BIT;
    GC9C01_SPI_Handle.MultWireParam.Instruct       = 0x32;
    GC9C01_SPI_Handle.MultWireParam.AddressLength  = ADDR_24BIT;
    GC9C01_SPI_Handle.MultWireParam.Address        = 0x003C00;
    
    LCD_SET_CS();
    spi_master_transmit_X2X4_DMA(&GC9C01_SPI_Handle);

    __DMA_GATHER_FUNC_ENABLE(gc9c01_DMA_Channel.Channel);
    __DMA_GATHER_INTERVAL(gc9c01_DMA_Channel.Channel, (interval-count)/2);
    __DMA_GATHER_COUNT(gc9c01_DMA_Channel.Channel, count/2);
    dma_linked_list_start_IT(Link_Channel, &LinkParameter, gc9c01_DMA_Channel.Channel);
}

__attribute__((section("ram_code"))) void gc9c01_dma_isr(void)
{
    void (*callback)();
    while(__SPI_IS_BUSY(SPIM1));

    // CS Release
    LCD_RELEASE_CS();
	
    /* Clear Transfer complete status */
    dma_clear_tfr_Status(gc9c01_DMA_Channel.Channel);
    /* channel Transfer complete interrupt disable */
    dma_tfr_interrupt_disable(gc9c01_DMA_Channel.Channel);

    __SPI_DISABLE(GC9C01_SPI_Handle.SPIx);
    __SPI_DATA_FRAME_SIZE(GC9C01_SPI_Handle.SPIx, SPI_FRAME_SIZE_8BIT);
    
    dma_transfer_done = true;
    callback = dma_trans_done_callback;
    dma_trans_done_callback = NULL;
    if(callback) {
        callback();
    }
}

