/*
  ******************************************************************************
  * @file    ipc.h
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Header file of core communication.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __IPC_H__
#define __IPC_H__

#include <string.h>
#include <stdint.h>

enum ipc_side_t {
    IPC_SIDE_MASTER,
    IPC_SIDE_SLAVE,
};

/* Exported parameter ---------------------------------------------------------*/

/* Exported functions ---------------------------------------------------------*/

/*********************************************************************
 * @fn      ipc_program_slave
 *
 * @brief   used by master to program slave at the beginning of user_main.
 */
void ipc_program_slave(void);

/*********************************************************************
 * @fn      ipc_recv_request
 *
 * @brief   receive transfer request from the other side, called by gpio interrupt.
 *
 * @param   pin_value : current GPIO value, used to check whether request is 
 *                      set or clear.
 * @return  None.
 */
void ipc_recv_request(uint32_t pin_value);

/*********************************************************************
 * @fn      ipc_wakeup_hook
 *
 * @brief   used reinit uart and GPIO after wakeup frome sleep.
 */
void ipc_wakeup_hook(void);

/*********************************************************************
 * @fn      ipc_sleep_hook
 *
 * @brief   used to set GPIO state before enter sleep.
 */
void ipc_sleep_hook(void);

/*********************************************************************
 * @fn      ipc_init
 *
 * @brief   used initial hardware and enviroment for IPC.
 *
 * @param   side : @ref ipc_side_t
 *          callback : callback when new message is received.
 * @return  None.
 */
void ipc_init(enum ipc_side_t side, void (*callback)(uint8_t *, uint16_t));

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
void ipc_send_msg(uint8_t *data, uint16_t length, void (*callback)(void *), void *arg);

/*********************************************************************
 * @fn      ipc_uart_isr
 *
 * @brief   uart interrupt handler, used to receive message from the other side.
 */
void ipc_uart_isr(void );

/*********************************************************************
 * @fn      ipc_reset_slave
 *
 * @brief   used by master to reset slave.
 */
void ipc_reset_slave(void);

#endif  // __IPC_H__

