/*
  ******************************************************************************
  * @file    driver_display.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   display abstract interfase.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/

#include <stdint.h>

#include "driver_display.h"
#include "driver_gc9c01.h"
#include "driver_jd9854.h"

#define DISPLAY_TYPE_GC9C01
//#define DISPLAY_TYPE_JD9854

void display_init(void)
{
#ifdef DISPLAY_TYPE_GC9C01
    gc9c01_init();
#endif
#ifdef DISPLAY_TYPE_JD9854
    jd9854_init();
#endif
}

void display_set_window(uint16_t x_s, uint16_t x_e, uint16_t y_s, uint16_t y_e)
{
#ifdef DISPLAY_TYPE_GC9C01
    gc9c01_set_window(x_s, x_e, y_s, y_e);
#endif
#ifdef DISPLAY_TYPE_JD9854
    jd9854_set_window(x_s, x_e, y_s, y_e);
#endif
}

void display_wait_transfer_done(void)
{
#ifdef DISPLAY_TYPE_GC9C01
    gc9c01_display_wait_transfer_done();
#endif
#ifdef DISPLAY_TYPE_JD9854
    jd9854_display_wait_transfer_done();
#endif
}

void display_update(uint32_t pixel_count, uint16_t *data, void(*callback)(void))
{
#ifdef DISPLAY_TYPE_GC9C01
    gc9c01_display(pixel_count, data, callback);
#endif
#ifdef DISPLAY_TYPE_JD9854
    jd9854_display(pixel_count, data, callback);
#endif
}

void display_gather_update(uint32_t pixel_count, uint16_t *data, uint16_t interval, uint16_t count, void(*callback)(void))
{
#ifdef DISPLAY_TYPE_GC9C01
    gc9c01_display_gather(pixel_count, data, interval, count);
#endif
#ifdef DISPLAY_TYPE_JD9854
    jd9854_display_gather(pixel_count, data, interval, count);
#endif
}

__attribute__((section("ram_code"))) void dma_isr(void)
{
#ifdef DISPLAY_TYPE_GC9C01
    gc9c01_dma_isr();
#endif
#ifdef DISPLAY_TYPE_JD9854
    jd9854_dma_isr();
#endif
}
