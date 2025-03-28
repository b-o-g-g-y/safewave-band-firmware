
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "jump_table.h"

#include "os_mem.h"
#include "plf.h"
#include "driver_system.h"
#include "driver_flash.h"
#include "driver_wdt.h"
#include "driver_uart.h"

//#include "qspi.h"
//#include "co_utils.h"
#include "gatt_api.h"
#include "gap_api.h"

#include "sys_utils.h"
#include "crc32.h"
#include "app.h"
#include "ota.h"
#include "ota_service.h"
//#include "flash_usage_config.h"
#include "driver_system.h"

#define A_ZONE_INFO_OFFSET          0
#define B_ZONE_INFO_OFFSET          0x1000//4K
#define A_ZONE_IMAGE_OFFSET         0x2000//8K
#define B_ZONE_IMAGE_OFFSET         0x2000//8K


#ifdef OTA_CRC_CHECK
#include "os_timer.h"
uint32_t Crc32CalByByte(int crc,uint8_t* ptr, int len);
uint8_t app_otas_crc_cal(uint32_t firmware_length,uint32_t new_bin_addr,uint32_t crc_data_t);

//extern void enable_cache(uint8_t invalid_ram);
//extern void disable_cache(void);

#endif
struct app_otas_status_t
{
    uint8_t read_opcode;
    uint16_t length;
    uint32_t base_addr;
} app_otas_status;

uint8_t first_loop = true;
static uint16_t at_data_idx;
static bool ota_recving_data = false;
static uint16_t ota_recving_data_index = 0;
static uint16_t ota_recving_expected_length = 0;
static uint8_t *ota_recving_buffer = NULL;
#ifdef OTA_CRC_CHECK
static os_timer_t os_timer_ota,os_timer_ota_reset;
static uint32_t ota_addr_check,ota_addr_check_len = 0;
#endif
static uint8_t ota_state = 0;

extern uint8_t app_boot_get_storage_type(void);
extern void app_boot_load_data(uint8_t *dest, uint32_t src, uint32_t len);
#ifdef OTA_CRC_CHECK
void os_timer_ota_cb(void *arg);
void os_timer_ota_reset_cb(void *arg);
#endif

__attribute__((section("ram_code"))) uint8_t app_get_ota_state(void)
{
    return ota_state;
}

__attribute__((section("ram_code"))) void app_set_ota_state(uint8_t state_flag)
{
    if(state_flag)
        ota_state = 1;
    else
        ota_state = 0;
}

static uint32_t app_otas_get_storage_address(void)
{
    struct image_info_t *image_info = (struct image_info_t *)(QSPI0_DAC_ADDRESS+A_ZONE_INFO_OFFSET);
    return (image_info->jump_table.image_size + A_ZONE_IMAGE_OFFSET);  //partA, then return partB flash Addr
}


__attribute__((section("ram_code"))) static void app_otas_flash_opration(uint32_t dest, uint8_t *src, uint32_t len,uint8_t write_or_earse)
{
    uint16_t status;
    uint32_t protect_mask;
    uint8_t mask;
    bool volatile_or_not;

    struct image_info_t *image_info = (struct image_info_t *)(QSPI0_DAC_ADDRESS+A_ZONE_INFO_OFFSET);

    protect_mask = image_info->flash_protect_mask;
    mask = protect_mask & 0xff;
    volatile_or_not = (protect_mask & 0x100) ? true : false;

    GLOBAL_INT_DISABLE();
    if(mask != 0xff) {
        status = flash_read_status(false);
        if(volatile_or_not) {
            flash_write_status_volatile(status & (~mask), false);
        }
        else {
            flash_write_status(status & (~mask), false);
        }
    }
    if(write_or_earse)
        flash_write(dest, len, src);
    else
        flash_erase(dest, 0x1000);

    if(mask != 0xff) {
        if(volatile_or_not) {
            flash_write_status_volatile(status, false);
        }
        else {
            flash_write_status(status, false);
        }
    }
    GLOBAL_INT_RESTORE();
}
__attribute__((section("ram_code"))) static void app_otas_save_data(uint32_t dest, uint8_t *src, uint32_t len)
{
    app_otas_flash_opration(dest, src, len,1);
}
__attribute__((section("ram_code"))) static void app_otas_erase(uint32_t dest)
{
    app_otas_flash_opration(dest, NULL, 0,0);
}


void ota_clr_buffed_pkt(uint8_t conidx)
{
  //  system_latency_enable(conidx);
    first_loop = true;
}
void ota_init(uint8_t conidx)
{
    app_otas_status.read_opcode = OTA_CMD_NULL;
    first_loop = true;
    at_data_idx = 0;
#ifdef OTA_CRC_CHECK
    ota_addr_check = 0;
#endif
}
void ota_deinit(uint8_t conidx)
{
    ota_clr_buffed_pkt(conidx);
    app_set_ota_state(0);
    if(ota_recving_buffer != NULL) {
        os_free(ota_recving_buffer);
        ota_recving_buffer = NULL;
    }
}
void __attribute__((weak)) ota_change_flash_pin(void)
{
    ;
}
void __attribute__((weak)) ota_recover_flash_pin(void)
{
    ;
}
#ifdef OTA_CRC_CHECK
void __attribute__((weak)) ota_start(void)
{
	
    os_timer_init(&os_timer_ota,os_timer_ota_cb,NULL);
    os_timer_init(&os_timer_ota_reset,os_timer_ota_reset_cb,NULL);
    os_timer_start(&os_timer_ota, OTA_TIMEOUT, 0);
    co_printf("ota_start\r\n\r\n");
		motor_init(80,1,20,4,0);
}
void __attribute__((weak)) ota_stop(ota_warning_type evt_id)
{
    co_printf("ota_stop\r\n\r\n");
    os_timer_stop(&os_timer_ota);
    os_timer_destroy(&os_timer_ota);
    ota_deinit(0);
    ota_init(0);
    switch (evt_id)
    {
        case OTA_TIMOUT:
            co_printf("OTA_TIMOUT\r\n");
            break;
        case OTA_ADDR_ERROR:
            co_printf("OTA_ADDR_ERROR\r\n");
            break;
        case OTA_CHECK_FAIL:
            co_printf("OTA_CHECK_FAIL\r\n");
            break;
    };
}
void os_timer_ota_cb(void *arg)
{
    ota_stop(OTA_TIMOUT);
    co_printf("os_timer_ota_cb\r\n");
}

void os_timer_ota_reset_cb(void *arg)
{
	platform_reset(0);
    //ota_stop(OTA_TIMOUT);
    //co_printf("os_timer_ota_cb\r\n");
}

#endif
void app_otas_recv_data(uint8_t conidx,uint8_t *p_data,uint16_t len)
{
    struct app_ota_cmd_hdr_t *cmd_hdr = (struct app_ota_cmd_hdr_t *)p_data;
    struct app_ota_rsp_hdr_t *rsp_hdr;
    uint16_t rsp_data_len = (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN+OTA_HDR_RESULT_LEN);
//    uint16_t i =0 ;

    if(first_loop)
    {
        first_loop = false;
        gap_conn_param_update(conidx, 6, 6, 0, 500);
    //    system_latency_disable(conidx);
        gatt_mtu_exchange_req(conidx);
        app_set_ota_state(1);

        if(ota_recving_buffer == NULL) {
            ota_recving_buffer = os_malloc(512);
        }
#ifdef OTA_CRC_CHECK
        ota_start();
#endif
    }
		idleCnt=0;
    co_printf("app_otas_recv_data[%d]: %d, %d. %d\r\n",at_data_idx, gatt_get_mtu(conidx), len, cmd_hdr->cmd.write_data.length);
    show_reg(p_data,sizeof(struct app_ota_cmd_hdr_t),1);
#ifdef OTA_CRC_CHECK
    os_timer_stop(&os_timer_ota);
    os_timer_start(&os_timer_ota, OTA_TIMEOUT, 0);
#endif
    at_data_idx++;

    // 支持手机端将包从应用层进行拆分的功能，而不是应用层发长包，L2CAP去拆分。
    if(ota_recving_data) {
        memcpy(ota_recving_buffer+ota_recving_data_index, p_data, len);
        ota_recving_data_index += len;
        ota_recving_expected_length -= len;
        if(ota_recving_expected_length != 0) {
            return;
        }
        ota_recving_data = false;
        ota_recving_buffer[0] = OTA_CMD_WRITE_DATA;
        p_data = ota_recving_buffer;
        cmd_hdr = (struct app_ota_cmd_hdr_t *)ota_recving_buffer;
    }
    
    ota_change_flash_pin();
    //wdt_feed();

    switch(cmd_hdr->opcode)
    {
        case OTA_CMD_NVDS_TYPE:
            rsp_data_len += 1;
            break;
        case OTA_CMD_GET_STR_BASE:
            at_data_idx = 0;
            first_loop = true;
            rsp_data_len += sizeof(struct storage_baseaddr);
            break;
        case OTA_CMD_READ_FW_VER:
            rsp_data_len += sizeof(struct firmware_version);
            break;
        case OTA_CMD_PAGE_ERASE:
            rsp_data_len += sizeof(struct page_erase_rsp);
            break;
        case OTA_CMD_WRITE_DATA:
            rsp_data_len += sizeof(struct write_data_rsp);
            break;
        case OTA_CMD_READ_DATA:
            rsp_data_len += sizeof(struct read_data_rsp) + cmd_hdr->cmd.read_data.length;
            if(rsp_data_len > OTAS_NOTIFY_DATA_SIZE)
            {
                // 数据太长，不能通过notify返回，通知client采用read方式获取
                rsp_data_len = sizeof(struct read_data_rsp) + (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN+OTA_HDR_RESULT_LEN);
                app_otas_status.read_opcode = OTA_CMD_READ_DATA;
                app_otas_status.length = cmd_hdr->cmd.read_data.length;
                app_otas_status.base_addr = cmd_hdr->cmd.read_data.base_address;
            }
            break;
        case OTA_CMD_WRITE_MEM:
            rsp_data_len += sizeof(struct write_mem_rsp);
            break;
        case OTA_CMD_READ_MEM:
            rsp_data_len += sizeof(struct read_mem_rsp) + cmd_hdr->cmd.read_mem.length;
            if(rsp_data_len > OTAS_NOTIFY_DATA_SIZE)
            {
                // 数据太长，不能通过notify返回，通知client采用read方式获取
                rsp_data_len = sizeof(struct read_data_rsp) + (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN+OTA_HDR_RESULT_LEN);
                app_otas_status.read_opcode = OTA_CMD_READ_MEM;
                app_otas_status.length = cmd_hdr->cmd.read_data.length;
                app_otas_status.base_addr = cmd_hdr->cmd.read_data.base_address;
            }
            else
            {
                app_otas_status.read_opcode = OTA_CMD_NULL;
            }
            break;
        case OTA_CMD_NULL:
            memcpy(ota_recving_buffer, p_data, len);
            ota_recving_expected_length = cmd_hdr->cmd.write_data.length;
            ota_recving_data_index = len;
            ota_recving_data = true;
            ota_recover_flash_pin();
            return;
    }

    struct otas_send_rsp *req = os_malloc(sizeof(struct otas_send_rsp) + rsp_data_len);
    uint16_t base_length;

    req->conidx = conidx;
    req->length = rsp_data_len;
    rsp_hdr = (struct app_ota_rsp_hdr_t *)&req->buffer[0];
    rsp_hdr->result = OTA_RSP_SUCCESS;
    rsp_hdr->org_opcode = cmd_hdr->opcode;
    rsp_hdr->length = rsp_data_len - (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN+OTA_HDR_RESULT_LEN);

    switch(cmd_hdr->opcode)
    {
        case OTA_CMD_NVDS_TYPE:
            rsp_hdr->rsp.nvds_type = app_boot_get_storage_type() | 0x10;    // 0x10 is used to identify FR8010H
            break;
        case OTA_CMD_GET_STR_BASE:
            rsp_hdr->rsp.baseaddr.baseaddr = app_otas_get_storage_address();
#ifdef OTA_CRC_CHECK
            ota_addr_check = rsp_hdr->rsp.baseaddr.baseaddr;
#endif
            break;
        case OTA_CMD_READ_FW_VER:
            rsp_hdr->rsp.version.firmware_version = __jump_table.firmware_version;
            break;
        case OTA_CMD_PAGE_ERASE:
        {
            rsp_hdr->rsp.page_erase.base_address = cmd_hdr->cmd.page_erase.base_address;
#if 1
            ///co_printf("cur_code_addr:%x\r\n",new_bin_base);
            if(rsp_hdr->rsp.page_erase.base_address < app_otas_get_storage_address())
            {
                gap_disconnect_req(conidx);
                break;
            }
#endif
            app_otas_erase(rsp_hdr->rsp.page_erase.base_address);
        }
        break;
        case OTA_CMD_CHIP_ERASE:
            break;
        case OTA_CMD_WRITE_DATA:
        {
            rsp_hdr->rsp.write_data.base_address = cmd_hdr->cmd.write_data.base_address;
            rsp_hdr->rsp.write_data.length = cmd_hdr->cmd.write_data.length;
//write user data.
#ifdef OTA_CRC_CHECK
            if((rsp_hdr->rsp.write_data.base_address !=(ota_addr_check + ota_addr_check_len)) &&
                (rsp_hdr->rsp.write_data.base_address !=ota_addr_check)){//for OTA write addr error  no req
                co_printf("rsp_hdr->rsp.write_data.base_address = %x\r\nota_addr_check=%x,\r\nlen = %d\r\nSUM=%x\r\n",rsp_hdr->rsp.write_data.base_address,ota_addr_check,len,rsp_hdr->rsp.write_data.base_address + len);
                os_free(req);
                ota_stop(OTA_ADDR_ERROR);
                return;
            }else{
                ota_addr_check = rsp_hdr->rsp.write_data.base_address;
                ota_addr_check_len = rsp_hdr->rsp.write_data.length;
            }
#endif
            app_otas_save_data(rsp_hdr->rsp.write_data.base_address,
                               p_data + (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN)+sizeof(struct write_data_cmd),
                               rsp_hdr->rsp.write_data.length);
        }
        break;
        case OTA_CMD_READ_DATA:
            rsp_hdr->rsp.read_data.base_address = cmd_hdr->cmd.read_data.base_address;
            rsp_hdr->rsp.read_data.length = cmd_hdr->cmd.read_data.length;
            base_length = sizeof(struct read_data_rsp) + (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN+OTA_HDR_RESULT_LEN);
            if(rsp_data_len != base_length)
            {
                app_boot_load_data((uint8_t*)rsp_hdr+base_length,
                                   rsp_hdr->rsp.read_data.base_address,
                                   rsp_hdr->rsp.read_data.length);
            }
            break;
        case OTA_CMD_WRITE_MEM:
            rsp_hdr->rsp.write_mem.base_address = cmd_hdr->cmd.write_mem.base_address;
            rsp_hdr->rsp.write_mem.length = cmd_hdr->cmd.write_mem.length;
            memcpy((void *)rsp_hdr->rsp.write_mem.base_address,
                   p_data + (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN)+sizeof(struct write_data_cmd),
                   rsp_hdr->rsp.write_mem.length);
            break;
        case OTA_CMD_READ_MEM:
            rsp_hdr->rsp.read_mem.base_address = cmd_hdr->cmd.read_mem.base_address;
            rsp_hdr->rsp.read_mem.length = cmd_hdr->cmd.read_mem.length;
            base_length = sizeof(struct read_mem_rsp) + (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN+OTA_HDR_RESULT_LEN);
            if(rsp_data_len != base_length)
            {
                memcpy((uint8_t*)rsp_hdr+base_length,
                       (void *)rsp_hdr->rsp.read_mem.base_address,
                       rsp_hdr->rsp.read_data.length);
            }
            break;
        case OTA_CMD_REBOOT:
            {
                co_printf("REBOOT\r\n\r\n");
                uint32_t image_offset, image_length;
                struct image_info_t image_info_tmp = {0};
                flash_read(app_otas_get_storage_address(), sizeof(struct jump_table_t), (uint8_t *)&image_info_tmp);
                image_offset = app_otas_get_storage_address() + B_ZONE_IMAGE_OFFSET;
                image_length = image_info_tmp.jump_table.image_size - B_ZONE_IMAGE_OFFSET;
                
                image_info_tmp.compressed_info.compressed_label = 0xFFFFFFFF;
                image_info_tmp.compressed_info.compressed_image_crc = 0;

                image_info_tmp.crc.image_crc = crc32(0xffffffff, (const uint8_t *)(QSPI0_DAC_ADDRESS + image_offset), image_length);
                image_info_tmp.crc.jump_table_crc 
                    = crc32(0xffffffff, (const uint8_t *)(&image_info_tmp),(uint32_t)&(((struct jump_table_t *)0)->checkword)); 

                image_info_tmp.image_storage_offset = image_offset;
                image_info_tmp.image_length = image_length;
                
                image_info_tmp.flash_protect_mask = 0x7C;//0xFFFFFFFF;
                image_info_tmp.image_valid = JUMP_TABLE_OTA_IMAGE_VALID;
                
#ifdef OTA_CRC_CHECK
								uint32_t new_bin_base = app_otas_get_storage_address();
                if(app_otas_crc_cal(cmd_hdr->cmd.fir_crc_data.firmware_length,new_bin_base,cmd_hdr->cmd.fir_crc_data.CRC32_data)){
#endif      
                app_otas_erase(B_ZONE_INFO_OFFSET);
                app_otas_save_data(B_ZONE_INFO_OFFSET,(uint8_t *)&image_info_tmp,sizeof(struct image_info_t));
								motor_init(100,2,15,10,0);
									
								co_printf("crc32 check success\r\n\r\n");
	
/*
                co_printf("image_storage_offset:0x%x,image_length:0x%x\r\n\r\n",image_info_tmp.image_storage_offset,image_info_tmp.image_length);
                uint32_t crc = crc32(0xffffffff, (const uint8_t *)(QSPI0_DAC_ADDRESS + B_ZONE_INFO_OFFSET),
                        (uint32_t)&(((struct jump_table_t *)0)->checkword));
                co_printf("check1,crc:0x%x,jump_table_crc:0x%x\r\n\r\n",crc,image_info_tmp.crc.jump_table_crc);
                crc = crc32(0xffffffff, (const uint8_t *)(QSPI0_DAC_ADDRESS + image_offset), image_length);
                co_printf("check2,crc:0x%x,image_crc:0x%x\r\n\r\n",crc,image_info_tmp.crc.image_crc);
                show_reg((uint8_t *)(QSPI0_DAC_ADDRESS + B_ZONE_INFO_OFFSET),80,1);
*/
#ifdef OTA_CRC_CHECK    
                }
                else{
                    co_printf("crc32 check fail\r\n\r\n");
                    os_free(req);
                    ota_stop(OTA_CHECK_FAIL);
                    platform_reset(0);
                }
#endif  
            }
            app_set_ota_state(0);
            //uart_finish_transfers(UART1_BASE);
						    os_timer_start(&os_timer_ota_reset, 1000, false);//delay 1s then goto reset,wait for motors vibration timeout
            //platform_reset(0);
            break;
        default:
            rsp_hdr->result = OTA_RSP_UNKNOWN_CMD;
            break;
    }

    ota_gatt_report_notify(conidx,req->buffer,req->length);
    ota_recover_flash_pin();
    os_free(req);
}



uint16_t app_otas_read_data(uint8_t conidx,uint8_t *p_data)
{
    uint16_t length;
    switch(app_otas_status.read_opcode)
    {
        case OTA_CMD_READ_DATA:
            app_boot_load_data(p_data,app_otas_status.base_addr,app_otas_status.length);
            length = app_otas_status.length;
            break;
        case OTA_CMD_READ_MEM:
            memcpy(p_data, (uint8_t *)app_otas_status.base_addr, app_otas_status.length);
            length = app_otas_status.length;
            break;
        default:
            length = 0;
            break;
    }
    app_otas_status.read_opcode = OTA_CMD_NULL;
    return length;
}

#ifdef OTA_CRC_CHECK
const int crc_table[256] = { 0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
				0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832,
				0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd,
				0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148,
				0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
				0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f,
				0x63066cd9, 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e,
				0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd,
				0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
				0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac,
				0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
				0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2,
				0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
				0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589,
				0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934,
				0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97,
				0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
				0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6,
				0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49,
				0x8cd37cf3, 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074,
				0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
				0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73,
				0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
				0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409,
				0xce61e49f, 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
				0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320,
				0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af,
				0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e,
				0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
				0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d,
				0x806567cb, 0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0,
				0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43,
				0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
				0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda,
				0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
				0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0,
				0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
				0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7,
				0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226,
				0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785,
				0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
				0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4,
				0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b,
				0x6fb077e1, 0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca,
				0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
				0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661,
				0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
				0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f,
				0x30b5ffe9, 0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
				0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e,
				0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1,
				0x5a05df1b, 0x2d02ef8d, };

__attribute__((section("ram_code")))uint32_t Crc32CalByByte(int crc,uint8_t* ptr, int len)
{
    int i = 0;
    while(len-- != 0)
    {
        int high = crc/256;
        crc <<= 8;
        crc ^= crc_table[(high^ptr[i])&0xff];
        crc &= 0xFFFFFFFF;
        i++;
    }
    return crc&0xFFFFFFFF;
}

__attribute__((section("ram_code")))uint8_t app_otas_crc_cal(uint32_t firmware_length,uint32_t new_bin_addr,uint32_t crc_data_t)
{   
    uint32_t crc_data = 0,i = 0;
    uint16_t crc_packet_num = (firmware_length-256)/256;
    uint32_t crc32_check_addr = new_bin_addr+256;
    uint8_t * crc_check_data = os_malloc(256);
    uint8_t ret = 0;
    co_printf("crc32_check_addr: 0x%x\r\n",crc32_check_addr);
    
//    uint32_t current_remap_address, remap_size;
    for( i = 0;i < crc_packet_num;i++){
//        current_remap_address = system_regs->remap_virtual_addr;
//        remap_size = system_regs->remap_length;

        GLOBAL_INT_DISABLE();
//        system_regs->remap_virtual_addr = 0;
//        system_regs->remap_length = 0;
//        disable_cache();
        flash_read((crc32_check_addr+256*i),256,crc_check_data);
//        enable_cache(true);
//        system_regs->remap_virtual_addr = current_remap_address;
//        system_regs->remap_length = remap_size;
        GLOBAL_INT_RESTORE();
        crc_data =  Crc32CalByByte(crc_data, crc_check_data, 256);
    }
    firmware_length -= (256*(crc_packet_num+1));
    if(firmware_length > 0){
//        uint32_t current_remap_address, remap_size;
//        current_remap_address = system_regs->remap_virtual_addr;
//        remap_size = system_regs->remap_length;

        GLOBAL_INT_DISABLE();
//        system_regs->remap_virtual_addr = 0;
//        system_regs->remap_length = 0;
//        disable_cache();
        flash_read((crc32_check_addr+256*i),firmware_length,crc_check_data);
//        enable_cache(true);
//        system_regs->remap_virtual_addr = current_remap_address;
//        system_regs->remap_length = remap_size;
        GLOBAL_INT_RESTORE();
        crc_data =  Crc32CalByByte(crc_data, crc_check_data, firmware_length);
    }
    // crc_data =  crc32(crc_data, (const unsigned char *)new_bin_base, cmd_hdr->cmd.fir_crc_data.firmware_length);
    co_printf("crc_data= %x\r\n",crc_data);	

    os_free(crc_check_data);
    if(crc_data_t == crc_data)
        ret = 1;

    return ret;
}

#endif


