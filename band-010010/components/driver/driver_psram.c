#include <stdint.h>
#include <string.h>

#include "plf.h"
#include "driver_qspi.h"
#include "driver_psram.h"
#include "driver_system.h"
#include "driver_pmu.h"

#include "co_printf.h"

#ifdef INLINE
#undef INLINE
#endif
#define INLINE

#define PSRAM_ENABLE_Q_MODE             1

#define PSRAM_READ_IDENTIFICATION       0x9F

#define PSRAM_READ_OPCODE               0x03
#define PSRAM_FAST_READ_OPCODE          0x0B
#define PSRAM_FAST_QUAL_READ_OPCODE     0xEB

#define PSRAM_PAGE_PROGRAM_OPCODE       0x02
#define PSRAM_PAGE_QUAL_PROGRAM_OPCODE  0x38

#define PSRAM_ENTER_QUAD_MODE_OPCODE    0x35
#define PSRAM_EXIT_QUAD_MODE_OPCODE     0xF5

#define PSRAM_RESET_ENABLE_OPCODE       0x66
#define PSRAM_RESET_OPCODE              0x99

#define QSPI0_STIG_MAX_SINGLE_LEN       8
#define QSPI0_STIG_BANK_DEPTH           128

#define QSPI0_HIGH_SPEED                1

static const struct qspi_stig_reg_t psram_read_id_cmd = {
    .enable_bank = 0,
    .dummy_cycles = 0,
    .write_bytes = 0,
    .enable_write = 0,
    .addr_bytes = 0,
    .enable_mode = 0,
    .enable_cmd_addr = 0,
    .read_bytes = 3,
    .enable_read = 1,
    .opcode = PSRAM_READ_IDENTIFICATION,
};

static const struct qspi_stig_reg_t enter_quad_mode_cmd = {
    .enable_bank = 0,
    .dummy_cycles = 0,
    .write_bytes = 0,
    .enable_write = 0,
    .addr_bytes = 0,
    .enable_mode = 0,
    .enable_cmd_addr = 0,
    .read_bytes = 0,
    .enable_read = 0,
    .opcode = PSRAM_ENTER_QUAD_MODE_OPCODE,
};

static const struct qspi_stig_reg_t reset_enable_cmd = {
    .enable_bank = 0,
    .dummy_cycles = 0,
    .write_bytes = 0,
    .enable_write = 0,
    .addr_bytes = 0,
    .enable_mode = 0,
    .enable_cmd_addr = 0,
    .read_bytes = 0,
    .enable_read = 0,
    .opcode = PSRAM_RESET_ENABLE_OPCODE,
};

static const struct qspi_stig_reg_t reset_cmd = {
    .enable_bank = 0,
    .dummy_cycles = 0,
    .write_bytes = 0,
    .enable_write = 0,
    .addr_bytes = 0,
    .enable_mode = 0,
    .enable_cmd_addr = 0,
    .read_bytes = 0,
    .enable_read = 0,
    .opcode = PSRAM_RESET_OPCODE,
};

//static const struct qspi_stig_reg_t exit_quad_mode_cmd = {
//    .enable_bank = 0,
//    .dummy_cycles = 0,
//    .write_bytes = 0,
//    .enable_write = 0,
//    .addr_bytes = 0,
//    .enable_mode = 0,
//    .enable_cmd_addr = 0,
//    .read_bytes = 0,
//    .enable_read = 0,
//    .opcode = PSRAM_EXIT_QUAD_MODE_OPCODE,
//};

volatile struct qspi_regs_t *qspi0_ctrl = (volatile struct qspi_regs_t *)QSPI0_APB_BASE;

INLINE void qspi0_cfg_set_enable(uint8_t en)
{
    qspi0_ctrl->config.enable = en;
}

INLINE void qspi0_cfg_set_cpol(uint8_t high)
{
    qspi0_ctrl->config.cpol = high;
}

INLINE void qspi0_cfg_set_cpha(uint8_t rising)
{
    qspi0_ctrl->config.cpha = rising;
}

INLINE void qspi0_cfg_set_hold(uint8_t hold)
{
    qspi0_ctrl->config.hold_pin = hold;
}

INLINE void qspi0_cfg_set_reset(uint8_t reset)
{
    qspi0_ctrl->config.reset_pin = reset;
}

INLINE void qspi0_cfg_set_reset_sel(uint8_t dedicated)
{
    qspi0_ctrl->config.reset_pin_conf = dedicated;
}

INLINE void qspi0_cfg_set_enable_dac(uint8_t en)
{
    qspi0_ctrl->config.enable_DAC = en;
}

INLINE void qspi0_cfg_set_enable_legacy(uint8_t en)
{
    qspi0_ctrl->config.enable_legacy = en;
}

INLINE void qspi0_cfg_set_write_protect(uint8_t wp)
{
    qspi0_ctrl->config.write_en_pin = wp;
}

INLINE void qspi0_cfg_set_enable_remap(uint8_t en)
{
    qspi0_ctrl->config.enable_AHB_remap = en;
}

static void qspi0_cfg_set_baudrate(uint8_t baudrate);

INLINE void qspi0_cfg_set_enable_AHB_decoder(uint8_t en)
{
    qspi0_ctrl->config.enable_AHB_decoder = en;
}

INLINE int qspi0_is_busy(void)
{
    return (qspi0_ctrl->config.status == 0);
}

INLINE void qspi0_read_set_opcode(uint8_t opcode)
{
    qspi0_ctrl->read_conf.opcode_no_XIP = opcode;
}

INLINE void qspi0_read_set_instruction_type(uint8_t type)
{
    qspi0_ctrl->read_conf.instruction_type = type;
}

INLINE void qspi0_read_set_address_type(uint8_t type)
{
    qspi0_ctrl->read_conf.addr_type = type;
}

INLINE void qspi0_read_set_data_type(uint8_t type)
{
    qspi0_ctrl->read_conf.data_type = type;
}

INLINE void qspi0_read_set_mode_en(uint8_t en)
{
    qspi0_ctrl->read_conf.enable_mode = en;
}

INLINE void qspi0_read_set_dummy_cycles(uint8_t cycles)
{
    qspi0_ctrl->read_conf.dummy_cycles = cycles;
}

INLINE void qspi0_write_set_opcode(uint8_t opcode)
{
    qspi0_ctrl->write_conf.opcode = opcode;
}

INLINE void qspi0_write_set_wel_dis(uint8_t disable)
{
    qspi0_ctrl->write_conf.disable_WEL = disable;
}

INLINE void qspi0_write_set_address_type(uint8_t type)
{
    qspi0_ctrl->write_conf.addr_type = type;
}

INLINE void qspi0_write_set_data_type(uint8_t type)
{
    qspi0_ctrl->write_conf.data_type = type;
}

INLINE void qspi0_write_set_dummy_cycles(uint8_t cycles)
{
    qspi0_ctrl->write_conf.dummy_cycles = cycles;
}

INLINE void qspi0_set_remap_address(uint32_t address)
{
    qspi0_ctrl->remap_address = address;
}

INLINE void qspi0_set_mode_bit(uint8_t mode)
{
    qspi0_ctrl->mode_bits = (uint32_t)mode;
}

INLINE void qspi0_poll_set_opcode(uint8_t opcode)
{
    qspi0_ctrl->poll_cfg.opcode_poll = opcode;
}

INLINE void qspi0_poll_set_bit_index(uint8_t index)
{
    qspi0_ctrl->poll_cfg.poll_bit_index = index;
}

INLINE void qspi0_poll_set_polarity(uint8_t pol)
{
    qspi0_ctrl->poll_cfg.poll_polarity = pol;
}

INLINE void qspi0_poll_set_disable(uint8_t dis)
{
    qspi0_ctrl->poll_cfg.disable_poll = dis;
}

INLINE void qspi0_poll_set_expire(uint8_t en, uint32_t duration)
{
    qspi0_ctrl->poll_cfg.enable_expiration = en;
    if(en) {
        qspi0_ctrl->poll_expiration = duration;
    }
}

INLINE void qspi0_poll_set_poll_count(uint8_t count)
{
    qspi0_ctrl->poll_cfg.poll_count = count;
}

INLINE void qspi0_poll_set_poll_delay(uint8_t delay)
{
    qspi0_ctrl->poll_cfg.poll_repetition_delay = delay;
}

INLINE void qspi0_set_cmd_addr(uint32_t addr)
{
    qspi0_ctrl->cmd_address = addr;
}

INLINE void qspi0_stig_set_opcode(uint8_t opcode)
{
    qspi0_ctrl->cmd_ctrl.opcode = opcode;
}

INLINE void qspi0_stig_set_read_en(uint8_t en)
{
    qspi0_ctrl->cmd_ctrl.enable_read = en;
}

INLINE void qspi0_stig_set_read_bytes(uint8_t bytes)
{
    qspi0_ctrl->cmd_ctrl.read_bytes = bytes;
}

INLINE void qspi0_stig_set_addr(uint8_t en, uint8_t bytes, uint32_t addr)
{
    qspi0_ctrl->cmd_ctrl.enable_cmd_addr = en;
    qspi0_ctrl->cmd_ctrl.addr_bytes = bytes;
    if(en) {
        qspi0_ctrl->cmd_address = addr;
    }
}

INLINE void qspi0_stig_set_mode_en(uint8_t en)
{
    qspi0_ctrl->cmd_ctrl.enable_mode = en;
}

INLINE void qspi0_stig_set_write_en(uint8_t en)
{
    qspi0_ctrl->cmd_ctrl.enable_write = en;
}

INLINE void qspi0_stig_set_write_bytes(uint8_t bytes)
{
    qspi0_ctrl->cmd_ctrl.write_bytes = bytes;
}

INLINE void qspi0_stig_set_dummy_cycles(uint8_t cycles)
{
    qspi0_ctrl->cmd_ctrl.dummy_cycles = cycles;
}

INLINE void qspi0_stig_set_mem_bank(uint8_t en, uint8_t bytes)
{
    qspi0_ctrl->cmd_ctrl.enable_bank = en;
    if(en) {
        qspi0_ctrl->cmd_ctrl_mem.mem_bank_req_bytes = bytes;
    }
}

static void qspi0_cfg_set_baudrate(uint8_t baudrate)
{
    qspi0_ctrl->config.baud_rate = baudrate;
}

static int qspi0_stig_cmd(struct qspi_stig_reg_t cmd, enum qspi_stig_cmd_type_t type, int len, uint8_t *buffer)
{
    uint32_t tmp_u32[2];
    uint8_t *tmp_u8 = (uint8_t *)tmp_u32;

    if(type == QSPI_STIG_CMD_BANK_READ) {
        if(QSPI0_STIG_BANK_DEPTH < len) {
            return -1;
        }
    }
    else {
        if(QSPI0_STIG_MAX_SINGLE_LEN < len) {
            return -1;
        }
    }

    while(qspi0_is_busy());

    if(type == QSPI_STIG_CMD_EXE) {
        qspi0_ctrl->cmd_ctrl = cmd;
        qspi0_ctrl->cmd_ctrl.execute = 1;
        while(qspi0_ctrl->cmd_ctrl.progress_status);
    }
    else {
        if(type == QSPI_STIG_CMD_WRITE) {
            memcpy(tmp_u8, buffer, len);
            qspi0_ctrl->write_data_L = tmp_u32[0];
            qspi0_ctrl->write_data_H = tmp_u32[1];
            cmd.write_bytes = len - 1;
            qspi0_ctrl->cmd_ctrl = cmd;
            qspi0_ctrl->cmd_ctrl.execute = 1;
            while(qspi0_ctrl->cmd_ctrl.progress_status);
        }
        else {
            cmd.read_bytes = len - 1;
            qspi0_ctrl->cmd_ctrl = cmd;
            qspi0_ctrl->cmd_ctrl.execute = 1;
            while(qspi0_ctrl->cmd_ctrl.progress_status);
            if(type == QSPI_STIG_CMD_READ) {
                tmp_u32[0] = qspi0_ctrl->read_data_L;
                tmp_u32[1] = qspi0_ctrl->read_data_H;
                //co_printf("READ_L: 0x%08x, READ_H: 0x%08x.\r\n", tmp_u32[0], tmp_u32[1]);
                memcpy(buffer, tmp_u8, len);
            }
            else {
                //TBD, BANK READ
            }
        }
    }

    return 0;
}


void psram_enter_quad(void)
{
    qspi0_stig_cmd(enter_quad_mode_cmd, QSPI_STIG_CMD_EXE, 0, NULL);
}

uint32_t psram_read_id(void)
{
    uint32_t flash_id;
    qspi0_stig_cmd(psram_read_id_cmd, QSPI_STIG_CMD_READ,  3, (uint8_t *)&flash_id);
    return (flash_id&0xffffff);
}

static void psram_controller_init(uint16_t page_boundary)
{
    while(qspi0_is_busy());
    
#if PSRAM_ENABLE_Q_MODE == 1
    qspi0_read_set_opcode(PSRAM_FAST_READ_OPCODE);
    qspi0_read_set_instruction_type(QSPI_WIRE_TYPE_QIO);
    qspi0_read_set_address_type(QSPI_WIRE_TYPE_QIO);
    qspi0_read_set_data_type(QSPI_WIRE_TYPE_QIO);
    qspi0_read_set_dummy_cycles(4);
    qspi0_read_set_mode_en(0);
    qspi0_set_mode_bit(0);//8 bits data after addr

    qspi0_write_set_opcode(PSRAM_PAGE_QUAL_PROGRAM_OPCODE);
    qspi0_write_set_address_type(QSPI_WIRE_TYPE_QIO);
    qspi0_write_set_data_type(QSPI_WIRE_TYPE_QIO);
    qspi0_write_set_dummy_cycles(0);
#else
    qspi0_read_set_opcode(PSRAM_FAST_QUAL_READ_OPCODE);	
    qspi0_read_set_instruction_type(QSPI_WIRE_TYPE_STAND);
    qspi0_read_set_address_type(QSPI_WIRE_TYPE_QIO);
    qspi0_read_set_data_type(QSPI_WIRE_TYPE_QIO);
    qspi0_read_set_dummy_cycles(6);
    qspi0_read_set_mode_en(0);
    qspi0_set_mode_bit(0);//8 bits data after addr

    qspi0_write_set_opcode(PSRAM_PAGE_QUAL_PROGRAM_OPCODE);
    qspi0_write_set_address_type(QSPI_WIRE_TYPE_QIO);
    qspi0_write_set_data_type(QSPI_WIRE_TYPE_QIO);
    qspi0_write_set_dummy_cycles(0);
#endif

    //init configuration register
    qspi0_cfg_set_cpol(0);
    qspi0_cfg_set_cpha(0);
    qspi0_cfg_set_enable_dac(1);
    qspi0_cfg_set_enable_legacy(0);
    qspi0_cfg_set_enable_remap(1);
#if QSPI0_HIGH_SPEED
    qspi0_cfg_set_baudrate(QSPI_BAUDRATE_DIV_2);
#else
    qspi0_cfg_set_baudrate(QSPI_BAUDRATE_DIV_32);
#endif
    qspi0_cfg_set_enable_AHB_decoder(1);
    qspi0_write_set_wel_dis(1);
    qspi0_poll_set_disable(1);

#if QSPI0_HIGH_SPEED
    qspi0_ctrl->read_cap.delay_capture = 3;
#else
    qspi0_ctrl->read_cap.delay_capture = 0;
#endif
    qspi0_ctrl->read_cap.enable_loopback_clk = 1;
    qspi0_ctrl->delay.sel_start_offset = 2;
    qspi0_ctrl->delay.sel_end_offset = 2;
    qspi0_ctrl->delay.sel_dessert = 2;
    qspi0_ctrl->cs_ctrl.rd_brk_en = 1;
    qspi0_ctrl->cs_ctrl.disable_cs_after_first_byte = 1;
    qspi0_ctrl->cs_ctrl.page_boundary_protect_en = 1;
    qspi0_ctrl->cs_ctrl.page_boundary = page_boundary;
    qspi0_set_remap_address(QSPI1_DAC_ADDRESS);
    qspi0_cfg_set_enable(1);
}

void psram_init(void)
{
    system_regs->mdm_qspi_cfg.qspi0_ref_clk_en=1;
    system_regs->mdm_qspi_cfg.qspi0_hclk_en=1;
    system_regs->mdm_qspi_cfg.qspi0_io_ctl_oen=0;
    system_regs->mdm_qspi_cfg.qspi0_ref_clk_sel = 1;  // set qspi0 reference clock to 96MHz

    qspi0_stig_cmd(reset_enable_cmd, QSPI_STIG_CMD_EXE, 0, NULL);
    qspi0_stig_cmd(reset_cmd, QSPI_STIG_CMD_EXE, 0, NULL);
    co_delay_10us(5);
#if PSRAM_ENABLE_Q_MODE == 1
    psram_enter_quad();
#endif  // PSRAM_ENABLE_Q_MODE == 1

#if QSPI0_HIGH_SPEED
    psram_controller_init(0x0040);
#else
    psram_controller_init(0x0008);
#endif
}

