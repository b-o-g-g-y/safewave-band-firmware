#ifndef CTS_SERVICE_H
#define CTS_SERVICE_H

#include <stdio.h>
#include <stdint.h>
#include "gatt_api.h"


#define CTS_CURRENT_TIME_LEN    10 
#define SERVICE_DATA_VAL_LEN	20
#define SERVICE_DATA_DESC_LEN	20

//characteristic description
#define CURRENT_TIME				"Current Time"
#define LOCAL_TIME_INFO				"Local Time Information"
#define REFERENCE_TIME_INFO			"Reference Time Information"

//wenkday
enum{
    MONDAY = 1,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    SUNDAY,
};

//reason param
__packed typedef struct current_time_reason
{
    uint8_t maunal_time_ipdate              :1;
    uint8_t external_reference_time_update  :1;
    uint8_t change_of_time_zone             :1;
    uint8_t change_of_dst                   :1;
}cts_reason_t;

__packed typedef struct
{
    uint16_t        year;
    uint8_t         month;
    uint8_t         day;
    uint8_t         hours;
    uint8_t         minutes;
    uint8_t         seconds;
    uint8_t         weekday;
    uint8_t         fraction;
    cts_reason_t    reason;
}current_time_t;    

//local time information
__packed typedef struct 
{
    uint8_t time_zone;
    uint8_t dst_offset;  
}local_time_info_t;

enum 
{
	CTS_SVC_ATTRIBUTE,
	
	CTS_CURRENT_TIME_CHAR,
	CTS_CURRENT_TIME_VAL,
    CTS_CURRENT_TIME_CFG,
	CTS_CURRENT_TIME_DESC,
	
	CTS_LOCAL_TIME_INFO_CHAR,
	CTS_LOCAL_TIME_INFO_VAL,
	CTS_LOCAL_TIME_INFO_DESC,
	
	CTS_REFERENCE_TIME_INFO_CHAR,
	CTS_REFERENCE_TIME_INFO_VAL,
	CTS_REFERENCE_TIME_INFO_DESC,

	CTS_ATTRIBUTE_NB,
};

//structure of current time
extern current_time_t current_time;
//structure oflocal time information
extern local_time_info_t   local_time_info;

void cts_measurement_send_data(uint8_t conidx,uint8_t * data, uint8_t len);
void init_current_time();
void cts_add_service(void);

#endif


