
#ifndef __DRIVER_JD9854_H__
#define __DRIVER_JD9854_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

/************************************************************************************
 * @fn      jd9854_init
 *
 * @brief   Initial 8080, GPIO and DMA used by jd9854, and init jd9854 registers.
 */
void jd9854_init(void);

/************************************************************************************
 * @fn      jd9854_set_window
 *
 * @brief   used to define area of frame memory where MCU can access. 
 *
 * @param   x_s: SC.
 *          x_e: EC.
 *          y_s: SP.
 *          y_e: EP.
 */
void jd9854_set_window(uint16_t x_s, uint16_t x_e, uint16_t y_s, uint16_t y_e);

/************************************************************************************
 * @fn      jd9854_display_wait_transfer_done
 *
 * @brief   Used to wait for all data have been sent to jd9854 when DMA is used.
 */
void jd9854_display_wait_transfer_done(void);

/************************************************************************************
 * @fn      jd9854_display
 *
 * @brief   transfer data to framebuffer of jd9854 with DMA
 *
 * @param   pixel_count: total pixels count to be sent.
 *          data: pointer to data buffer
 *          callback: used to notify caller after DMA transfer is done.
 */
void jd9854_display(uint32_t pixel_count, uint16_t *data, void(*callback)(void));

/************************************************************************************
 * @fn      jd9854_display_gather
 *
 * @brief   used to send left or right part of frame with DMA gather mode
 *
 * @param   pixel_count: total pixels count to be sent.
 *          data: address of first pixel to be sent
 *          interval: lcd width
 *          count: data to be sent in a single line
 */
void jd9854_display_gather(uint32_t pixel_count, uint16_t *data, uint16_t interval, uint16_t count);

/************************************************************************************
 * @fn      jd9854_dma_isr
 *
 * @brief   DMA isr handler for jd9854.
 */
void jd9854_dma_isr(void);

#ifdef __cplusplus
}
#endif

#endif	/* __DRIVER_JD9854_H__ */

