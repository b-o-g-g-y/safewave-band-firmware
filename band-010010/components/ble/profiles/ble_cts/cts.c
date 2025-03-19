#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"
#include "sys_utils.h"

#include "driver_system.h"
#include "cts_service.h"

//Cycling Spend and Cadence gatt profile uuid : 0x1816
const uint8_t cts_svc_uuid[] = UUID16_ARR(CURRENT_TIME_SERV_UUID);

local_time_info_t   local_time_info;

const gatt_attribute_t cts_att_table[CTS_ATTRIBUTE_NB] = {
	[CTS_SVC_ATTRIBUTE] = {
							{UUID_SIZE_2,UUID16_ARR(GATT_PRIMARY_SERVICE_UUID)},
							GATT_PROP_READ,
							UUID_SIZE_2,
							(uint8_t*)cts_svc_uuid,
							},
	
	//Characteristic 1 Declaration
	[CTS_CURRENT_TIME_CHAR] = {
								{UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
								GATT_PROP_READ,
								0,
								NULL,
								},
	//Characteristic 1 Value
	[CTS_CURRENT_TIME_VAL]  = {
								{UUID_SIZE_2,UUID16_ARR(CURRENT_TIME_UUID)},
								GATT_PROP_NOTI | GATT_PROP_READ | GATT_PROP_WRITE,
								SERVICE_DATA_VAL_LEN,
								NULL,
                              },
    //characteristic 1 Description					                           
    [CTS_CURRENT_TIME_CFG]   = { 
								{UUID_SIZE_2,UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
								GATT_PROP_READ |GATT_PROP_WRITE,
								0x02,
								NULL,
								},
	//Characteristic 1 Description
	[CTS_CURRENT_TIME_DESC] = {
								{UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
								GATT_PROP_READ,
								SERVICE_DATA_DESC_LEN,
								(uint8_t*)CURRENT_TIME,
								},
	
	//Characteristic 2 Declaration
	[CTS_LOCAL_TIME_INFO_CHAR]  = {
								{UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
								GATT_PROP_READ,
								0,
								NULL,
								},
	//Characteristic 2 Value
	[CTS_LOCAL_TIME_INFO_VAL]   = {
								{UUID_SIZE_2,UUID16_ARR(LOCAL_TIME_INFO_UUID)},
								GATT_PROP_READ | GATT_PROP_WRITE,
								SERVICE_DATA_VAL_LEN,
								NULL,
								},
	//Characteristic 2 Description
	[CTS_LOCAL_TIME_INFO_DESC]  = {
								{UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
								GATT_PROP_READ,
								SERVICE_DATA_DESC_LEN,
								(uint8_t*)LOCAL_TIME_INFO,
								},
	
	//Charactristic 3 Declaration
	[CTS_REFERENCE_TIME_INFO_CHAR] = {
									{UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
									GATT_PROP_READ,
									0,
									NULL,
									},
	//Characteristic 3 value
    [CTS_REFERENCE_TIME_INFO_VAL]  = {
                                    {UUID_SIZE_2,UUID16_ARR(REF_TIME_INFO_UUID)},
                                    GATT_PROP_READ,
                                    SERVICE_DATA_VAL_LEN,
                                    NULL,
                                    },
	//Characteristic 3 Description
	[CTS_REFERENCE_TIME_INFO_DESC] = {
                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                    GATT_PROP_READ,
                                    SERVICE_DATA_DESC_LEN,
                                    (uint8_t*)REFERENCE_TIME_INFO,
                                    },
};

static uint8_t cts_svc_id = 0xff;

void cts_time_send_data(uint8_t conidx,uint8_t * data, uint8_t len)
{
	gatt_ntf_t ntf_att;
	ntf_att.att_idx = CTS_CURRENT_TIME_VAL;
	ntf_att.conidx = conidx;
	ntf_att.svc_id = cts_svc_id;
	ntf_att.data_len = MIN(len,gatt_get_mtu(conidx) - 7);
	ntf_att.p_data = data;
	
	gatt_notification(ntf_att);

}
static void cts_gatt_read_cb(uint8_t *p_read, uint16_t *len, uint16_t att_idx)
{
	switch(att_idx)
	{
		case CTS_CURRENT_TIME_VAL:
            //year
            p_read[0] = (uint8_t)((current_time.year & 0xff00) >> 8);
            p_read[1] = (uint8_t)((current_time.year & 0x00ff )>> 0);
            //date
            memcpy(&p_read[2],&(current_time.month),CTS_CURRENT_TIME_LEN-2);
            *len = CTS_CURRENT_TIME_LEN;
           // current_time_read_decode(p_read,&current_time[0],len); 
             
            co_printf("read_cb->p_read:\n");
            for(int i =0; i< *len;i++)
                co_printf(" %02x ",p_read[i]);
            co_printf("\r\n");        
		break;
        
		default:
		break;
	}
}

static void cts_gatt_write_cb(uint8_t *write_buf, uint16_t len, uint16_t att_idx)
{
    switch(att_idx)
    {
        case CTS_CURRENT_TIME_VAL :
            for(int i =0; i< len;i++)
                co_printf(" %02x ",write_buf[i]);
            co_printf("\r\n");

            current_time.year = ((uint16_t)(write_buf[1] & 0x00ff) >> 0) | ((uint16_t)(write_buf[0] & 0x00ff) << 8);
            memcpy(&current_time.month,write_buf+2,len-2);
            
            current_time.reason.maunal_time_ipdate              =(write_buf[CTS_CURRENT_TIME_LEN-1] >> 0) & 0x01;
            current_time.reason.external_reference_time_update  =(write_buf[CTS_CURRENT_TIME_LEN-1] >> 1) & 0x01;
            current_time.reason.change_of_time_zone             =(write_buf[CTS_CURRENT_TIME_LEN-1] >> 2) & 0x01;
            current_time.reason.change_of_dst                   =(write_buf[CTS_CURRENT_TIME_LEN-1] >> 3) & 0x01;
            
            co_printf("current time from client\n");
            co_printf("year-month-day:%04x,%02x,%02x, hours-minutes-second:%02x,%02x,%02x, weekday:%02x\r\n",current_time.year,
                                current_time.month,current_time.day,current_time.hours,current_time.minutes,current_time.seconds,current_time.weekday);
        break;     
        
        case CTS_LOCAL_TIME_INFO_VAL :
            co_printf("local time information include of time zone and dst offset\n");
            memcpy(&local_time_info,write_buf,len);
        default:
		break;
    }
}

static uint16_t cts_svc_msg_handler(gatt_msg_t* p_msg)
{
	switch(p_msg->msg_evt){
		case GATTC_MSG_READ_REQ:  
			cts_gatt_read_cb((uint8_t *)(p_msg->param.msg.p_msg_data),&(p_msg->param.msg.msg_len),p_msg->att_idx);
		break;
        
        case GATTC_MSG_WRITE_REQ:
			cts_gatt_write_cb((uint8_t *)(p_msg->param.msg.p_msg_data),(p_msg->param.msg.msg_len), p_msg->att_idx);
		break;
        
		default:
		break;
	}
	
	return p_msg->param.msg.msg_len;
}

void cts_add_service(void)
{
	static gatt_service_t service;
	service.p_att_tb = cts_att_table;
	service.att_nb = CTS_ATTRIBUTE_NB;
	service.gatt_msg_handler = cts_svc_msg_handler;
	cts_svc_id = gatt_add_service(&service);
}

void init_current_time()
{
    current_time.year = 2021;
    current_time.month = 7;
    current_time.day = 8;
    current_time.hours = 19;
    current_time.minutes = 7;
    current_time.seconds =36;
    current_time.weekday = TUESDAY;

}
