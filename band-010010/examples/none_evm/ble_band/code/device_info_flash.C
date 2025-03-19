/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */

/*
 * INCLUDES (����ͷ�ļ�)
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "os_mem.h"
#include "plf.h"
#include "driver_system.h"
#include "driver_wdt.h"
#include "driver_uart.h"

// #include "qspi.h"
// #include "co_utils.h"
#include "gatt_api.h"
#include "gap_api.h"

#include "sys_utils.h"
#include "crc32.h"

#include "ota.h"
#include "ota_service.h"
#include "os_timer.h"
#include "driver_system.h"

#include "app.h"

#include "compiler.h"

//
// #define KEY_INFO_OFFSET          0x7f000//����KEY
// #define PAIRING_INFO_OFFSET          0x7e000//�����Ϣ
// #define CONN_INFO_OFFSET          0x7d000//�Զ˷�����Ϣ
#define DEVICE_NAME_OFFSET 0x7c000
#define APPID_LIST_OFFSET 0x7B000

extern uint8_t local_name[13]; // 13
extern uint8_t dev_name[20];
extern uint8_t dev_name_size;

uint8_t appid_temp_len;
uint8_t appid_temp[100];

// static os_timer_t os_timer_ancs_notify;

void dis_flash_data(uint32_t addr);
void device_name_save(void);
void device_name_init(void);
void device_name_default(void);

void appID_list_init(void);
void appID_list_save(uint8_t *pdata, uint16_t lenth);
void appID_list_default(void);
uint8_t findAppID(uint8_t *pdata, uint8_t lenth);

__attribute__((section("ram_code"))) static void app_device_flash_opration(uint32_t dest, uint8_t *src, uint32_t len, uint8_t write_or_earse)
{
	motor_stop();
	GLOBAL_INT_DISABLE();
	if (write_or_earse)
		flash_write(dest, len, src);
	else
		flash_erase(dest, 0x1000);
	GLOBAL_INT_RESTORE();
}

void show_string(uint8_t *data, uint32_t len, uint8_t type)
{
	uint32_t i = 0;
	if (len == 0)
		return;
	for (; i < len; i++)
	{
		if (type == 0)
		{
			co_printf("%02X", *data++);
		}
		else
		{
			co_printf("%c", *data++);
		}
	}
	co_printf("\r\n");
}

//////////////////////////////////////////////////////////////////////////////////
void device_name_init(void)
{
	uint8_t j[25] = {0, 0, 0, 0};
	uint32_t addr;
	addr = DEVICE_NAME_OFFSET;
	flash_read(addr, 25, j);
	if ((j[0] == 0x55) && (j[1] == 0xaa) && (j[2] == 0x55) && (j[3] == 0xaa))
	{
		dev_name_size = j[4];
		memcpy(dev_name, &j[5], 20); // get name form flash
		co_printf("device name: ");
		show_string(dev_name, dev_name_size, 1);
	}
	else
	{
		device_name_default(); // save name to flash
	}
}

void device_name_save(void)
{
	uint8_t i[4] = {0x55, 0xaa, 0x55, 0xaa};
	uint32_t addr;
	addr = DEVICE_NAME_OFFSET;
	app_device_flash_opration(addr, NULL, 0, 0);
	app_device_flash_opration(addr, i, 4, 1);
	app_device_flash_opration(addr + 4, &dev_name_size, 1, 1);
	app_device_flash_opration(addr + 5, dev_name, 20, 1);
}

void device_name_default(void)
{
	dev_name_size = sizeof(local_name);
	memcpy(dev_name, local_name, dev_name_size);
	device_name_save();
	co_printf("device factory name: ");
	show_string(dev_name, dev_name_size, 1);
}

////////////////////////////////////////////////////////////////////////////////////////
// �� com.apple.MobileSMS, 100, 1, 10, 10; com.safewavetech.beta, 50, 2, 10, 10; com.ring, 75, 3, 10, 10��
// test
//  com.apple.MobileSMS, 100, 1, 10, 10; com.safewavetech.beta, 50, 2, 10, 10; com.apple.mobilephone, 75, 3, 10, 10;
//  com.apple.MobileSMS, 100, 2, 20, 20; com.safewavetech.beta, 50, 2, 10, 10; com.apple.mobilephone, 75, 6, 30, 30;

const char appid_defalut[] = " com.apple.MobileSMS, 100, 1, 10, 10; com.safewavetech.beta, 50, 2, 10, 10; com.apple.mobilephone, 75, 3, 10, 10; com.apple.mobilecal, 50, 4, 15, 15; com.apple.reminders, 50, 5, 10, 10;";
uint8_t appid_buffer[1024];
uint16_t appid_buffer_size;

void os_timer_ancs_notify_cb(void *arg)
{
	appIDnotify(0, &appid_temp[0], appid_temp_len);
	co_printf("notify ancs:");
	show_string(&appid_temp[0], appid_temp_len, 1);
}

void appID_list_init(void)
{
	uint8_t j[6] = {0, 0, 0, 0, 0, 0};
	uint32_t addr;
	addr = APPID_LIST_OFFSET;
	flash_read(addr, sizeof(j), j);
	addr += sizeof(j);
	if ((j[0] == 0x55) && (j[1] == 0xaa) && (j[2] == 0x55) && (j[3] == 0xaa))
	{
		appid_buffer_size = j[5] << 8 | j[4];
		flash_read(addr, appid_buffer_size, appid_buffer);
		co_printf("appID in band:");
		show_string(appid_buffer, appid_buffer_size, 1);
	}
	else
	{
		appID_list_default();
	}
	// os_timer_init(&os_timer_ancs_notify,os_timer_ancs_notify_cb,NULL);
}

void appID_list_save(uint8_t *pdata, uint16_t lenth)
{
	uint8_t i[4] = {0x55, 0xaa, 0x55, 0xaa};
	uint32_t addr;
	addr = APPID_LIST_OFFSET;
	app_device_flash_opration(addr, NULL, 0, 0);
	app_device_flash_opration(addr, i, sizeof(i), 1);
	addr += sizeof(i);
	app_device_flash_opration(addr, (uint8_t *)&lenth, sizeof(lenth), 1);
	addr += sizeof(lenth);
	app_device_flash_opration(addr, pdata, lenth, 1);
}

void appID_list_default(void)
{
	uint16_t i;
	for (i = 0; i < sizeof(appid_defalut); i++)
	{
		appid_buffer[i] = appid_defalut[i];
	}
	appID_list_save(appid_buffer, sizeof(appid_defalut));
}

uint8_t str_to_char(uint8_t *pdata, uint8_t lenth)
{
	uint8_t i = 0;
	uint8_t value = 0;
	for (i = 0; i < lenth; i++)
	{
		if ((*pdata >= '0') && (*pdata <= '9'))
		{
			if (i)
			{
				value = value * 10;
			}
			value += *pdata - '0';
			pdata++;
		}
		else
		{
			return 0; // err
		}
	}
	return value;
}

void getMotorData(uint8_t *pdata, uint8_t lenth)
{
	uint8_t i = 0;
	uint8_t j = 0;
	static uint8_t buffer[3];
	static uint8_t strength, num, ontime, offtime;
	// com.apple.MobileSMS, 100, 1, 10, 10
	// find end flag ','
	for (i = 0; i < lenth; i++)
	{
		if (*pdata++ == ',')
		{
			break;
		}
	}

	j = 0;
	if (*pdata++ == ' ')
	{
		do
		{
			buffer[j++] = *pdata++;
		} while ((*pdata != ',') && (j < 4));
	}
	pdata++;
	if (j < 4)
	{
		strength = str_to_char(buffer, j);
		if (strength) // next
		{
			j = 0;
			if (*pdata++ == ' ')
			{
				do
				{
					buffer[j++] = *pdata++;
				} while ((*pdata != ',') && (j < 4));
			}
			pdata++;
			if (j < 4)
			{
				num = str_to_char(buffer, j);
				if (num) // next
				{
					j = 0;
					if (*pdata++ == ' ')
					{
						do
						{
							buffer[j++] = *pdata++;
						} while ((*pdata != ',') && (j < 4));
					}
					pdata++;
					if (j < 4)
					{
						ontime = str_to_char(buffer, j);
						if (ontime) // next
						{
							j = 0;
							if (*pdata++ == ' ')
							{
								do
								{
									buffer[j++] = *pdata++;
								} while ((*pdata != ';') && (j < 4));
							}
							pdata++;
							if (j < 4)
							{
								offtime = str_to_char(buffer, j);
								if (offtime)
								{
									// co_printf("motor datas:0x%02X%02X%02X%02X\r\n", strength,num,ontime,offtime);
									motor_init(strength, num, ontime, offtime, 0);
								}
							}
						}
					}
				}
			}
		}
	}
}

uint8_t findAppID(uint8_t *pdata, uint8_t lenth)
{
	uint8_t j = 0;
	uint8_t fg_sta = 0;
	uint16_t i;

	fg_sta = 0;
	for (i = 0; i < 1024; i++)
	{
		if (fg_sta == 0)
		{
			if (appid_buffer[i] == ' ') // apps start flag
			{
				fg_sta = 1;
				j = 0;
			}
			else
			{
				co_printf("appid data err\r\n");
				return 1;
			}
		}
		else
		{
			if (appid_buffer[i] == ';') // apps end
			{
				appid_temp[j++] = appid_buffer[i];
				//		show_string(appid_temp,j,1);//test
				if (memcmp(pdata, appid_temp, lenth) == 0)
				{
					co_printf("IDmatch:");
					show_string(appid_temp, j, 1); // test
					getMotorData(appid_temp, j);
					appid_temp_len = 0;
					for (appid_temp_len = 0; appid_temp_len < j; appid_temp_len++)
					{
						if (appid_temp[appid_temp_len] == ',')
						{
							appIDnotify(0, &appid_temp[0], appid_temp_len);
							// co_printf("notify ancs:");show_string(&appid_temp[0],appid_temp_len,1);
							return 0;
						}
					}
				}
				else
				{
					j = 0;
					fg_sta = 0;
				}
			}
			else
			{
				appid_temp[j++] = appid_buffer[i];
			}
		}
	}
	co_printf("no appid match\r\n");
	return 1;
}

// for test
void dis_flash_data(uint32_t addr)
{
	uint8_t j[256] = {0, 0, 0, 0};
	flash_read(addr, 256, j);
	co_printf("addr+0x00:");
	show_reg(&j[0], 16, 1);
	co_printf("addr+0x10:");
	show_reg(&j[0x10], 16, 1);
	co_printf("addr+0x20:");
	show_reg(&j[0x20], 16, 1);
	co_printf("addr+0x30:");
	show_reg(&j[0x30], 16, 1);
	co_printf("addr+0x40:");
	show_reg(&j[0x40], 16, 1);
	co_printf("addr+0x50:");
	show_reg(&j[0x50], 16, 1);
	co_printf("addr+0x60:");
	show_reg(&j[0x60], 16, 1);
	co_printf("addr+0x70:");
	show_reg(&j[0x70], 16, 1);
	co_printf("addr+0x80:");
	show_reg(&j[0x80], 16, 1);
	co_printf("addr+0x90:");
	show_reg(&j[0x90], 16, 1);
	co_printf("addr+0xa0:");
	show_reg(&j[0xa0], 16, 1);
	co_printf("addr+0xb0:");
	show_reg(&j[0xb0], 16, 1);
	co_printf("addr+0xc0:");
	show_reg(&j[0xc0], 16, 1);
	co_printf("addr+0xd0:");
	show_reg(&j[0xd0], 16, 1);
	co_printf("addr+0xe0:");
	show_reg(&j[0xe0], 16, 1);
	co_printf("addr+0xf0:");
	show_reg(&j[0xf0], 16, 1);
}
