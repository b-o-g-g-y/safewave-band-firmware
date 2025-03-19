/*
  ******************************************************************************
  * @file    driver_flash.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   flash module driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "driver_flash.h"
#include "driver_qspi.h"
#include "sys_utils.h"

extern const struct qspi_stig_reg_t read_status_cmd;
extern const struct qspi_stig_reg_t write_enable_cmd;
extern const struct qspi_stig_reg_t write_disable_cmd;

//For securuty_sec_erase
const struct qspi_stig_reg_t sec_sector_erase_cmd = {
    .enable_bank = 0,
    .dummy_cycles = 0,
    .write_bytes = 0,
    .enable_write = 0,
    .addr_bytes = QSPI_STIG_ADDR_BYTES_3,
    .enable_mode = 0,
    .enable_cmd_addr = 1,
    .read_bytes = 0,
    .enable_read = 0,
    .opcode = FLASH_SEC_REG_ERASE_OPCODE,
};
//For securuty_sec_read
const struct qspi_stig_reg_t sec_sector_read_cmd = {
    .enable_bank = 0,
    .dummy_cycles = 1,
    .write_bytes = 0,
    .enable_write = 0,
    .addr_bytes = QSPI_STIG_ADDR_BYTES_3,
    .enable_mode = 0,
    .enable_cmd_addr = 1,
    .read_bytes = 0,
    .enable_read = 1,
    .opcode = FLASH_SEC_REG_READ_OPCODE,
};
//For securuty_sec_write
const struct qspi_stig_reg_t sec_sector_write_cmd = {
    .enable_bank = 0,
    .dummy_cycles = 0,
    .write_bytes = 0,
    .enable_write = 1,
    .addr_bytes = QSPI_STIG_ADDR_BYTES_3,
    .enable_mode = 0,
    .enable_cmd_addr = 1,
    .read_bytes = 0,
    .enable_read = 0,
    .opcode = FLASH_SEC_REG_PROGRAM_OPCODE,
};

/*********************************************************************
* @fn      flash_OTP_read
*
* @brief   read data from OTP sections. otp section size: 512 = 0x200 
*
* @param   offset - Security flash section offset, only valid for 0x1000,0x2000,0x3000 
*          length - buffer length to be read from security flash section. rang[1,512]
*          buffer - pointer to buffer which will store data
*
* @return  None.
*/
__attribute__((section("ram_code"))) void flash_OTP_read(uint32_t offset, uint32_t length, uint8_t *buffer)
{
    uint32_t i = 0;

    offset &= 0x3000;
    if(length > 512) 
        length = 512;

    GLOBAL_INT_DISABLE();
    for(; (i+8) <= length; )
    {
        *(volatile uint32_t *)0x500B8094 = offset;
        qspi_stig_cmd(sec_sector_read_cmd, QSPI_STIG_CMD_READ, 8, buffer + i );
        i+=8;
        offset+=8;
    }
    if(i < length)          
    {
        *(volatile uint32_t *)0x500B8094 = offset;
        qspi_stig_cmd(sec_sector_read_cmd, QSPI_STIG_CMD_READ, (length-i), (buffer+i));
    }
    GLOBAL_INT_RESTORE();
}

/*********************************************************************
* @fn      flash_OTP_write
*
* @brief   write data to OTP sections. otp section size: 512 = 0x200 
*
* @param   offset - Security flash section offset, only valid for 0x1000,0x2000,0x3000 
*          length - buffer length to be written into security flash section. rang[1,512]
*          buffer - pointer to buffer which will be written
*
* @return  None.
*/
__attribute__((section("ram_code"))) void flash_OTP_write(uint32_t offset, uint32_t length, uint8_t *buffer)
{
    uint8_t status[1];
    uint32_t i = 0;

#define SINGLE_LENGTH       8
    offset &= 0x3000;
    if(length > 512) 
        length = 512;

    GLOBAL_INT_DISABLE();

    for(; (i+SINGLE_LENGTH) <= length; )
    {
        qspi_stig_cmd(write_enable_cmd, QSPI_STIG_CMD_EXE, 0, 0);
        *(volatile uint32_t *)0x500B8094 = offset;
        qspi_stig_cmd(sec_sector_write_cmd, QSPI_STIG_CMD_WRITE, SINGLE_LENGTH, buffer+i);
        while(1) {
            co_delay_10us(5);
            qspi_stig_cmd(read_status_cmd, QSPI_STIG_CMD_READ, 1, &status[0]);
            if((status[0] & 0x01) == 0) {
                break;
            }
        }
        qspi_stig_cmd(write_disable_cmd, QSPI_STIG_CMD_EXE, 0, 0);
        i+=SINGLE_LENGTH;
        offset+=SINGLE_LENGTH;
    }
    if(i < length)          
    {
        qspi_stig_cmd(write_enable_cmd, QSPI_STIG_CMD_EXE, 0, 0);
        *(volatile uint32_t *)0x500B8094 = offset;
        qspi_stig_cmd(sec_sector_write_cmd, QSPI_STIG_CMD_WRITE, (length-i), (buffer+i));
        while(1) {
            co_delay_10us(5);
            qspi_stig_cmd(read_status_cmd, QSPI_STIG_CMD_READ, 1, &status[0]);
            if((status[0] & 0x01) == 0) {
                break;
            }
        }
        qspi_stig_cmd(write_disable_cmd, QSPI_STIG_CMD_EXE, 0, 0);
    }
    
    GLOBAL_INT_RESTORE();
}

/*********************************************************************
* @fn      flash_OTP_erase
*
* @brief   erase whole OTP section . 
*
* @param   offset - Security flash section offset, only valid for 0x1000,0x2000,0x3000 
*
* @return  None.
*/
__attribute__((section("ram_code"))) void flash_OTP_erase(uint32_t offset)
{
    uint8_t status;
    offset &= 0x3000;

    GLOBAL_INT_DISABLE();
    qspi_stig_cmd(write_enable_cmd, QSPI_STIG_CMD_EXE, 0, 0);
    *(volatile uint32_t *)0x500B8094 = offset;
    qspi_stig_cmd(sec_sector_erase_cmd, QSPI_STIG_CMD_EXE, 0, 0);
    
    while(1) {
        co_delay_10us(5);
        qspi_stig_cmd(read_status_cmd, QSPI_STIG_CMD_READ, 1, &status);
        if((status & 0x01) == 0) {
            break;
        }
    }
    qspi_stig_cmd(write_disable_cmd, QSPI_STIG_CMD_EXE, 0, 0);
    GLOBAL_INT_RESTORE();
}
