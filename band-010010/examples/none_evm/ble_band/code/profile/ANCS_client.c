/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 * ANCS client profile. protocol websit:
 * https://developer.apple.com/library/archive/documentation/CoreBluetooth/Reference/AppleNotificationCenterServiceSpecification
 * /Specification/Specification.html#//apple_ref/doc/uid/TP40013460-CH1-SW14
 *
 */

/*
 * INCLUDES (包含头文件)
 */
#include <stdio.h>
#include <string.h>

#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"
#include "sys_utils.h"
#include "ANCS_client.h"
#include "os_timer.h"
#include "app.h"

/*
 * MACROS (宏定义)
 */

/*
 * CONSTANTS (常量定义)
 */

/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */


/*
 * LOCAL VARIABLES (本地变量)
 */
uint8_t ANCS_client_id;

#define CTL_POINT_UUID     {0xd9, 0xd9, 0xaa, 0xfd, 0xbd, 0x9b, 0x21, 0x98, \
                                            0xa8, 0x49, 0xe1, 0x45, 0xf3,0xd8, 0xd1, 0x69}
#define NTF_SRC_UUID       {0xbd, 0x1d, 0xa2, 0x99, 0xe6, 0x25, 0x58, 0x8c, \
                                            0xd9, 0x42, 0x01, 0x63, 0x0d,0x12, 0xbf, 0x9f}
#define DATA_SRC_UUID      {0xfb, 0x7b, 0x7c, 0xce, 0x6a, 0xb3, 0x44, 0xbe, \
                                            0xb5, 0x4b, 0xd6, 0x24, 0xe9,0xc6, 0xea, 0x22}

	/*																					
	Notification Source: UUID 9FBF120D-6301-42D9-8C58-25E699A21DBD (notifiable)
Control Point: UUID 69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9 (writeable with response)
Data Source: UUID 22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB (notifiable)																					
		*/																				
/*********************************************************************
 * Profile Attributes - Table
 * 每一项都是一个characteristic attribute对应的UUID的定义。
 * 第一个是UUID的长度，
 * 每二个是UUID的值。
 */
const gatt_uuid_t ANCS_att_tb[] =
{
    [ANCS_ATT_IDX_CTL_POINT] = { UUID_SIZE_16, CTL_POINT_UUID},
    [ANCS_ATT_IDX_NTF_SRC] = { UUID_SIZE_16, NTF_SRC_UUID},
    [ANCS_ATT_IDX_DATA_SRC] = { UUID_SIZE_16, DATA_SRC_UUID},
};
static uint16_t ANCS_hdl_cache[3] = {0};
uint32_t call_notification_uid ;
uint32_t social_notification_uid ;
uint8_t msg_ok,parse_enable;

uint8_t indexID;
uint8_t IDappleBuffer[10][200];

#define EVENT_ID_NOTIFICATION_ADD       (0)      /**< The arrival of a new iOS notification on the NP */
#define EVENT_ID_NOTIFICATION_MODIFIED  (1) /**< The modification of an iOS notification on the NP */
#define EVENT_ID_NOTIFICATION_REMOVED   (2)  /**< The removal of an iOS notification on the NP */
#define EVENT_ID_NOTIFICATION_RESERVED  (0xff)

#define EVT_FLAG_SILENT         BIT(0)
#define EVT_FLAG_IMPORTANT      BIT(1)
#define EVT_FLAG_PRE_EXSITING   BIT(2)
#define EVT_FLAG_POSITIVE       BIT(3)
#define EVT_FLAG_NEGATIVE       BIT(4)

#define CATGRY_ID_OTHER         (0)
#define CATGRY_ID_INCOMING_CALL (1)
#define CATGRY_ID_MISS_CALL     (2)
#define CATGRY_ID_VOICE_MAIL    (3)
#define CATGRY_ID_SOCIAL        (4)
#define CATGRY_ID_SCHEDULE      (5)
#define CATGRY_ID_EMAIL         (6)
#define CATGRY_ID_NEWS          (7)
#define CATGRY_ID_HEALTH        (8)
#define CATGRY_ID_BUSINESS      (9)
#define CATGRY_ID_LOCATION      (10)
#define CATGRY_ID_ENTERTAINMENT (11)

struct ancs_ntf_src
{
    uint8_t event_id;
    uint8_t event_flags;
    uint8_t category_id;
    uint8_t category_cnt;
    uint32_t ntf_uid;
};

#define NTF_ATT_ID_APPLE        0
#define NTF_ATT_ID_TITLE        1
#define NTF_ATT_ID_SUBTITLE     2
#define NTF_ATT_ID_MSG          3
#define NTF_ATT_ID_MSG_SIZE     4
#define NTF_ATT_ID_DATE         5
#define NTF_ATT_ID_POSITIVE_ACT 6
#define NTF_ATT_ID_NEGATIVE_ACT 7


static os_timer_t os_timer_ancs_msg;
void os_timer_ancs_msg_cb(void *arg)
{
	parse_enable=1;
}

void ANCS_recv_ntf_src(uint8_t conidx,uint8_t *p_data, uint16_t len)
{
    if(len != 8){return;}
    struct ancs_ntf_src *ntf_src = (struct ancs_ntf_src *)p_data;
    //co_printf("id:%d,es:%x,cid:%d,cnt:%d,uid:%x\r\n",ntf_src->event_id,ntf_src->event_flags,ntf_src->category_id,ntf_src->category_cnt,ntf_src->ntf_uid);

		idleCnt=0;
		/*
		if(sleep_param)
		{
			sleep_param=0;
			gap_conn_param_update(slave_link_conidx, 6, 6, 0, 500);
		}
	*/
		if( (ntf_src->category_id == CATGRY_ID_OTHER
					|| ntf_src->category_id == CATGRY_ID_INCOMING_CALL
					|| ntf_src->category_id == CATGRY_ID_VOICE_MAIL
					|| ntf_src->category_id == CATGRY_ID_SOCIAL
					|| ntf_src->category_id == CATGRY_ID_SCHEDULE
					|| ntf_src->category_id == CATGRY_ID_EMAIL
					|| ntf_src->category_id == CATGRY_ID_NEWS
					|| ntf_src->category_id == CATGRY_ID_HEALTH
					|| ntf_src->category_id == CATGRY_ID_BUSINESS
					|| ntf_src->category_id == CATGRY_ID_LOCATION
					|| ntf_src->category_id == CATGRY_ID_ENTERTAINMENT) &&
        ((ntf_src->event_flags & EVT_FLAG_PRE_EXSITING) != 0x04))
    {
        if(ntf_src->category_id == CATGRY_ID_INCOMING_CALL)
				{
						//co_printf("incall\r\n");
            call_notification_uid = ntf_src->ntf_uid;
				}
        if(ntf_src->category_id == CATGRY_ID_SOCIAL)
				{
					 ///co_printf("social\r\n");
           social_notification_uid = ntf_src->ntf_uid;
				}
        if(ntf_src->category_id == CATGRY_ID_MISS_CALL)
				{
						//co_printf("mscall\r\n");
				}

        if(ntf_src->event_id != EVENT_ID_NOTIFICATION_REMOVED)
        {
            uint16_t max_att_len = gatt_get_mtu(conidx)-3;
            uint8_t rsp[19];
            uint8_t i = 0;
            rsp[i++] = ANCS_CMD_ID_GET_NOTIFICATION_ATTR;   //cmd id
            *(uint32_t *)(rsp + i) = ntf_src->ntf_uid;  //ntf_uid
            i+=4;

            rsp[i++] = NTF_ATT_ID_APPLE;
            rsp[i++] = NTF_ATT_ID_TITLE;
            rsp[i++] = (max_att_len & 0xff);
            rsp[i++] = (max_att_len & 0xff00)>>8;

            rsp[i++] = NTF_ATT_ID_SUBTITLE;
            rsp[i++] = (max_att_len & 0xff);
            rsp[i++] = (max_att_len & 0xff00)>>8;

            rsp[i++] = NTF_ATT_ID_MSG;
            rsp[i++] = (max_att_len & 0xff);
            rsp[i++] = (max_att_len & 0xff00)>>8;

            rsp[i++] = NTF_ATT_ID_MSG_SIZE;
            rsp[i++] = NTF_ATT_ID_DATE;
            rsp[i++] = NTF_ATT_ID_POSITIVE_ACT;
            rsp[i++] = NTF_ATT_ID_NEGATIVE_ACT;

            ANCS_gatt_write_req(conidx,ANCS_ATT_IDX_CTL_POINT,rsp,sizeof(rsp));
        }
    }
}

void saveAppID(uint8_t *p_data, uint16_t len)
{
	if( memcmp(&IDappleBuffer[indexID][1],p_data,len)!=0)//compare with the saved apple-id 
	{
		//if comes a new apple-id
		IDappleBuffer[indexID][0]=len;
		memcpy(&IDappleBuffer[indexID][1],p_data,len);//save new apple-id
		if(indexID>=9){indexID=9;}
		else{indexID++;}
		
	}
	else
	{
		if(indexID==0)
		{
			IDappleBuffer[indexID][0]=len;
			memcpy(&IDappleBuffer[indexID][1],p_data,len);//save new apple-id
			indexID++;
		}
	}
}

void parseAppID(void)
{
	unsigned char i;
	if((indexID)&&(parse_enable))
	{
		for(i=0;i<indexID;i++)
		{
			if(IDappleBuffer[i][0])
			{
				findAppID(&IDappleBuffer[i][1],IDappleBuffer[i][0]);
				IDappleBuffer[i][0]=0;
			}
		}
		indexID=0;
	}
}

void ANCS_recv_data_src(uint8_t conidx,uint8_t *p_data, uint16_t len)
{
    uint8_t *str = NULL;
    uint16_t str_len = 0;
   
    uint16_t i = 0;
    uint16_t data_len = 0;

    uint8_t event_id = p_data[i++];
    uint32_t uid = *(uint32_t *)(p_data + i);
    i+=4;
    co_printf("evt_id:%d,uid:%x,len:%d\r\n",event_id,uid,len);
		idleCnt=0;
    while(i < len)
    {
        switch(p_data[i++])
        {
            case NTF_ATT_ID_APPLE:
                data_len = *(uint16_t *)(p_data + i);
                i+=2;
								if(data_len<200)
								{
									co_printf("IDapple:");show_string(p_data + i,data_len,1);//test
									saveAppID(p_data + i,data_len);
									msg_ok=1;
								}
								else
								{
									msg_ok=0;
								}
						
                i+=data_len;
                break;
            case NTF_ATT_ID_TITLE:  //UTF-8
                data_len = *(uint16_t *)(p_data + i);
                i+=2;
                co_printf("TITLE: ");//show_utf8_hex(p_data + i,data_len,1);

                i+=data_len;
                break;
            case NTF_ATT_ID_SUBTITLE:   //UTF-8
                data_len = *(uint16_t *)(p_data + i);
                
                i+=2;
                co_printf("SUBTITLE: "); //show_utf8_hex(p_data + i,data_len,1);
                i+=data_len;
                break;
            case NTF_ATT_ID_MSG:    //UTF-8
                data_len = *(uint16_t *)(p_data + i);
                i+=2;
                co_printf("MSG: ");//show_utf8_hex(p_data + i,data_len,1);


                i+=data_len;
                break;
            case NTF_ATT_ID_MSG_SIZE:   //ASCII
                data_len = *(uint16_t *)(p_data + i);
                i+=2;
								co_printf("MSG SIZE: ");show_string(p_data + i,data_len,1);
                i+=data_len;
                break;
            case NTF_ATT_ID_DATE:       //ASCII
                data_len = *(uint16_t *)(p_data + i);
                i+=2;
								co_printf("DATE: ");show_string(p_data + i,data_len,1);
                i+=data_len;
								if((msg_ok)&&(parse_enable==0))
								{
									msg_ok=0;
									os_timer_stop(&os_timer_ancs_msg);//?
									os_timer_start(&os_timer_ancs_msg, 1000, false);//2s timer
								}
						
                break;
            case NTF_ATT_ID_POSITIVE_ACT:       //UTF-8
                data_len = *(uint16_t *)(p_data + i);
                i+=2;
                co_printf("POSITIVE: "); //show_utf8_hex(p_data + i,data_len,1);

                i+=data_len;
                break;
            case NTF_ATT_ID_NEGATIVE_ACT:       //UTF-8
                data_len = *(uint16_t *)(p_data + i);
                i+=2;
                co_printf("NEGATIVE: ");//show_utf8_hex(p_data + i,data_len,1);
                i+=data_len;
                break;
            default:
                co_printf("ERR ID\r\n");
                break;
        }
    }
    *(str + str_len) = 0;
}

/*********************************************************************
 * @fn      dis_gatt_msg_handler
 *
 * @brief   Device information gatt message handler.
 *
 *
 * @param   p_msg  - messages from GATT layer.
 *
 * @return  none.
 */
uint16_t ANCS_gatt_msg_handler(gatt_msg_t *p_msg)
{
    //co_printf("CCC:%x\r\n",p_msg->msg_evt);
		idleCnt=0;
		parse_enable=0;
    switch(p_msg->msg_evt)
    {
        case GATTC_MSG_NTF_REQ:
        {
            if(p_msg->att_idx == ANCS_ATT_IDX_CTL_POINT)
            {
                ;
            }
            else if(p_msg->att_idx == ANCS_ATT_IDX_NTF_SRC)
            {
                ANCS_recv_ntf_src(p_msg->conn_idx,p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len);
            }
            else if(p_msg->att_idx == ANCS_ATT_IDX_DATA_SRC)
            {
                ANCS_recv_data_src(p_msg->conn_idx,p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len);
            }
        }
        break;
        case GATTC_MSG_READ_IND:
        {
            ;
        }
        break;
        case GATTC_MSG_CMP_EVT:
        {
            //co_printf("op:%d done\r\n",p_msg->param.op.operation);
            if(p_msg->param.op.operation == GATT_OP_PEER_SVC_REGISTERED)
            {
                memcpy(ANCS_hdl_cache,p_msg->param.op.arg,6);
                //show_reg((uint8_t *)ANCS_hdl_cache,6,1);
                gatt_client_enable_ntf_t ntf_enable;
                ntf_enable.conidx = p_msg->conn_idx;
                ntf_enable.client_id = ANCS_client_id;
                ntf_enable.att_idx = ANCS_ATT_IDX_NTF_SRC;
                gatt_client_enable_ntf(ntf_enable);
                ntf_enable.att_idx = ANCS_ATT_IDX_DATA_SRC;
                gatt_client_enable_ntf(ntf_enable);
            }
        }
        break;
        default:
            break;
    }

    return 0;
}



/*********************************************************************
 * @fn      batt_gatt_notify
 *
 * @brief   Send batt level notification to peer.
 *
 *
 * @param   conidx  - link idx.
 *          batt_level  - battery energy percentage.
 *
 * @return  none.
 */
void ANCS_gatt_write_cmd(uint8_t conidx,enum ancs_att_idx att_idx,uint8_t *p_data, uint16_t len)
{
    gatt_client_write_t write;
    write.conidx = conidx;
    write.client_id = ANCS_client_id;
    write.att_idx = att_idx;
    write.p_data = p_data;
    write.data_len = len;
    gatt_client_write_cmd(write);
}
void ANCS_gatt_write_req(uint8_t conidx,enum ancs_att_idx att_idx,uint8_t *p_data, uint16_t len)
{
    gatt_client_write_t write;
    write.conidx = conidx;
    write.client_id = ANCS_client_id;
    write.att_idx = att_idx;
    write.p_data = p_data;
    write.data_len = len;
    gatt_client_write_req(write);
}

void ANCS_gatt_read(uint8_t conidx,enum ancs_att_idx att_idx)
{
    gatt_client_read_t read;
    read.conidx = conidx;
    read.client_id = ANCS_client_id;
    read.att_idx = att_idx;
    gatt_client_read(read);
}

void ANCS_perform_ntf_act(uint8_t conidx, uint32_t notification_uid, enum ancs_act_id action_id)
{
    if(ANCS_hdl_cache[ANCS_ATT_IDX_CTL_POINT])
    {
        uint8_t rsp[12];
        uint8_t i = 0;
        rsp[i++] = ANCS_CMD_ID_PERFORM_NOTIFICATION_ACTION;   //cmd id
        *(uint32_t *)(rsp + i) = notification_uid;  //ntf_uid
        i+=4;
        rsp[i++] = action_id;

        ANCS_gatt_write_req(conidx,ANCS_ATT_IDX_CTL_POINT,rsp,i);
    }
}




/*********************************************************************
 * @fn      batt_gatt_add_service
 *
 * @brief   Simple Profile add GATT service function.
 *          添加GATT service到ATT的数据库里面。
 *
 * @param   None.
 *
 *
 * @return  None.
 */
void ANCS_gatt_add_client(void)
{
    gatt_client_t client;

    client.p_att_tb = ANCS_att_tb;
    client.att_nb = ANCS_ATT_IDX_MAX;
    client.gatt_msg_handler = ANCS_gatt_msg_handler;
    ANCS_client_id = gatt_add_client(&client);
		parse_enable=0;
		indexID=0;
		os_timer_init(&os_timer_ancs_msg,os_timer_ancs_msg_cb,NULL);
		co_printf("ANCS_GATT_ID:%d \r\n", ANCS_client_id);

}





