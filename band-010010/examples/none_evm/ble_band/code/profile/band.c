/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */

/*
 * INCLUDES (包含头文件)
 */
#include <stdio.h>
#include <string.h>
#include "co_printf.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"

#include "band.h"//
#include "app.h"
#include "os_timer.h"

// Simple GATT Profile Service UUID: 0xFFF0
const uint8_t sp_svc_uuid[] = UUID16_ARR(SP_SVC_UUID);

extern uint8_t slave_link_conidx;
extern uint8_t fg_rename_done;
extern uint8_t dev_name[20];
extern uint8_t dev_name_temp[20];
extern uint8_t dev_name_size,dev_name_temp_size;
extern uint8_t fg_power_off;
uint8_t battery_state[6];
uint8_t pre_battery_state[6];
uint32_t temp_addr;
uint8_t fg_appid_rawdata=0;
uint16_t appid_size_temp;

extern os_timer_t os_timer_key_reset;

extern void os_timer_ota_key_cb(void *arg);



/******************************* motor defination *******************************/
#define APPID_MAX_LEN  20
uint8_t appid_buffer_temp[1024];
uint8_t nctemp[20];
#define APPID_DESC_LEN   5
const uint8_t motor_desc[APPID_DESC_LEN] = "appID";

#define APPID_CCC_LEN   2
uint8_t appid_ccc[APPID_CCC_LEN] = {0};

/******************************* system defination *******************************/
#define SYSTEM_MAX_LEN  20
uint8_t system_buffer[SYSTEM_MAX_LEN] = {0,0,0,0,0,0,0,0};
// Characteristic 3 User Description
#define SYSTEM_DESC_LEN   10
const uint8_t system_desc[SYSTEM_DESC_LEN] = "power&test";

/******************************* BAT defination *******************************/
#define BAT_MAX_LEN  2
uint8_t bat_buffer[BAT_MAX_LEN] = {0};
// Characteristic 4 client characteristic configuration
#define BAT_CCC_LEN   2
uint8_t bat_ccc[BAT_CCC_LEN] = {0,0};
// Characteristic 4 User Description
#define BAT_DESC_LEN   3
const uint8_t bat_desc[BAT_DESC_LEN] = "Bat";

/******************************* Characteristic 5 defination *******************************/
// Characteristic 5 data
#define NAME_MAX_LEN  20
uint8_t name_value[NAME_MAX_LEN] = {0};
// Characteristic 5 User Description
#define NAME_DESC_LEN   4
const uint8_t name_desc[NAME_DESC_LEN] = "Name";

/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */
uint8_t sp_svc_id = 0;
uint8_t ntf_char2_enable = 0;
uint8_t ntf_char4_enable = 0;



/*
 * LOCAL VARIABLES (本地变量)
 */
static gatt_service_t simple_profile_svc;

/*********************************************************************
 * Profile Attributes - Table
 * 每一项都是一个attribute的定义。
 * 第一个attribute为Service 的的定义。
 * 每一个特征值(characteristic)的定义，都至少包含三个attribute的定义；
 * 1. 特征值声明(Characteristic Declaration)
 * 2. 特征值的值(Characteristic value)
 * 3. 特征值描述符(Characteristic description)
 * 如果有notification 或者indication 的功能，则会包含四个attribute的定义，
 * 除了前面定义的三个，还会有一个特征值客户端配置(client characteristic configuration)。
 *
 */

const gatt_attribute_t simple_profile_att_table[SP_IDX_NB] =
{
    // Simple gatt Service Declaration
    [SP_IDX_SERVICE]                        =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_PRIMARY_SERVICE_UUID) },     /* UUID */
        GATT_PROP_READ,                                             /* Permissions */
        UUID_SIZE_2,                                                /* Max size of the value */     /* Service UUID size in service declaration */
        (uint8_t*)sp_svc_uuid,                                      /* Value of the attribute */    /* Service UUID value in service declaration */
    },

    // Characteristic 1 Declaration
    [SP_IDX_APPID_DECLARATION]          =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
        GATT_PROP_READ,                                             /* Permissions */
        0,                                                          /* Max size of the value */
        NULL,                                                       /* Value of the attribute */
    },
    // Characteristic 1 Value
    [SP_IDX_APPID_VALUE]                =   {
        { UUID_SIZE_16, APPID_UUID },                         /* UUID */
        GATT_PROP_READ | GATT_PROP_NOTI| GATT_PROP_WRITE,                           /* Permissions */
        APPID_MAX_LEN,                                         /* Max size of the value */
        NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
    },

    // Characteristic 1 client characteristic configuration
    [SP_IDX_APPID_CFG]                  =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID) },     /* UUID */
        GATT_PROP_READ | GATT_PROP_WRITE,                           /* Permissions */
        sizeof(uint16_t),                                           /* Max size of the value */
        NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
    },
		
    // Characteristic 1 User Description
    [SP_IDX_APPID_USER_DESCRIPTION]     =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
        GATT_PROP_READ,                                             /* Permissions */
        APPID_DESC_LEN,                                          /* Max size of the value */
        (uint8_t *)motor_desc,                                   /* Value of the attribute */
    },


    // Characteristic 3 Declaration
    [SP_IDX_SYSTEM_DECLARATION]          =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
        GATT_PROP_READ,                                             /* Permissions */
        0,                                                          /* Max size of the value */
        NULL,                                                       /* Value of the attribute */
    },
    // Characteristic 3 Value
    [SP_IDX_SYSTEM_VALUE]                =   {
        { UUID_SIZE_16, SYSTEM_UUID },                 						/* UUID */
        GATT_PROP_WRITE,                                            /* Permissions */
        SYSTEM_MAX_LEN,                                         /* Max size of the value */
        NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
    },
    // Characteristic 3 User Description
    [SP_IDX_SYSTEM_USER_DESCRIPTION]     =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
        GATT_PROP_READ,                                             /* Permissions */
        SYSTEM_DESC_LEN,                                          /* Max size of the value */
        (uint8_t *)system_desc,                                   /* Value of the attribute */
    },


    // Characteristic 4 Declaration
    [SP_IDX_BAT_DECLARATION]          =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
        GATT_PROP_READ,                                             /* Permissions */
        0,                                                          /* Max size of the value */
        NULL,                                                       /* Value of the attribute */
    },
    // Characteristic 4 Value
    [SP_IDX_BAT_VALUE]                =   {
        { UUID_SIZE_16, BAT_UUID },                 /* UUID */
        GATT_PROP_READ | GATT_PROP_NOTI,                           /* Permissions */
        SYSTEM_MAX_LEN,                                         /* Max size of the value */
        NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
    },
    // Characteristic 4 client characteristic configuration
    [SP_IDX_BAT_CFG]                  =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID) },     /* UUID */
        GATT_PROP_READ | GATT_PROP_WRITE,                           /* Permissions */
        sizeof(uint16_t),                                           /* Max size of the value */
        NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
    },
    // Characteristic 4 User Description
    [SP_IDX_BAT_USER_DESCRIPTION]     =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
        GATT_PROP_READ,                                             /* Permissions */
        BAT_DESC_LEN,                                          /* Max size of the value */
        (uint8_t *)bat_desc,                                   /* Value of the attribute */
    },


    // Characteristic 5 Declaration
    [SP_IDX_NAME_DECLARATION]          =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
        GATT_PROP_READ,                                             /* Permissions */
        0,                                                          /* Max size of the value */
        NULL,                                                       /* Value of the attribute */
    },
    // Characteristic 5 Value
    [SP_IDX_NAME_VALUE]                =   {
        { UUID_SIZE_16, NAME_UUID },                 /* UUID */
        GATT_PROP_READ | GATT_PROP_WRITE,                           /* Permissions */
        NAME_MAX_LEN,                                         /* Max size of the value */
        NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
    },
    // Characteristic 5 User Description
    [SP_IDX_NAME_USER_DESCRIPTION]     =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
        GATT_PROP_READ,                                             /* Permissions */
        NAME_DESC_LEN,                                          /* Max size of the value */
        (uint8_t *)name_desc,                                   /* Value of the attribute */
    },
};

void show_reg(uint8_t *data,uint32_t len,uint8_t dbg_on)
{
    uint32_t i=0;
    if(len == 0 || (dbg_on==0)) return;
    for(; i<len; i++)
    {
        co_printf("0x%02X,",data[i]);
    }
    co_printf("\r\n");
}


void appIDnotify(uint8_t con_idx,uint8_t *pdata,uint16_t len)
{
	 //uint16_t i=0;
	while(len>20)
	{
		ntf_data(con_idx,SP_IDX_APPID_VALUE,pdata,20);
		//show_string(pdata,20,1);
		len=len-20;
		pdata=pdata+20;
	}
	ntf_data(con_idx,SP_IDX_APPID_VALUE,pdata,len);
	//show_string(pdata,len,1);
}



void ntf_data(uint8_t con_idx,uint8_t att_idx,uint8_t *data,uint16_t len)
{
    gatt_ntf_t ntf_att;
    ntf_att.att_idx = att_idx;
    ntf_att.conidx = con_idx;
    ntf_att.svc_id = sp_svc_id;
    ntf_att.data_len = len;
    ntf_att.p_data = data;
    gatt_notification(ntf_att);
}
/*********************************************************************
 * @fn      sp_gatt_msg_handler
 *
 * @brief   Simple Profile callback funtion for GATT messages. GATT read/write
 *          operations are handeled here.
 *
 * @param   p_msg       - GATT messages from GATT layer.
 *
 * @return  uint16_t    - Length of handled message.
 */
static uint16_t sp_gatt_msg_handler(gatt_msg_t *p_msg)
{
    switch(p_msg->msg_evt)
    {
        case GATTC_MSG_READ_REQ:
        {	
						idleCnt=0;
             if(p_msg->att_idx == SP_IDX_APPID_VALUE)
            {
                //memcpy(p_msg->param.msg.p_msg_data, appid_buffer, appid_size_temp);
                //return appid_buffer_size;
            }
            else if(p_msg->att_idx == SP_IDX_APPID_CFG)
            {
                memcpy(p_msg->param.msg.p_msg_data, appid_ccc, APPID_CCC_LEN);
                return APPID_CCC_LEN;
            }
            else if(p_msg->att_idx == SP_IDX_BAT_VALUE)
            {
								memcpy(p_msg->param.msg.p_msg_data, battery_state, sizeof(battery_state));
                return sizeof(battery_state);
            }
            else if(p_msg->att_idx == SP_IDX_BAT_CFG)
            {
                memcpy(p_msg->param.msg.p_msg_data, bat_ccc, BAT_CCC_LEN);
                return BAT_CCC_LEN;
            }
            else if(p_msg->att_idx == SP_IDX_NAME_VALUE)
            {
                memcpy(p_msg->param.msg.p_msg_data, dev_name, dev_name_size);
                return dev_name_size;
            }
        }
        break;

        case GATTC_MSG_WRITE_REQ:
        {
						idleCnt=0;
            if (p_msg->att_idx == SP_IDX_APPID_VALUE)
            {
							//show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
							if(fgPowerLow==0)
							{
								if(fg_appid_rawdata==0)
								{
									memcpy(nctemp,p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len);
									if(nctemp[0]==0xff)//reset ro default appIDs
									{
										appID_list_default();
										appID_list_init();
										motor_init(100,1,20,10,0);
									}
									else if(nctemp[0]==0xfe)//notify appIDs
									{
										appIDnotify(p_msg->conn_idx,appid_buffer,appid_buffer_size-1);
									}
									if(p_msg->param.msg.msg_len==20)
									{
										if((nctemp[0]=='c')&&(nctemp[1]=='o')&&(nctemp[2]=='m')&&(nctemp[3]=='.'))//if no start flag' ',insert' '
										{
											co_printf("APPID start\r\n");
											fg_appid_rawdata=1;
											appid_buffer_temp[0]=' ';//insert a space ' '
											appid_size_temp=1;
											memcpy(&appid_buffer_temp[appid_size_temp],p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len);
											appid_size_temp+=p_msg->param.msg.msg_len;
										}
										else if((nctemp[0]==' ')&&(nctemp[1]=='c')&&(nctemp[2]=='o')&&(nctemp[3]=='m')&&(nctemp[4]=='.'))
										{
											co_printf("APPID start:\r\n");
											fg_appid_rawdata=1;
											appid_size_temp=0;
											memcpy(&appid_buffer_temp[appid_size_temp],p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len);
											appid_size_temp+=p_msg->param.msg.msg_len;
										}
									}
								}
								else
								{
										memcpy(&appid_buffer_temp[appid_size_temp],p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len);
										appid_size_temp+=p_msg->param.msg.msg_len;
								
										//Because there is no end symbol defined for the appID text,
										// when the size of “the appID text ” is An integer multiple of 20,an error will occur
										if(p_msg->param.msg.msg_len<20)
										{
											appid_buffer_temp[appid_size_temp]=';';//insert ';'
											appid_size_temp++;
											//show_string(appid_buffer_temp,appid_size_temp,1);
											unsigned i;
											for(i=0;i<appid_size_temp-1;i++)
											{
												if(appid_buffer[i]!=appid_buffer_temp[i])
												{
													appID_list_save(appid_buffer_temp,appid_size_temp);
													appID_list_init();
													co_printf("appIDs changed,save newID OK");
													break;
												}
											}
											motor_init(100,1,20,10,0);
											appIDnotify(p_msg->conn_idx,appid_buffer,appid_buffer_size-1);////notify appIDs,send back to phone
											//co_printf("APPID receive complete, size is:%d \r\n", appid_size_temp);
											fg_appid_rawdata=0;
										}
								}
							}
            }
						else if (p_msg->att_idx == SP_IDX_APPID_CFG)
            {
							/*
                co_printf("notify setting:");
                show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
								memcpy(appid_ccc,p_msg->param.msg.p_msg_data,APPID_CCC_LEN);
								if(appid_ccc[0])
								{
									ntf_data(p_msg->conn_idx,SP_IDX_APPID_VALUE,p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len);
								}*/
            }
            else if (p_msg->att_idx == SP_IDX_SYSTEM_VALUE)
            {
                co_printf("Power or test:");
               show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
								memcpy(system_buffer,p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len);
								if((system_buffer[0]==0xFF)&&(system_buffer[1]==0xAA))//power off device
								{
									power_off();//231113
								}
								else if((system_buffer[0]==0xFF)&&(system_buffer[1]==0x11))//reset ro default appIDs,
								{
									appID_list_default();
									appID_list_init();
									motor_init(100,1,20,10,0);
								}
								else if((system_buffer[0]==0xFF)&&(system_buffer[1]==0x22))//notify appIDs
								{
									appIDnotify(p_msg->conn_idx,appid_buffer,appid_buffer_size-1);
								}
								else if(system_buffer[0]=='~')//TEST ~appID
								{
									show_string(system_buffer,p_msg->param.msg.msg_len,1);
									findAppID(&system_buffer[1],p_msg->param.msg.msg_len-1);
								}
								
								else if((system_buffer[0]==0xFF)&&(system_buffer[1]==0x44))//reset to default name
								{
									device_name_default();
									fg_rename_done=1;
									gap_disconnect_req(slave_link_conidx);
								}	
								else if(system_buffer[0]==0xFE)
								{
									//if system_buffer[1]==00:Never go to sleep
									//system_buffer[1]==0x01-0xff, delay 1-255 Second goto sleep
									delaySecToSleep=system_buffer[1];
								}
								
								else if(dev_name[0]==0xFD)//read FLASH,一次256byte,for debug
								{
									//uint32_t addr;
									temp_addr=0;
									temp_addr=(system_buffer[2]<<24);
									temp_addr+=(uint32_t)(system_buffer[3]<<16);
									temp_addr+=(uint32_t)(system_buffer[4]<<8);
									temp_addr+=(uint32_t)(system_buffer[5]);
									dis_flash_data(temp_addr);
								}
								else if(system_buffer[0]<=100)
								{
									motor_init(system_buffer[0],system_buffer[1],system_buffer[2],system_buffer[3],0);//motor test
								}
            }
						/*
            else if (p_msg->att_idx == SP_IDX_CHAR4_VALUE)
            {
                co_printf("Battery level receive:");
                show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
            }*/
            else if (p_msg->att_idx == SP_IDX_BAT_CFG)
            {
                co_printf("Batt notify setting:");
                show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
                memcpy(bat_ccc,p_msg->param.msg.p_msg_data,BAT_CCC_LEN);
								if(bat_ccc[0])
								{
									ntf_data(0,SP_IDX_BAT_VALUE,battery_state,sizeof(battery_state));
								}
            }
            else if (p_msg->att_idx == SP_IDX_NAME_VALUE)
            {
                co_printf("name:");show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
								dev_name_temp_size=p_msg->param.msg.msg_len;
								memcpy(dev_name_temp,p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len);

							if(fgPowerLow==0)
							{
								uint8_t i,j=0;
								for(i=0;i<dev_name_temp_size;i++)
								{
									if(dev_name_temp[0]==0xff)
									{
										device_name_default();
										fg_rename_done=1;
										motor_init(100,1,20,10,0);
										gap_disconnect_req(slave_link_conidx);
										os_timer_init(&os_timer_key_reset,os_timer_ota_key_cb,NULL);
										os_timer_start(&os_timer_key_reset, 2000, false);//wait 2 s platform_reset
										j=1;break;
									}
									if((dev_name_temp[i]<0x20)||(dev_name_temp[i]>0x7e))
									{
										j=1;break;
									}
								}
								if(j==0)
								{
									
									for(i=0;i<dev_name_temp_size;i++)
									{
										if(dev_name_temp[i]!=dev_name[i])
										{
											dev_name_size=dev_name_temp_size;
											memcpy(dev_name,dev_name_temp,dev_name_temp_size);
											device_name_save();
											co_printf("device new name: ");show_string(dev_name,dev_name_size,1);
											gap_disconnect_req(slave_link_conidx);
											os_timer_init(&os_timer_key_reset,os_timer_ota_key_cb,NULL);
											os_timer_start(&os_timer_key_reset, 2000, false);//wait 2 s platform_reset
											break;
										}
									}
									motor_init(100,1,20,10,0);
									fg_rename_done=1;
								}
							}
            }
        }
        break;
        case GATTC_MSG_LINK_CREATE:
						idleCnt=0;
            co_printf("link_created\r\n");
            break;
        case GATTC_MSG_LINK_LOST:
						idleCnt=0;
            co_printf("BAND-link_lost\r\n");
            //ntf_char2_enable = 0;
						//ntf_char4_enable = 0;
            break;
        default:
            break;
    }
    return p_msg->param.msg.msg_len;
}

/*********************************************************************
 * @fn      sp_gatt_add_service
 *
 * @brief   Simple Profile add GATT service function.
 *          添加GATT service到ATT的数据库里面。
 *
 * @param   None.
 *
 *
 * @return  None.
 */
void sp_gatt_add_service(void)
{
    simple_profile_svc.p_att_tb = simple_profile_att_table;
    simple_profile_svc.att_nb = SP_IDX_NB;
    simple_profile_svc.gatt_msg_handler = sp_gatt_msg_handler;
    sp_svc_id = gatt_add_service(&simple_profile_svc);
		co_printf("BAND_GATT_ID:%d \r\n", sp_svc_id);
}





