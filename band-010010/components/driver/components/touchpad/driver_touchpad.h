#ifndef _DRIVER_TOUCHPAD_H
#define _DRIVER_TOUCHPAD_H

#include <stdint.h>

/************************************************************************************
 * @fn      touchpad_read_data
 *
 * @brief   used to read data from touchpad, these data are update in interrupt
 *
 * @param   rx_buffer: buffer used to store received data.
 *          length: length should be set to 8.
 */
void touchpad_read_data(uint8_t *rx_buffer, uint8_t length);

/************************************************************************************
 * @fn      touchpad_read_data_raw
 *
 * @brief   used to read data directly from touchpad registers.
 *
 * @param   rx_buffer: buffer used to store received data.
 *          length: length should be set to 8.
 */
void touchpad_read_data_raw(uint8_t *rx_buffer, uint8_t length);

/************************************************************************************
 * @fn      touchpad_init
 *
 * @brief   init GPIO, Interrupt and I2C used by touchpad
 *
 * @param   callback: used to notify application layer to fetch data
 */
void touchpad_init(void (*callback)(void));

#endif  // _DRIVER_TOUCHPAD_H
