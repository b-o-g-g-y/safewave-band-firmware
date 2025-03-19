/*
  ******************************************************************************
  * @file    usb_hid.h
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Header file for usb_hid.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __USB_HID_H__
#define __USB_HID_H__

#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#include "usb_core.h"
#include "usb_dev.h"

#define KEY_LEFT_CONTROL    (0x1001)
#define KEY_LEFT_SHIFT      (0x1002)
#define KEY_LEFT_ALT        (0x1004)
#define KEY_LEFT_GUI        (0x1008)
#define KEY_RIGHT_CONTROL   (0x1010)
#define KEY_RIGHT_SHIFT     (0x1020)
#define KEY_RIGHT_ALT       (0x1040)
#define KEY_RIGHT_GUI       (0x1080)

/* Exported inline functions --------------------------------------------------------*/


/* usb_hid_set_mouse_report */
void usb_hid_set_mouse_report(uint8_t fu8_Index, uint8_t fu8_Value);

/* usb_hid_set_keyboard_report */
void usb_hid_set_keyboard_report(uint16_t fu16_Key);

/* usb_hid_clear_keyboard_report */
void usb_hid_clear_keyboard_report(uint16_t fu16_Key);

/* usb_hid_send_mouse_report */
void usb_hid_send_mouse_report(void);

/* usb_hid_send_keyboard_report */
void usb_hid_send_keyboard_report(void);

/* usb_hid_init */ 
void usb_hid_init(void);

#endif
