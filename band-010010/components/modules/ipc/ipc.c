/*
  ******************************************************************************
  * @file    ipc.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   freqchip Multimachine communication.
  *          This file provides firmware functions to manage the 
  *          core communication.
  *          This module is used for FR8008APD.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include <stdbool.h>

#include "ipc.h"

#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_uart_ex.h"
#include "driver_pmu.h"

#include "plf.h"
#include "os_mem.h"
#include "co_log.h"
#include "co_list.h"

#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL             (LOG_LEVEL_NONE)

#define PC_TO_MASTER_UART           UART0
#define MASTER_TO_SLAVE_UART        UART1

#define UP_OPCODE_READ              0x08
#define UP_OPCODE_READ_RAM          0x0a
#define UP_OPCODE_DISCONNECT        0x10
#define UP_OPCODE_CHANGE_BANDRATE   0x12
#define UP_OPCODE_CALC_MD5          0x1f
#define UP_OPCODE_CALC_CRC32        0x21

/* communication start character, stop character */
#define CHAR_START_0    ('/')    /* 0x2F */
#define CHAR_START_1    ('#')    /* 0x23 */
#define CHAR_START_2    ('D')    /* 0x44, Data package */
#define CHAR_START_3    ('S')    /* 0x53, Status package */

/* communication status index */
#define TYPE_STATUS    ('S')
#define TYPE_STATUS_READY    ('R')    /* Receive ready */
#define TYPE_STATUS_OK       ('O')    /* OK */
#define TYPE_STATUS_FAIL     ('F')    /* FAIL */

enum ipc_msg_type_t {
    IPC_MSG_TYPE_TX,
    IPC_MSG_TYPE_RX,
    IPC_MSG_TYPE_READY,
};

enum ipc_msg_recv_state_t {
    IPC_MSG_RECV_STATE_IDLE,
    IPC_MSG_RECV_STATE_HEADER_1,
    IPC_MSG_RECV_STATE_HEADER_2,
    IPC_MSG_RECV_STATE_TYPE,
    IPC_MSG_RECV_STATE_MSG_LENGTH,
    IPC_MSG_RECV_STATE_READY,
    IPC_MSG_RECV_STATE_PAYLOAD,
};

struct ipc_msg_t {
    struct co_list_hdr hdr;

    void (*callback)(void *);
    void *arg;
    enum ipc_msg_type_t type;
    uint8_t preamble_sent;
    uint16_t length;
    uint8_t data[];
};

/* The following variables are used for program slave */
static const uint8_t pc_tool_conn_req[] = {'F','R','E','Q','C','H','I','P'};//from embedded to pc, request
static const uint8_t app_boot_conn_req[] = {'f','r','e','q','c','h','i','p'};//from embedded to pc, request
static const uint8_t app_boot_conn_ack[] = {'F','R','8','0','0','8','O','K'};//from pc to embedded,ack
static const uint8_t slave_program_conn_ack[] = {'F','R','8','0','0','0','O','K'};//from pc to embedded,ack
static const uint8_t app_boot_conn_success[] = {'o','k'};

/* The following variables are used for IPC */
static struct co_list ipc_msg_send_list;
static void (*ipc_msg_recv_callback)(uint8_t *, uint16_t) = NULL;
static struct ipc_msg_t *ipc_recving_msg = NULL;
static UART_HandleTypeDef uart_handle;
static bool ipc_send_ongoing = false;
static bool ipc_reqeust_cleared = true;
static enum ipc_side_t ipc_local_side;
static enum ipc_msg_recv_state_t ipc_recv_state = IPC_MSG_RECV_STATE_IDLE;
static uint8_t ipc_recv_buffer[5];

static uint8_t ipc_data_preamble[5] = {CHAR_START_0, CHAR_START_1, CHAR_START_2};
static const uint8_t ipc_ready_preamble[] = {CHAR_START_0, CHAR_START_1, CHAR_START_3, TYPE_STATUS, TYPE_STATUS_READY};
static const char *ipc_log_tag = "ipc: ";

static volatile void uart_rx_callback(struct __UART_HandleTypeDef *huart);
static volatile void uart_tx_callback(struct __UART_HandleTypeDef *huart);
static void send_next_frame(void);

static void init_uart(void)
{
    GPIO_InitTypeDef gpio_handle;
    
    // Alternate Function

    if(ipc_local_side == IPC_SIDE_MASTER) {
        gpio_handle.Pin       = GPIO_PIN_2|GPIO_PIN_3;
        gpio_handle.Alternate = GPIO_FUNCTION_5;
    }
    else if(ipc_local_side == IPC_SIDE_SLAVE) {
        gpio_handle.Pin       = GPIO_PIN_0|GPIO_PIN_1;
        gpio_handle.Alternate = GPIO_FUNCTION_4;
    }
    else {
        return;
    }
    gpio_handle.Mode      = GPIO_MODE_AF_PP;
    gpio_handle.Pull      = GPIO_PULLUP;
    gpio_init(GPIO_A, &gpio_handle);

    // Uart init
    if(ipc_local_side == IPC_SIDE_MASTER) {
        __SYSTEM_UART1_CLK_ENABLE();
        uart_handle.UARTx = Uart1;
    }
    else if(ipc_local_side == IPC_SIDE_SLAVE) {
        __SYSTEM_UART0_CLK_ENABLE();
        uart_handle.UARTx = Uart0;
    }
    else {
        return;
    }
    uart_handle.Init.BaudRate   = 115200;
    uart_handle.Init.DataLength = UART_DATA_LENGTH_8BIT;
    uart_handle.Init.StopBits   = UART_STOPBITS_1;
    uart_handle.Init.Parity     = UART_PARITY_NONE;
    uart_handle.Init.FIFO_Mode  = UART_FIFO_ENABLE;

    uart_handle.RxCpltCallback = uart_rx_callback;
    uart_handle.TxCpltCallback = uart_tx_callback;

    uart_init_ex(&uart_handle);

    __UART_RxFIFO_THRESHOLD(((UART_HandleTypeDef *)(&uart_handle)), 2);
    __UART_TxFIFO_THRESHOLD(((UART_HandleTypeDef *)(&uart_handle)), 3);

    __UART_INT_RX_ENABLE(uart_handle.UARTx);

    if(ipc_local_side == IPC_SIDE_MASTER) {
        NVIC_EnableIRQ(UART1_IRQn);
    }
    else if(ipc_local_side == IPC_SIDE_SLAVE) {
        NVIC_EnableIRQ(UART0_IRQn);
    }

    ipc_recv_state = IPC_MSG_RECV_STATE_HEADER_1;
    uart_receive_IT(&uart_handle, &ipc_recv_buffer[0], 1);
}

static void init_IO(void)
{
    // Alternate Function

    if(ipc_local_side == IPC_SIDE_MASTER) {
        // request receive pin (USE PMU control)
        pmu_port_wakeup_func_set(GPIO_PORT_A, GPIO_PIN_6);

        // request pin
        pmu_set_pin_dir(GPIO_PORT_A, GPIO_PIN_5, GPIO_DIR_OUT);
        ool_write(PMU_REG_PORTA_V, ool_read(PMU_REG_PORTA_LAST_STATUS) | (GPIO_PIN_5));
        pmu_set_pin_to_PMU(GPIO_PORT_A, GPIO_PIN_5);

        pmu_set_pin_dir(GPIO_PORT_A, GPIO_PIN_2 | GPIO_PIN_3, GPIO_DIR_IN);
        pmu_set_pin_pull(GPIO_PORT_A, GPIO_PIN_2 | GPIO_PIN_3, GPIO_PULL_UP);
    }
    else if(ipc_local_side == IPC_SIDE_SLAVE) {
        // request receive pin (USE PMU control)
        pmu_port_wakeup_func_set(GPIO_PORT_A, GPIO_PIN_6);

        // request pin
        pmu_set_pin_dir(GPIO_PORT_A, GPIO_PIN_7, GPIO_DIR_OUT);
        ool_write(PMU_REG_PORTA_V, ool_read(PMU_REG_PORTA_LAST_STATUS) | (GPIO_PIN_7));
        pmu_set_pin_to_PMU(GPIO_PORT_A, GPIO_PIN_7);

        pmu_set_pin_dir(GPIO_PORT_A, GPIO_PIN_0 | GPIO_PIN_1, GPIO_DIR_IN);
        pmu_set_pin_pull(GPIO_PORT_A, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PULL_UP);
    }
    else {
        return;
    }
}

static void init_hw(void)
{
    init_IO();
    init_uart();
}

static void uart_set_tx_req(void)
{
    if(ipc_local_side == IPC_SIDE_MASTER) {
        ool_write(PMU_REG_PORTA_V, ool_read(PMU_REG_PORTA_LAST_STATUS) & (~GPIO_PIN_5));
    }
    else if(ipc_local_side == IPC_SIDE_SLAVE) {
        ool_write(PMU_REG_PORTA_V, ool_read(PMU_REG_PORTA_LAST_STATUS) & (~GPIO_PIN_7));
    }
}

static void uart_clear_tx_req(void)
{
    if(ipc_local_side == IPC_SIDE_MASTER) {
        ool_write(PMU_REG_PORTA_V, ool_read(PMU_REG_PORTA_LAST_STATUS) | (GPIO_PIN_5));
    }
    else if(ipc_local_side == IPC_SIDE_SLAVE) {
        ool_write(PMU_REG_PORTA_V, ool_read(PMU_REG_PORTA_LAST_STATUS) | (GPIO_PIN_7));
    }
}

static volatile void uart_rx_callback(struct __UART_HandleTypeDef *huart)
{
    switch(ipc_recv_state) {
        case IPC_MSG_RECV_STATE_HEADER_1:
            if(ipc_recv_buffer[0] == CHAR_START_0) {
                ipc_recv_state = IPC_MSG_RECV_STATE_HEADER_2;
            }
            uart_receive_IT(&uart_handle, &ipc_recv_buffer[0], 1);
            break;
        case IPC_MSG_RECV_STATE_HEADER_2:
            if(ipc_recv_buffer[0] == CHAR_START_1) {
                ipc_recv_state = IPC_MSG_RECV_STATE_TYPE;
            }
            else {
                ipc_recv_state = IPC_MSG_RECV_STATE_HEADER_1;
            }
            uart_receive_IT(&uart_handle, &ipc_recv_buffer[0], 1);
            break;
        case IPC_MSG_RECV_STATE_TYPE:
            if(ipc_recv_buffer[0] == CHAR_START_2) {
                ipc_recv_state = IPC_MSG_RECV_STATE_MSG_LENGTH;
                uart_receive_IT(&uart_handle, &ipc_recv_buffer[0], 2);
            }
            else if(ipc_recv_buffer[0] == CHAR_START_3) {
                ipc_recv_state = IPC_MSG_RECV_STATE_READY;
                uart_receive_IT(&uart_handle, &ipc_recv_buffer[0], 2);
            }
            else {
                ipc_recv_state = IPC_MSG_RECV_STATE_HEADER_1;
                uart_receive_IT(&uart_handle, &ipc_recv_buffer[0], 1);
            }
            break;
        case IPC_MSG_RECV_STATE_READY:
            if((ipc_recv_buffer[0] == TYPE_STATUS) && (ipc_recv_buffer[1] == TYPE_STATUS_READY)) {
                LOG_INFO(ipc_log_tag, "uart_rx_callback: IPC_MSG_RECV_STATE_READY.\r\n");
                uart_clear_tx_req();
                
                struct ipc_msg_t *msg;
                msg = (void *)co_list_pick(&ipc_msg_send_list);
                ipc_data_preamble[3] = msg->length;
                ipc_data_preamble[4] = msg->length >> 8;
                ipc_send_ongoing = true;
                uart_transmit_IT(&uart_handle, ipc_data_preamble, sizeof(ipc_data_preamble));
            }
            ipc_recv_state = IPC_MSG_RECV_STATE_HEADER_1;
            uart_receive_IT(&uart_handle, &ipc_recv_buffer[0], 1);
            break;
        case IPC_MSG_RECV_STATE_MSG_LENGTH:
            {
                uint16_t length = ipc_recv_buffer[0] | (ipc_recv_buffer[1]<<8);
                ipc_recving_msg = (void *)os_malloc(sizeof(struct ipc_msg_t)+length);
                ipc_recving_msg->type = IPC_MSG_TYPE_RX;
                ipc_recving_msg->length = length;
                ipc_recv_state = IPC_MSG_RECV_STATE_PAYLOAD;
                uart_receive_IT(&uart_handle, ipc_recving_msg->data, length);
            }
            break;
        case IPC_MSG_RECV_STATE_PAYLOAD:
            if(ipc_msg_recv_callback) {
                ipc_msg_recv_callback(ipc_recving_msg->data, ipc_recving_msg->length);
            }
            os_free(ipc_recving_msg);
            ipc_recving_msg = NULL;
            
            ipc_recv_state = IPC_MSG_RECV_STATE_HEADER_1;
            uart_receive_IT(&uart_handle, &ipc_recv_buffer[0], 1);
            break;
        default:
            ipc_recv_state = IPC_MSG_RECV_STATE_HEADER_1;
            uart_receive_IT(&uart_handle, &ipc_recv_buffer[0], 1);
            break;
    }
}

static volatile void uart_tx_callback(struct __UART_HandleTypeDef *huart)
{
    struct ipc_msg_t *msg;
    
    msg = (void *)co_list_pick(&ipc_msg_send_list);
    if(msg) {
        if(msg->type == IPC_MSG_TYPE_TX) {
            LOG_INFO(ipc_log_tag, "uart_tx_callback: IPC_MSG_TYPE_TX %d.\r\n", msg->preamble_sent);
            if(msg->preamble_sent) {
                msg = (void *)co_list_pop_front(&ipc_msg_send_list);
                if(msg->callback) {
                    msg->callback(msg->arg);
                }
                os_free(msg);
                ipc_send_ongoing = false;
                send_next_frame();
            }
            else {
                msg->preamble_sent = 1;
                uart_transmit_IT(&uart_handle, msg->data, msg->length);
            }
        }
        else if(msg->type == IPC_MSG_TYPE_READY) {
            LOG_INFO(ipc_log_tag, "uart_tx_callback: IPC_MSG_TYPE_READY.\r\n");
            msg = (void *)co_list_pop_front(&ipc_msg_send_list);
            os_free(msg);
            ipc_send_ongoing = false;
            send_next_frame();
        }
        else {
            LOG_ERR(ipc_log_tag, "send_next_frame: unexpect frame.\r\n");
        }
    }
}

static void send_next_frame(void)
{
    struct ipc_msg_t *msg;

    GLOBAL_INT_DISABLE();
    if(ipc_send_ongoing == false) {
        msg = (void *)co_list_pick(&ipc_msg_send_list);
        if(msg) {
            if(msg->type == IPC_MSG_TYPE_TX) {
//                uart_transmit_IT(&uart_handle, ipc_data_preamble, sizeof(ipc_data_preamble));
                uart_set_tx_req();
            }
            else if(msg->type == IPC_MSG_TYPE_READY) {
                ipc_send_ongoing = true;
                uart_transmit_IT(&uart_handle, (uint8_t *)ipc_ready_preamble, sizeof(ipc_ready_preamble));
            }
            else {
                LOG_ERR(ipc_log_tag, "send_next_frame: unexpect frame.\r\n");
            }
        }
    }
    GLOBAL_INT_RESTORE();
}

static int program_serial_gets(uint32_t uart, uint32_t ms, uint8_t *data_buf, uint32_t buf_size)
{
    int i, n=0;

    for(i=0; i<ms; i++)
    {
        co_delay_100us(10);
        n += uart_get_data_nodelay_noint(uart, data_buf+n, buf_size-n);
        if(n == buf_size)
        {
            return n;
        }
    }

    return -1;
}

static void program_hw_init(uint16_t baudrate)
{
    GPIO_InitTypeDef gpio_type;
    
    gpio_type.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gpio_type.Pull = GPIO_PULLUP;
    gpio_type.Mode = GPIO_MODE_AF_PP;
    gpio_type.Alternate = GPIO_FUNCTION_5;
    gpio_init(GPIO_A, &gpio_type);
    
#if MASTER_TO_SLAVE_UART == UART0
    __SYSTEM_UART0_CLK_ENABLE();
#else
    __SYSTEM_UART1_CLK_ENABLE();
#endif
    uart_init(MASTER_TO_SLAVE_UART, baudrate);
}

static void program_relay(void)
{
    uint8_t opcode, opcode_ack;
    uint32_t address, address_ack;
    uint16_t length, length_ack;
    uint8_t data;
    
    while(1) {
        /* receive command from program tools */
        uart_read(PC_TO_MASTER_UART, &opcode, 1);
        uart_write(MASTER_TO_SLAVE_UART, &opcode, 1);
        uart_read(PC_TO_MASTER_UART, (void *)&address, sizeof(address));
        uart_write(MASTER_TO_SLAVE_UART, (void *)&address, sizeof(address));
        uart_read(PC_TO_MASTER_UART, (void *)&length, sizeof(length));
        uart_write(MASTER_TO_SLAVE_UART, (void *)&length, sizeof(length));
        if((opcode != UP_OPCODE_READ)
            && (opcode != UP_OPCODE_READ_RAM)) {
            while(length--) {
                uart_read(PC_TO_MASTER_UART, &data, 1);
                uart_putc_noint_no_wait(MASTER_TO_SLAVE_UART, data);
            }
        }
        
        /* receive response from slave core */
        uart_read(MASTER_TO_SLAVE_UART, &opcode_ack, 1);
        uart_write(PC_TO_MASTER_UART, &opcode_ack, 1);
        uart_read(MASTER_TO_SLAVE_UART, (void *)&address_ack, sizeof(address_ack));
        uart_write(PC_TO_MASTER_UART, (void *)&address_ack, sizeof(address_ack));
        uart_read(MASTER_TO_SLAVE_UART, (void *)&length_ack, sizeof(length_ack));
        uart_write(PC_TO_MASTER_UART, (void *)&length_ack, sizeof(length_ack));
        if((opcode == UP_OPCODE_READ)
            || (opcode == UP_OPCODE_READ_RAM)
            || (opcode == UP_OPCODE_CALC_MD5)
            || (opcode == UP_OPCODE_CALC_CRC32)) {
            while(length--) {
                uart_read(MASTER_TO_SLAVE_UART, &data, 1);
                uart_putc_noint_no_wait(PC_TO_MASTER_UART, data);
            }
        }
        
        if(opcode == UP_OPCODE_CHANGE_BANDRATE) {
            const uint16_t uart_baudrate_table[] = {12,24,48,96,144,192,384,576,1152,2304,4608,9216};
            fr_uart_setBaudrate(PC_TO_MASTER_UART, uart_baudrate_table[address & 0xFF]);
            fr_uart_setBaudrate(MASTER_TO_SLAVE_UART, uart_baudrate_table[address & 0xFF]);
        }
        
        if(opcode == UP_OPCODE_DISCONNECT) {
            fr_uart_setBaudrate(MASTER_TO_SLAVE_UART, 1152);
            break;
        }
    }
}

/*********************************************************************
 * @fn      ipc_program_slave
 *
 * @brief   used by master to program slave at the beginning of user_main.
 */
void ipc_program_slave(void)
{
    uint8_t buffer[8];
    uint8_t do_handshake = 1;
    
    /* try to do handshake with program tools */
    uart_write(PC_TO_MASTER_UART, pc_tool_conn_req, sizeof(pc_tool_conn_req));
    if(program_serial_gets(PC_TO_MASTER_UART, 30, buffer, sizeof(app_boot_conn_ack))==sizeof(app_boot_conn_ack)) {
        if(memcmp(buffer, app_boot_conn_ack, sizeof(app_boot_conn_ack)) != 0) {
            do_handshake = 0;
        }
    }
    else {
        do_handshake = 0;
    }
    
    if(do_handshake) {
        /* try to do handshake with slave core */
        ipc_reset_slave();
        program_hw_init(1152);
        
        do {
            program_serial_gets(MASTER_TO_SLAVE_UART, 100, buffer, 1);
        } while(buffer[0] != app_boot_conn_req[0]);
        if(program_serial_gets(MASTER_TO_SLAVE_UART, 100, &buffer[1], sizeof(app_boot_conn_req)-1)==(sizeof(app_boot_conn_req)-1)) {
            if(memcmp(buffer, app_boot_conn_req, sizeof(app_boot_conn_req)) == 0) 
            {
                uart_write(MASTER_TO_SLAVE_UART, slave_program_conn_ack, sizeof(slave_program_conn_ack));
                program_serial_gets(MASTER_TO_SLAVE_UART, 10, buffer, sizeof(app_boot_conn_success));
                if(memcmp(buffer, app_boot_conn_success, sizeof(app_boot_conn_success)) != 0) {
                    while(1);
                }
                else {
                    uart_write(PC_TO_MASTER_UART, app_boot_conn_success, sizeof(app_boot_conn_success));
                }
                /* relay data between program tools and slave core */
                program_relay();
            }
            else {
                while(1);
            }
        }
    }
}

/*********************************************************************
 * @fn      ipc_recv_request
 *
 * @brief   receive transfer request from the other side, called by gpio interrupt.
 *
 * @param   pin_value : current GPIO value, used to check whether request is 
 *                      set or clear.
 * @return  None.
 */
void ipc_recv_request(uint32_t pin_value)
{
    LOG_INFO(ipc_log_tag, "ipc_recv_request.\r\n");
    if(ipc_local_side == IPC_SIDE_MASTER) {
        if((pin_value & GPIO_PA6) != 0) {
            ipc_reqeust_cleared = true;
            return;
        }
        else {
            if(ipc_reqeust_cleared) {
                ipc_reqeust_cleared = false;
            }
            else {
                return;
            }
        }
    }
    else if(ipc_local_side == IPC_SIDE_SLAVE) {
        if((pin_value & GPIO_PA6) != 0) {
            ipc_reqeust_cleared = true;
            return;
        }
        else {
            if(ipc_reqeust_cleared) {
                ipc_reqeust_cleared = false;
            }
            else {
                return;
            }
        }
    }
    else {
        return;
    }
    struct ipc_msg_t *msg;
    msg = (void *)os_malloc(sizeof(struct ipc_msg_t));
    msg->type = IPC_MSG_TYPE_READY;
    msg->preamble_sent = false;
    msg->callback = NULL;
    msg->length = 0;
    
    GLOBAL_INT_DISABLE();
    if(ipc_send_ongoing) {
        co_list_push_back(&ipc_msg_send_list, &msg->hdr);
    }
    else {
        /*
         * send receive ready as soon as possible. There is a scenario: local side 
         * is waiting for response to send a new message, and a new request from the
         * other side is received during waiting time. If this response message is 
         * push back of the sending list, IPC will blocked forever.
         */
        co_list_push_front(&ipc_msg_send_list, &msg->hdr);
    }
    GLOBAL_INT_RESTORE();

    send_next_frame();
}

/*********************************************************************
 * @fn      ipc_wakeup_hook
 *
 * @brief   used reinit uart and GPIO after wakeup frome sleep.
 */
void ipc_wakeup_hook(void)
{
    // TBD, put uart port to CPU
    
    init_uart();
}

/*********************************************************************
 * @fn      ipc_sleep_hook
 *
 * @brief   used to set GPIO state before enter sleep.
 */
void ipc_sleep_hook(void)
{
    // TBD, put uart port to PMU
}

/*********************************************************************
 * @fn      ipc_init
 *
 * @brief   used initial hardware and enviroment for IPC.
 *
 * @param   side : @ref ipc_side_t
 *          callback : callback when new message is received.
 * @return  None.
 */
void ipc_init(enum ipc_side_t side, void (*callback)(uint8_t *, uint16_t))
{
    co_list_init(&ipc_msg_send_list);
    ipc_msg_recv_callback = callback;
    ipc_local_side = side;

    init_hw();
}

/*********************************************************************
 * @fn      ipc_send_msg
 *
 * @brief   used to send data to the other side.
 *
 * @param   data : buffer pointer
 *          length : How many bytes to send
 *          callback: callback function when message is sent to the other side
 *          arg : parameter passed to callback function
 * @return  None.
 */
void ipc_send_msg(uint8_t *data, uint16_t length, void (*callback)(void*), void *arg)
{
    struct ipc_msg_t *msg;
    
    LOG_INFO(ipc_log_tag, "ipc_send_msg.\r\n");

    msg = os_malloc(sizeof(struct ipc_msg_t) + length);
    msg->type = IPC_MSG_TYPE_TX;
    msg->preamble_sent = false;
    msg->callback = callback;
    msg->arg = arg;
    msg->length = length;
    memcpy(msg->data, data, length);

    GLOBAL_INT_DISABLE();
    co_list_push_back(&ipc_msg_send_list, &msg->hdr);
    GLOBAL_INT_RESTORE();

    send_next_frame();
}

/*********************************************************************
 * @fn      ipc_reset_slave
 *
 * @brief   used by master to reset slave.
 */
void ipc_reset_slave(void)
{
    pmu_set_pin_to_PMU(GPIO_PORT_B, (1<<GPIO_BIT_6));
    pmu_set_pin_dir(GPIO_PORT_B, (1<<GPIO_BIT_6), GPIO_DIR_OUT);
    pmu_portb_write(pmu_portb_read() & (~(1<<GPIO_BIT_6)));
    co_delay_100us(100);
    pmu_portb_write(pmu_portb_read() | (1<<GPIO_BIT_6));
}

/*********************************************************************
 * @fn      ipc_uart_isr
 *
 * @brief   uart interrupt handler, used to receive message from the other side.
 */
__attribute__((section("ram_code"))) void ipc_uart_isr(void )
{
    void uart_IRQHandler(UART_HandleTypeDef *huart);
    uart_IRQHandler(&uart_handle);
}

