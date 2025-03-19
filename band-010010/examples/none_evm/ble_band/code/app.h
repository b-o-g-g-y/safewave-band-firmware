/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */
// #ifndef BLE_APP_H
// #define BLE_APP_H

#include <stdint.h>
#include <stdbool.h>

#include "plf.h"

/** @defgroup GAP_EVT_TYPE_DEFINES for application layer callbacks
 * @{
 */

typedef enum
{
	RF_NONE,
	RF_ADV,
	RF_CONNECT,
} rf_state_t;

typedef enum
{
	POWER_ON,
	POWER_IDLE,
	POWER_SLEEP,
	POWER_OFF,
} device_state_t;

typedef struct
{
	uint8_t strength;
	uint8_t num;
	uint8_t ontime;
	uint8_t offtime;
	uint8_t on;
	uint8_t ontime_temp;
	uint8_t offtime_temp;
} struct_motor_t;

// ��com.apple.MobileSMS, 100, 1, 10, 10;

// Define a struct to represent the linked list of apps
/*
 typedef struct
{
  uint8_t id[100];
  uint8_t strength;
  uint8_t num;
  uint8_t ontime;
  uint8_t offtime;
}struct_appid_t;
*/

#define Version 0x00010011 // 0x00010007, 0x00010010

#define LED1_PORT GPIO_A
#define LED1_PIN GPIO_PIN_5
#define LED1_SET1 gpio_write_pin(LED1_PORT, LED1_PIN, GPIO_PIN_SET)
#define LED1_SET0 gpio_write_pin(LED1_PORT, LED1_PIN, GPIO_PIN_CLEAR)

#define KEY1_PORT GPIO_C
#define KEY1_PIN GPIO_PIN_2
#define KEY1_EXTI_LINE EXTI_LINE18_PC2
#define READ_KEY1 gpio_read_pin(KEY1_PORT, KEY1_PIN)

#define VIN_PORT GPIO_D
#define VIN_PIN GPIO_PIN_1
#define VIN_EXTI_LINE EXTI_LINE25_PD1
#define READ_VIN gpio_read_pin(VIN_PORT, VIN_PIN)

#define ADC_PORT GPIO_D
#define ADC_PIN GPIO_PIN_6

#define CON_BAT_MAX 408
#define CON_BAT_MIN 320

#define CHARGE_END_PORT GPIO_C
#define CHARGE_END_PIN GPIO_PIN_3
#define READ_CHARGE_END gpio_read_pin(CHARGE_END_PORT, CHARGE_END_PIN)

// pwm
#define PWM1_PORT GPIO_A	//
#define PWM1_PIN GPIO_PIN_4 //
#define PWM1_CHANNEL PWM_CHANNEL_4

#define PWM2_PORT GPIO_D	//
#define PWM2_PIN GPIO_PIN_7 //
#define PWM2_CHANNEL PWM_CHANNEL_7

extern struct_motor_t motor;
extern uint8_t LOCK;
extern uint32_t ms_10cnt;
extern uint8_t fgCharge, forceUpdate;
extern uint32_t idleCnt;
extern uint8_t delaySecToSleep;
extern uint8_t fg_power_off;
extern uint8_t fg_rename_done;
extern uint8_t battery_state[6], pre_battery_state[6];
extern uint8_t fg_bat_ntf;
extern uint8_t bat_ccc[2];

extern uint8_t appid_buffer[1024];
extern uint16_t appid_buffer_size;
extern uint8_t sleep_param;

extern rf_state_t rf_state;
extern uint8_t slave_link_conidx;

extern uint8_t fgCharge, preFgCharge, forceUpdate, fgPowerLow;

void power_off(void);

void ntf_data(uint8_t con_idx, uint8_t att_idx, uint8_t *data, uint16_t len);

void app_pwm_init(void);
void app_pwm_deinit(void);

uint8_t findAppID(uint8_t *pdata, uint8_t lenth);
void show_string(uint8_t *data, uint32_t len, uint8_t type);

void dis_flash_data(uint32_t addr);
void device_name_save(void);
void device_name_init(void);
void device_name_default(void);

void appIDnotify(uint8_t con_idx, uint8_t *pdata, uint16_t len);

void appID_list_init(void);
void appID_list_save(uint8_t *pdata, uint16_t lenth);
void appID_list_default(void);
uint8_t findAppID(uint8_t *pdata, uint8_t lenth);

void motor_init(uint8_t strength, uint8_t num, uint8_t ontime, uint8_t offtime, uint8_t on);

void motor_load(void);
void motor_test(void);
void motor_stop(void);
void motor_task(struct_motor_t *pmotor);
void pwm1_update(uint8_t duty);
void pwm2_update(uint8_t duty);

void app_adc_deinit(void);

void app_io_init(void);
void keyTask(void);

void app_exti_init(void);
void app_pmu_int_init(void);

void app_adc_init(void);
void adc_task(void);
void charge_task(void);
void rf_goto_sleep(void);
void sp_start_adv(void);

// #endif
