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
#include "co_printf.h"//
#include "driver_pwm.h"//
#include "driver_gpio.h"//
#include "os_timer.h"
#include "app.h"

void	motor_load(void);
void	motor_test(void);


struct_motor_t motor;

//os_timer_t motor1_test_timer,motor2_test_timer;
struct_PWM_Config_t pwm1_cfg,pwm2_cfg;

//struct_PWM_DAC_Config_t DAC_Config;
struct_PWM_Complementary_Config_t cpl_pwm1_cfg,cpl_pwm2_cfg;


extern uint8_t motor_buffer[];

////////////////////////////////////////////////////////
void	app_pwm_init(void)
{
		GPIO_InitTypeDef GPIO_Handle;
    __SYSTEM_PWM_CLK_ENABLE();
	
    /* init GPIO Alternate Function */
    GPIO_Handle.Pin       = PWM1_PIN;
    GPIO_Handle.Mode      = GPIO_MODE_AF_PP;
    GPIO_Handle.Pull      = GPIO_NOPULL;//GPIO_PULLUP;
    GPIO_Handle.Alternate = GPIO_FUNCTION_6;
    gpio_init(PWM1_PORT, &GPIO_Handle);
	
    GPIO_Handle.Pin       = PWM2_PIN;
    GPIO_Handle.Mode      = GPIO_MODE_AF_PP;
    GPIO_Handle.Pull      = GPIO_NOPULL;//GPIO_PULLUP;
    GPIO_Handle.Alternate = GPIO_FUNCTION_6;
    gpio_init(PWM2_PORT, &GPIO_Handle);
	
	
            //--------------------------//
            //         PWM Mode         //
            //--------------------------//
	
    pwm1_cfg.Prescale = 1000;
    pwm1_cfg.Period   = 100;
    pwm1_cfg.Posedge  = 100;//ÉÏÉý
    pwm1_cfg.Negedge  = 0;//ÏÂ½µ
    pwm_config(PWM1_CHANNEL, pwm1_cfg);
    pwm_output_enable(PWM1_CHANNEL);
    //co_printf("pwm1 init\r\n");
		
    pwm2_cfg.Prescale = 1000;
    pwm2_cfg.Period   = 100;
    pwm2_cfg.Posedge  = 100;
    pwm2_cfg.Negedge  = 0;
    pwm_config(PWM2_CHANNEL, pwm2_cfg);
    pwm_output_enable(PWM2_CHANNEL);
    //co_printf("pwm2 init\r\n");
}

////////////////////////////////////////////////////////
void	app_pwm_deinit(void)
{
		
		GPIO_InitTypeDef GPIO_Handle;
    __SYSTEM_PWM_CLK_DISABLE();
    pwm_output_disable(PWM1_CHANNEL|PWM2_CHANNEL);
    // Output 
    GPIO_Handle.Pin       = PWM1_PIN;
    GPIO_Handle.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_init(PWM1_PORT, &GPIO_Handle);
    gpio_write_pin(PWM1_PORT, PWM1_PIN, GPIO_PIN_CLEAR);
	
    GPIO_Handle.Pin       = PWM2_PIN;
    GPIO_Handle.Mode      = GPIO_MODE_OUTPUT_PP;
    gpio_init(PWM2_PORT, &GPIO_Handle);
    gpio_write_pin(PWM2_PORT, PWM2_PIN, GPIO_PIN_CLEAR);
	
		motor.num=0;
		motor.ontime=50;
		motor.offtime=10;
		motor.on=1;
		pwm1_update(0);
		pwm2_update(0);
	
	
    //co_printf("pwm deinit\r\n");
}

void	pwm1_update(uint8_t duty)
{
	if(duty)
	{
		if(duty>pwm1_cfg.Period)
		{
			pwm1_cfg.Posedge=0;
			pwm1_cfg.Negedge=pwm1_cfg.Period-1;
		}
		else
		{
			pwm1_cfg.Posedge=0;
			pwm1_cfg.Negedge  = duty;
		}
		//co_printf("MT1:%d \r\n", duty);
	}
	else
	{
		pwm1_cfg.Posedge=100;
		pwm1_cfg.Negedge  = 0;

	}
	pwm_config(PWM1_CHANNEL, pwm1_cfg);
	pwm_output_enable(PWM1_CHANNEL);

}

void	pwm2_update(uint8_t duty)
{
	if(duty)
	{	
		if(duty>pwm2_cfg.Period)
		{
			pwm2_cfg.Posedge=0;
			pwm2_cfg.Negedge=pwm2_cfg.Period-1;
		}
		else
		{
			pwm2_cfg.Posedge=0;
			pwm2_cfg.Negedge  = duty;
		}
				
	}
	else
	{
		pwm2_cfg.Posedge=100;
		pwm2_cfg.Negedge  = 0;
	}
	pwm_config(PWM2_CHANNEL, pwm2_cfg);
	pwm_output_enable(PWM2_CHANNEL);
	
}


void	motor1_test(uint8_t on_time)
{			
	//os_timer_init(&motor1_test_timer, motor1_test_timer_handler, NULL); 
	//os_timer_start(&motor1_test_timer, on_time, false);
}

void	motor2_test(uint8_t on_time)
{			
	//os_timer_init(&motor2_test_timer, motor2_test_timer_handler, NULL); 
	//os_timer_start(&motor2_test_timer, on_time, false);
}





void	motor_test(void)
{
	delaySecToSleep=0;
	pwm1_update(100);
	pwm2_update(100);
}

void	motor_stop(void)
{
	motor.num=0;
	pwm1_update(0);
	pwm2_update(0);
}

////////////////////////////////////////////////////////////
/*
		uint8_t strength;
		uint8_t num;
		uint8_t ontime;
		uint8_t offtime;
		uint8_t on;
		uint8_t ontime_temp;
		uint8_t offtime_temp;
*/


void	motor_init(uint8_t strength,uint8_t num,uint8_t ontime,uint8_t offtime,uint8_t on)
{
	if(motor.num==0)
	{
		motor.strength=strength;
		motor.num=num;
		motor.ontime=ontime;
		motor.offtime=offtime;
		motor.on=on;
		motor.ontime_temp=motor.ontime;
		motor.offtime_temp=motor.offtime;
		pwm1_update(0);
		pwm2_update(0);
	}
}


/*
void	motor_load(void)
{
	if(fgCharge==0)
	{
		
	
	}
	else
	{
			motor_stop();
			LOCK=0;
	}
}
*/
void	motor_task(struct_motor_t * pmotor)
{
	if(pmotor->num)
	{
		idleCnt=0;
		if(pmotor->on)
		{
			if(pmotor->ontime_temp)
			{
				pmotor->ontime_temp--;
				
				if(pmotor->ontime_temp==0)
				{
					pmotor->on=0;
					pmotor->offtime_temp=pmotor->offtime;
					pmotor->num--;
					co_printf("MT:%d \r\n", pmotor->num);
					pwm1_update(0);
					pwm2_update(0);
				}
			}
		}
		else
		{
			if(pmotor->offtime_temp)
			{
				pmotor->offtime_temp--;
				if(pmotor->offtime_temp==0)
				{
					pmotor->on=1;
					pmotor->ontime_temp=pmotor->ontime;
					pwm1_update(pmotor->strength);
					pwm2_update(pmotor->strength);
				}
			}
		}
	}
	else
	{
		//LED1_SET0;
		pwm1_update(0);
		pwm2_update(0);
	}
}







