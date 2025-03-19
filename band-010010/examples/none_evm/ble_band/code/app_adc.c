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
#include "driver_adc.h"
#include "driver_gpio.h"
#include "app.h"
#include "band.h"

extern uint8_t fg_sleep;


uint8_t batAdcCnt,chargeCnt;
uint8_t fgCharge,preFgCharge,forceUpdate,fgPowerLow;
uint16_t Vbat,bakVbat,adc_cnt;
uint32_t Vtemp;
uint8_t fg_bat_ntf,lowBatCnt;

/////////////////////////////////////////////////////////
void app_adc_init(void)
{
		//adc ADC_SINGLE_CHANNEL_USE_FIFO
		
		GPIO_InitTypeDef    GPIO_Handle;
    adc_InitParameter_t ADC_InitParam;
    
    __SYSTEM_ADC_CLK_ENABLE();
    
						GPIO_Handle.Pin       = ADC_PIN;
            GPIO_Handle.Mode      = GPIO_MODE_AF_PP;
            GPIO_Handle.Pull      = GPIO_NOPULL;
            GPIO_Handle.Alternate = GPIO_FUNCTION_8;
            gpio_init(GPIO_D, &GPIO_Handle);

            ADC_InitParam.ADC_CLK_DIV          = 2000;
            ADC_InitParam.ADC_SetupDelay       = 1000;
            ADC_InitParam.ADC_Reference        = ADC_REF_LDOIO;
            ADC_InitParam.FIFO_Enable          = FIFO_DISABLE;
            ADC_InitParam.FIFO_AlmostFullLevel = 2;

            adc_init(ADC_InitParam);

            adc_Channel_ConvertConfig(ADC_CHANNEL_1);

            adc_convert_enable();
/*
            while(1)
            {
                if (!(adc_get_fifo_Status() & FIFO_EMPTY))
                {
									bakVbat=adc_get_data();
                  co_printf("ADC conversion value = %d \r\n", adc_get_data());
                }
            }
            {
                if (!(adc_get_fifo_Status() & FIFO_EMPTY))
                {
									bakVbat=adc_get_data();
                  co_printf("ADC conversion value = %d \r\n", adc_get_data());
                }
            }
						*/
	
	
	
	/*
						GPIO_Handle.Pin       = ADC_PIN;
            GPIO_Handle.Mode      = GPIO_MODE_AF_PP;
            GPIO_Handle.Pull      = GPIO_NOPULL;
            GPIO_Handle.Alternate = GPIO_FUNCTION_8;
            gpio_init(ADC_PORT, &GPIO_Handle);
            
            ADC_InitParam.ADC_CLK_DIV    = 2000;//5000
            ADC_InitParam.ADC_SetupDelay = 1000;
            ADC_InitParam.ADC_Reference  = ADC_REF_LDOIO;
            ADC_InitParam.FIFO_Enable    = FIFO_DISABLE;

            adc_init(ADC_InitParam);
						adc_Channel_ConvertConfig(ADC_CHANNEL_1);
            adc_convert_enable();
						
						
            
            while(1)
            {
							
							
                for (int i = 0; i < 2; i++)
                {
                    co_printf("Channel[%d] = %d \r\n", i, adc_get_channel_data(i));
                }
            }
*/
}

void app_adc_deinit(void)
{
		GPIO_InitTypeDef    GPIO_Handle;
    __SYSTEM_ADC_CLK_DISABLE();
			//input
    GPIO_Handle.Pin       = ADC_PIN;
    GPIO_Handle.Mode      = GPIO_MODE_INPUT;
    GPIO_Handle.Pull      = GPIO_NOPULL;
    gpio_init(KEY1_PORT, &GPIO_Handle);
    gpio_read_pin(ADC_PORT, KEY1_PIN);
}

void charge_task(void)
{
		preFgCharge=fgCharge;
		if(READ_VIN){fgCharge=1;}else{fgCharge=0;}
		//²åÈë´¦Àí
		if((preFgCharge==0)&&(fgCharge==1))
		{
			idleCnt=0;
			forceUpdate=1;
			fgPowerLow=0;
		}
		if((preFgCharge==1)&&(fgCharge==0))
		{
			forceUpdate=1;
		}
	
		if(fgCharge)
		{
			if(READ_CHARGE_END)
			{
				LED1_SET0;
			}
			else
			{
				chargeCnt++;
				if(chargeCnt&0x40){LED1_SET1;}else{LED1_SET0;}
			}
		}
		else
		{
			//LED1_SET0;
		}
		pre_battery_state[1]=battery_state[1];
		battery_state[1]=fgCharge;
}


unsigned int  lowBatLEDCnt;
static uint16_t temp;
//adc_task runing per 10ms 
void adc_task(void)
{
		lowBatLEDCnt++;
		if((fgCharge==0)&&(fgPowerLow==1))
		{
				if(lowBatLEDCnt>3000)
				{
					lowBatLEDCnt=0;
				}
				if((lowBatLEDCnt==50)||(lowBatLEDCnt==250))
				{
					LED1_SET1;
				}
				if((lowBatLEDCnt==150)||(lowBatLEDCnt==350))
				{
					LED1_SET0;
				}
		}
		adc_cnt++;
		
    if(adc_cnt>100)//1s
    {
				adc_cnt=0;
				adc_convert_enable();
				temp=adc_get_channel_data(1);
				bakVbat=Vbat;
				Vbat=temp*585/1024;//2*3.00V-LDO
				//if(fg_sleep==0){co_printf("VBAT:%d \r\n", Vbat);}
				forceUpdate=1;
				if(fgCharge)
				{
					if(forceUpdate==0)
					{
						if(bakVbat>Vbat){Vbat=bakVbat;}	
						
					}
					else{bakVbat=Vbat;forceUpdate=0;}
				}
				else
				{
					if(forceUpdate==0)
					{
						if(Vbat>bakVbat){Vbat=bakVbat;}
					}
					else{bakVbat=Vbat;forceUpdate=0;}
				}
				
				
				pre_battery_state[0]=battery_state[0];
				//co_printf("V_BAT_valid:%d \r\n", Vbat);
				if(Vbat<CON_BAT_MIN)
				{
					battery_state[0]=0;
					lowBatCnt++;
					if(lowBatCnt>10){fgPowerLow=1;}
				}
				else
				{
					lowBatCnt=0;
					fgPowerLow=0;
					Vtemp=(Vbat-CON_BAT_MIN)*100;
					Vtemp=Vtemp/(CON_BAT_MAX-CON_BAT_MIN);
					if(Vtemp<=100)
					{
						battery_state[0]=Vtemp;
					}
					else
					{
						battery_state[0]=100;
					}
				}
				if(rf_state==RF_CONNECT)
				{
					if((pre_battery_state[0]!=battery_state[0])||(pre_battery_state[1]!=battery_state[1])||(fg_bat_ntf==1))
					{
						fg_bat_ntf=0;
						//if(bat_ccc[0])
						{
							battery_state[2]=Version>>24;
							battery_state[3]=(Version&0xffffff)>>16;
							battery_state[4]=(Version&0xffff)>>8;
							battery_state[5]=Version&0xff;
							
							ntf_data(0,SP_IDX_BAT_VALUE,battery_state,sizeof(battery_state));
							
											if(fg_sleep==0)
											{
												co_printf("BLevel:%d \r\n", battery_state[0]);
												co_printf("Bstate:%d \r\n", battery_state[1]);
											}
						}
					}
				}
    }
}







