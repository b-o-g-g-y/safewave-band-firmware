/*
  ******************************************************************************
  * @file    usb_winusb.h
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
#ifndef __USB_WINUSB_H__
#define __USB_WINUSB_H__

#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#include "usb_core.h"
#include "usb_dev.h"

/* WinUSB Descriptor type and number */
#define GET_MS_DESCRIPTOR     (1)

/* Request OS OS feature descriptor */
#define EXTENDED_COMPAT_ID    (0x04)
#define EXTENDED_PROPERTIES   (0x05)


/* Exported inline functions --------------------------------------------------------*/

/* usb_winusb_init */
void usb_winusb_init(void);

#endif
