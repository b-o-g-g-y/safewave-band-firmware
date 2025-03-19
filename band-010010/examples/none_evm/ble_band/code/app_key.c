/*
  ******************************************************************************
  * @file    driver_pwm.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   SPI module driver.
  *          This file provides firmware functions to manage the 
  *          Pulse-Width Modulatio (PWM) peripheral
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "co_printf.h"
#include "co_log.h"//
#include "driver_adc.h"
#include "driver_gpio.h"
#include "app.h"
#include "gap_api.h"
#include "os_timer.h"

#define BIT_POWER 0x01
void os_timer_ota_key_cb(void *arg);
os_timer_t os_timer_key_reset;

uint8_t BIT_LONG_KEY_DOWN,LOCK;
uint8_t BIT_BIG_PWM_TIMER,BIT_LIT_PWM_TIMER,BIT_BIGMOTOR_STOP,BIT_LITMOTOR_STOP,MOTOR_SELECT;

uint8_t pre_key,key,selectCnt;
uint32_t key_cnt,selectTime,selectTimeADD;
uint32_t idleCnt;

void os_timer_ota_key_cb(void *arg)
{
	platform_reset(0);
}


void lock_on(void)
{
		fg_power_off=1;
		rf_goto_sleep();
		co_printf("lock\r\n");
		motor_init(100,5,15,8,0);
}

void lock_off(void)
{
		fg_power_off=0;
		forceUpdate=1;
		fgPowerLow=0;
		gap_stop_advertising();
		sp_start_adv();
		rf_state=RF_ADV;
		idleCnt=0;
		co_printf("unlock\r\n");
		motor_init(100,2,15,15,0);
}

void power_on(void)
{
		fg_power_off=0;
		forceUpdate=1;
		fgPowerLow=0;
	
		co_printf("ON\r\n");
		co_printf("IDs in band:");show_string(appid_buffer,appid_buffer_size,1);
		motor_init(100,2,15,15,0);
	
		gap_stop_advertising();
		sp_start_adv();
		rf_state=RF_ADV;
		idleCnt=0;

		
}

void power_off(void)
{
	co_printf("OFF\r\n");
	fg_power_off=1;
	forceUpdate=1;
	rf_goto_sleep();
	motor_init(100,1,6,10,0);
}

/*
click the button 5times, lock the device
*/

#define CLICK_CNT 20
#define CLICK_SUM_TIME_OUT CLICK_CNT*25
#define CLICK_SUM_TIME_MIN CLICK_CNT*10
#define CLICK_INTERVAL_TIME_MIN 3
#define CLICK_INTERVAL_TIME_OUT 30

void keyTask(void)
{
	pre_key = key;
	key = 0;
	if(READ_KEY1){key &=~0x01;}else{ key|=0x01;}
    
	if((key) &&(pre_key != key))//Press down
	{
		motor_stop();
		if(selectTime>CLICK_INTERVAL_TIME_MIN){selectCnt++;}else{selectCnt=0;selectTimeADD=0;}
    selectTime=0;
  }

	if((key ==0) &&(pre_key != key))//released
	{
			key_cnt = 0;
      if (selectTime>CLICK_INTERVAL_TIME_MIN){selectCnt++;}else{selectCnt=0;selectTimeADD=0;}
      selectTime=0;
      if((selectCnt>=CLICK_CNT))
      {
				if(selectTimeADD>CLICK_SUM_TIME_MIN)
        {
           selectCnt=0;
						if(LOCK)
						{
							LOCK=0;
							lock_off();
						}
						else
						{
							LOCK=1;
							fg_power_off=1;
							lock_on();
						}
            return;
         }
         selectCnt=0;
         selectTimeADD=0;
       }
        
			if(pre_key == BIT_POWER)
			{
				if(BIT_LONG_KEY_DOWN){BIT_LONG_KEY_DOWN=0;}
        else
        {
					if(fg_power_off==0)
					{
						motor_stop();
					}
        }
			}   
  }

	//pressed....
	else if((pre_key !=0) &&(pre_key == key))
	{
		key_cnt++;
    selectTime++;
    selectTimeADD++;
    if(selectTimeADD>CLICK_SUM_TIME_OUT){selectCnt=0;selectTime=0;selectTimeADD=0;}
    if(selectTime>CLICK_INTERVAL_TIME_OUT){selectCnt=0;selectTime=0;selectTimeADD=0;}
		if((key == BIT_POWER)&&(LOCK==0))
		{
			if(key_cnt ==250)//3s
			{
		 		key_cnt++;
				if(fg_power_off==0){ power_off();}
				BIT_LONG_KEY_DOWN=1;
			}
			if(key_cnt ==2800)//30s
			{
								key_cnt++;
								appID_list_default();
								appID_list_init();
								device_name_default();
								os_timer_init(&os_timer_key_reset,os_timer_ota_key_cb,NULL);
								os_timer_start(&os_timer_key_reset, 1500, false);
								motor_init(80,3,30,20,0);
								//if(fg_power_off==0){ power_off();}
								BIT_LONG_KEY_DOWN=1;
			}
			if(key_cnt ==5)//50MS
			{
		 		key_cnt++;
				if(fg_power_off){power_on();}
				BIT_LONG_KEY_DOWN=1;
			}
		}
	}
	else
	{
      selectTime++;
      selectTimeADD++;
      if(selectTimeADD>CLICK_SUM_TIME_OUT){selectCnt=0;selectTime=0;selectTimeADD=0;}
      if(selectTime>CLICK_INTERVAL_TIME_OUT){selectCnt=0;selectTime=0;selectTimeADD=0;}
			key_cnt = 0;
	}
}



////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void app_io_init(void)
{
		GPIO_InitTypeDef    GPIO_Handle;

	  __SYSTEM_GPIO_CLK_ENABLE();
    // Output 
    GPIO_Handle.Pin       = LED1_PIN;
    GPIO_Handle.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_init(LED1_PORT, &GPIO_Handle);
		LED1_SET0;
	
    GPIO_Handle.Pin       = PWM1_PIN;
    GPIO_Handle.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_init(PWM1_PORT, &GPIO_Handle);
		gpio_write_pin(PWM1_PORT, PWM1_PIN, GPIO_PIN_CLEAR);
	
    GPIO_Handle.Pin       = PWM2_PIN;
    GPIO_Handle.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_init(PWM2_PORT, &GPIO_Handle);
		gpio_write_pin(PWM2_PORT, PWM2_PIN, GPIO_PIN_CLEAR);
	
		//input
    GPIO_Handle.Pin       = KEY1_PIN;
    GPIO_Handle.Mode      = GPIO_MODE_INPUT;
    GPIO_Handle.Pull      = GPIO_PULLUP;
    gpio_init(KEY1_PORT, &GPIO_Handle);
    gpio_read_pin(KEY1_PORT, KEY1_PIN);
	
    GPIO_Handle.Pin       = CHARGE_END_PIN;
    GPIO_Handle.Mode      = GPIO_MODE_INPUT;
    GPIO_Handle.Pull      = GPIO_PULLUP;
    gpio_init(CHARGE_END_PORT, &GPIO_Handle);
	
    GPIO_Handle.Pin       = VIN_PIN;
    GPIO_Handle.Mode      = GPIO_MODE_INPUT;
    GPIO_Handle.Pull      = GPIO_NOPULL;
    gpio_init(VIN_PORT, &GPIO_Handle);
}


void app_exti_init(void)
{
			GPIO_InitTypeDef    GPIO_Handle;
	  __SYSTEM_GPIO_CLK_ENABLE();

		// EXTI interrupt
    GPIO_Handle.Pin       = KEY1_PIN;
    GPIO_Handle.Mode      = GPIO_MODE_EXTI_IT_FALLING;
    GPIO_Handle.Pull      = GPIO_PULLUP;
    gpio_init(KEY1_PORT, &GPIO_Handle);
		
    GPIO_Handle.Pin       = VIN_PIN;
    GPIO_Handle.Mode      = GPIO_MODE_EXTI_IT_RISING;
    GPIO_Handle.Pull      = GPIO_NOPULL;
    gpio_init(VIN_PORT, &GPIO_Handle);
		
		exti_interrupt_enable(KEY1_EXTI_LINE);
		exti_interrupt_enable(VIN_EXTI_LINE);
	
		exti_set_Filter(KEY1_EXTI_LINE,100,100);
		exti_set_Filter(VIN_EXTI_LINE,100,100);
	
		exti_clear_LineStatus(KEY1_EXTI_LINE);
		exti_clear_LineStatus(VIN_EXTI_LINE);
		NVIC_EnableIRQ(GPIO_IRQn);
	}

void app_pmu_int_init(void)
{
    pmu_set_pin_to_PMU(GPIO_PORT_C, (1<<GPIO_BIT_2));
    pmu_set_pin_dir(GPIO_PORT_C, (1<<GPIO_BIT_2), GPIO_DIR_IN);
    pmu_set_pin_pull(GPIO_PORT_C, (1<<GPIO_BIT_2),GPIO_PULL_UP);
		pmu_port_wakeup_func_set(GPIO_PORT_C, GPIO_PIN_2);
	
    pmu_set_pin_to_PMU(GPIO_PORT_D, (1<<GPIO_BIT_1));
    pmu_set_pin_dir(GPIO_PORT_D, (1<<GPIO_BIT_1), GPIO_DIR_IN);
    pmu_set_pin_pull(GPIO_PORT_D, (1<<GPIO_BIT_1),GPIO_PULL_DOWN);
		pmu_port_wakeup_func_set(GPIO_PORT_D, GPIO_PIN_1);
}
	
	


