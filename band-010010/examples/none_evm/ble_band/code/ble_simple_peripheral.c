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
#include <stdbool.h>

#include "co_log.h"

#include "gap_api.h"
#include "gatt_api.h"

#include "os_timer.h"
#include "os_mem.h"

#include "ota_service.h"
#include "dev_info_service.h"
#include "batt_service.h"
#include "simple_gatt_service.h"

#include "ble_simple_peripheral.h"
#include "driver_gpio.h"
#include "driver_efuse.h"

#include "ANCS_client.h"

#include "app.h"

 //uint8_t  ANCS_SVC[16]={0xd0, 0x00, 0x2d, 0x12, 0x1e, 0x4b, 0x0f, 0xa4, \
                                            0x99, 0x4e, 0xce, 0xb5, 0x31,0xf4, 0x05, 0x79};
uint8_t ATT_SVC_ANCS_UUID128[16]=  {0xD0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0xA4, \
                          0x99, 0x4E, 0xCE, 0xB5, 0x31, 0xF4, 0x05, 0x79};




rf_state_t rf_state;

uint8_t slave_link_conidx;
uint8_t master_link_conidx;
uint8_t tick = 1;
uint8_t fg_power_off;
uint8_t sleep_param=0;
os_timer_t update_timer;


void connect_duty_update(void);

void connect_duty_update(void)
{
    gap_conn_param_update(slave_link_conidx, 12, 24, 24, 600);
	//gap_conn_param_update(slave_link_conidx, 12, 12, 12, 500);
}


os_timer_t update_timer;
void updata_tim_fn(void *arg)
{
	gap_conn_param_update(slave_link_conidx, 6, 24, 24, 600);
   // gap_conn_param_update(slave_link_conidx, 24, 24, 33, 600);
}



/*
 * MACROS (宏定义)
 */
#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL        (LOG_LEVEL_INFO)
extern const char *app_tag;

/*
 * CONSTANTS (常量定义)
 */

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
/*
						0x02, // length
					  0x01, // AD Type: Flags
					  0x06, // LE General Discoverable Mode & BR/EDR Not Supported
*///--------SDK load

static uint8_t adv_data[] =
{
	/*
    // service UUID, to notify central devices what services are included
    0x03,   // length of this data
    GAP_ADVTYPE_16BIT_MORE,      // some of the UUID's, but not all
    0xFE,
    0xFF,
	*/
	// appearance
    0x03,   // length of this data
    GAP_ADVTYPE_APPEARANCE,
    LO_UINT16(GAP_APPEARE_GENERIC_HID),
    HI_UINT16(GAP_APPEARE_GENERIC_HID),

    // service UUIDs, to notify central devices what services are included
    // in this peripheral. ??central??????, ??o???h????.
    0x03,   // length of this data
    GAP_ADVTYPE_16BIT_COMPLETE,
    LO_UINT16(HID_SERV_UUID),
    HI_UINT16(HID_SERV_UUID),
};

// GAP - Scan response data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
// GAP-Scan response内容,最长31个字节.短一点的内容可以节省广播时的系统功耗.



uint8_t scan_rsp_data_size;
uint8_t scan_rsp_data[31];

uint8_t dev_name[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t dev_name_temp[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	
uint8_t dev_name_size,dev_name_temp_size;
uint8_t fg_rename_done;
uint8_t local_name[13] = {'S','a','f','e','w','a','v','e',' ','B','a','n','d'};

//uint8_t local_name[26] = {'S','a','f','e','w','a','v','e',' ','B','a','n','d','S','a','f','e','w','a','v','e',' ','B','a','n','d'};

/*
static uint8_t scan_rsp_data[] =
{
    // complete name 设备名字
    0x12,   // length of this data
    GAP_ADVTYPE_LOCAL_NAME_COMPLETE,
    'S','i','m','p','l','e',' ','P','e','r','i','p','h','e','r','a','l',

    // Tx power level 发射功率
    0x02,   // length of this data
    GAP_ADVTYPE_POWER_LEVEL,
    0,      // 0dBm
};
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


 
/*
 * LOCAL FUNCTIONS (本地函数)
 */
 void sp_start_adv(void);
void sp_start_white(void);

/** @function group ble peripheral device APIs (ble外设相关的API)
 * @{
 */

/*********************************************************************
 * @fn      app_gap_evt_cb
 *
 * @brief   Application layer GAP event callback function. Handles GAP evnets.
 *
 * @param   p_event - GAP events from BLE stack.
 *       
 *
 * @return  None.
 */
void app_gap_evt_cb(gap_event_t *p_event)
{
    switch(p_event->type)
    {
        case GAP_EVT_ADV_END:
        {
            //LOG_INFO(app_tag, "adv_end,status:0x%02x\r\n",p_event->param.adv_end.status);
        }
        break;
        
        case GAP_EVT_ALL_SVC_ADDED:
        {
          LOG_INFO(app_tag, "added\r\n");
					idleCnt=0;
					fg_bat_ntf=1;
					if(fg_power_off==0)
					{
						gap_stop_advertising();
						sp_start_adv();
						rf_state=RF_ADV;
						forceUpdate=1;
					}
        }
        break;

        case GAP_EVT_SLAVE_CONNECT:
        {
            //LOG_INFO(app_tag, "slave[%d],connect. link_num:%d\r\n",p_event->param.slave_connect.conidx,gap_get_connect_num());
					  //show_reg(p_event->param.slave_connect.peer_addr.addr,6,1);
            //slave_link_conidx = p_event->param.slave_connect.conidx;
						rf_state=RF_CONNECT;
						fg_bat_ntf=1;
						idleCnt=0;
            //gap_security_req(p_event->param.slave_connect.conidx);
					
					
						slave_link_conidx = p_event->param.slave_connect.conidx;
            gatt_mtu_exchange_req(p_event->param.slave_connect.conidx);
            gap_security_req(p_event->param.slave_connect.conidx);
					
					
        }
        break;

        case GAP_EVT_DISCONNECT:
        {
            //LOG_INFO(app_tag, "Link[%d] disconnect,reason:0x%02X\r\n",p_event->param.disconnect.conidx,p_event->param.disconnect.reason);
						//
						if(fg_power_off==0)
						{
							idleCnt=0;
							forceUpdate=1;
							gap_stop_advertising();
							sp_start_adv();
							rf_state=RF_ADV;
						}
        }
        break;

        case GAP_EVT_LINK_PARAM_REJECT:
            LOG_INFO(app_tag, "Link[%d]param reject,status:0x%02x\r\n"
                      ,p_event->param.link_reject.conidx,p_event->param.link_reject.status);
            break;

        case GAP_EVT_LINK_PARAM_UPDATE:
            LOG_INFO(app_tag, "Link[%d]param update,interval:%d,latency:%d,timeout:%d\r\n",p_event->param.link_update.conidx
                      ,p_event->param.link_update.con_interval,p_event->param.link_update.con_latency,p_event->param.link_update.sup_to);
            break;

        case GAP_EVT_PEER_FEATURE:
            LOG_INFO(app_tag, "peer[%d] feats ind\r\n",p_event->param.peer_feature.conidx);
            break;

        case GAP_EVT_MTU:
            LOG_INFO(app_tag, "mtu update,conidx=%d,mtu=%d\r\n"
                      ,p_event->param.mtu.conidx,p_event->param.mtu.value);
            break;
        
        case GAP_EVT_LINK_RSSI:
            LOG_INFO(app_tag, "link rssi %d\r\n",p_event->param.link_rssi);
            break;
                
        case GAP_SEC_EVT_SLAVE_ENCRYPT:
            LOG_INFO(app_tag, "slave[%d]_encrypted\r\n",p_event->param.slave_encrypt_conidx);
            gatt_discovery_peer_svc(ANCS_client_id,p_event->param.slave_connect.conidx,16,ATT_SVC_ANCS_UUID128);
				
				    //os_timer_init(&update_timer,updata_tim_fn,NULL);
            //if(gap_security_get_bond_req())
                //os_timer_start(&update_timer,40000,0);
            //else
                //os_timer_start(&update_timer,30000,0);
            //break;
						//os_timer_init(&update_timer,updata_tim_fn,(void *)(p_event->param.slave_connect.conidx));
				
				break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      sp_start_adv
 *
 * @brief   Set advertising data & scan response & advertising parameters and start advertising
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void sp_start_adv(void)
{
    // Set advertising parameters
	
    gap_adv_param_t adv_param;
	  adv_param.adv_mode = GAP_ADV_MODE_UNDIRECT;
    adv_param.adv_addr_type = GAP_ADDR_TYPE_PUBLIC;
    adv_param.adv_chnl_map = GAP_ADV_CHAN_ALL;
    adv_param.adv_filt_policy = GAP_ADV_ALLOW_SCAN_ANY_CON_ANY;
    adv_param.adv_intv_min = 80;
    adv_param.adv_intv_max = 80;
    gap_set_advertising_param(&adv_param);
	
	
	
	/*
    gap_adv_param_t adv_param;
    adv_param.adv_mode = GAP_ADV_MODE_EXTEND_CONN_UNDIRECT;
		adv_param.disc_mode = GAP_ADV_DISC_MODE_NON_DISC;
		adv_param.adv_addr_type = GAP_ADDR_TYPE_PUBLIC; 
		adv_param.adv_chnl_map = GAP_ADV_CHAN_ALL; 
		adv_param.adv_filt_policy = GAP_ADV_ALLOW_SCAN_ANY_CON_ANY; 
		adv_param.adv_intv_min = 200; 
		adv_param.adv_intv_max = 200;
		adv_param.phy_mode = GAP_PHY_CODED; //GAP_PHY_2MBPS 
		adv_param.adv_sid = 0x2; 
    gap_set_advertising_param(&adv_param);
	*/
	
    // Set advertising data & scan response data
    gap_set_advertising_data(adv_data, sizeof(adv_data));//周期广播数据
	
		uint8_t i=0,j=0;
		scan_rsp_data_size=0;
		//scan_rsp_data[j++]=3;// length
		//scan_rsp_data[j++]=0XFF;// AD Type: MANUFACTURER SPECIFIC DATA
		//scan_rsp_data[j++]='S';
		//scan_rsp_data[j++]='B';
	
		//scan_rsp_data_size+=4;
		scan_rsp_data_size+=dev_name_size+2;
		scan_rsp_data[j++]=dev_name_size+1;
		scan_rsp_data[j++]=GAP_ADVTYPE_LOCAL_NAME_COMPLETE;
		for(i=0;i<dev_name_size;i++)
		{
			scan_rsp_data[j++]=dev_name[i];
		}
    gap_set_advertising_rsp_data(scan_rsp_data, scan_rsp_data_size);//改成为动态长///
    LOG_INFO(app_tag, "Start advertising...\r\n");
    gap_start_advertising(0);
		
		forceUpdate=1;
		idleCnt=0;
}

/*********************************************************************
 * @fn      simple_peripheral_init
 *
 * @brief   Initialize simple peripheral profile, BLE related parameters.
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void simple_peripheral_init(void)
{
    // set local device name
		//uint8_t i;
		//gap_dev_name_set(local_name, sizeof(local_name));
		//dev_name_size=sizeof(local_name);
		//memcpy(dev_name,local_name,dev_name_size);
		device_name_init();
		gap_dev_name_set(dev_name, dev_name_size);
		appID_list_init();
    // Initialize security related settings.
    gap_security_param_t param =
    {
        .mitm = false,
        .ble_secure_conn = false,
        .io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ,
        .bond_auth = true,
        .password = 0,
    };
    
    gap_security_param_init(&param);
 
    gap_set_cb_func(app_gap_evt_cb);
    
    mac_addr_t addr;
    enum ble_addr_type addr_type;
    gap_address_get(&addr, &addr_type);
  
    // Adding services to database
		
		//dis_gatt_add_service();删除设备服务
    //batt_gatt_add_service();

		ANCS_gatt_add_client();	
    sp_gatt_add_service(); 
    ota_gatt_add_service();	
	
}

