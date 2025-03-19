#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "plf.h"
#include "driver_i2c.h"
#include "driver_gpio.h"
#include "driver_system.h"
#include "driver_touchpad.h"

#include "sys_utils.h"

#define TOUCHPAD_TYPE_FR8000_EVM            1
#define TOUCHPAD_TYPE_1                     0

#define TOUCHPAD_IIC_SLAVE_ADDRESS          (0x15<<1)

#define TOUCHPAD_RX_BUFFER_SIZE             8

#if TOUCHPAD_TYPE_1
#define TOUCHPAD_RELEASE_RESET()            gpio_write_pin(GPIO_A, GPIO_PIN_5, GPIO_PIN_SET)
#define TOUCHPAD_SET_RESET()                gpio_write_pin(GPIO_A, GPIO_PIN_5, GPIO_PIN_CLEAR)
#endif
#if TOUCHPAD_TYPE_FR8000_EVM
#define TOUCHPAD_RELEASE_RESET()            gpio_write_pin(GPIO_B, GPIO_PIN_7, GPIO_PIN_SET)
#define TOUCHPAD_SET_RESET()                gpio_write_pin(GPIO_B, GPIO_PIN_7, GPIO_PIN_CLEAR)
#endif

I2C_HandleTypeDef  I2C0_Handle;

static uint8_t touchpad_rx_buffer[TOUCHPAD_RX_BUFFER_SIZE];

static void (*touchpad_int_callback)(void) = NULL;

static void touchpad_init_pin(bool enable_irq)
{
    GPIO_InitTypeDef GPIO_Handle;

#if TOUCHPAD_TYPE_1
    /* configure GPIO_PB4 and GPIO_PB5 to I2C0 mode */
    GPIO_Handle.Pin       = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_Handle.Mode      = GPIO_MODE_AF_PP;
    GPIO_Handle.Pull      = GPIO_PULLUP;
    GPIO_Handle.Alternate = GPIO_FUNCTION_1;
    gpio_init(GPIO_A, &GPIO_Handle);

    /* configure GPIO_PB7 to reset pin */
    __SYSTEM_GPIO_CLK_ENABLE();
    GPIO_Handle.Pin       = GPIO_PIN_5;
    GPIO_Handle.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_Handle.Alternate = GPIO_FUNCTION_0;
    gpio_init(GPIO_A, &GPIO_Handle);
    gpio_write_pin(GPIO_A, GPIO_PIN_5, GPIO_PIN_SET);

    /* configure GPIO_PB6 to interrupt pin */
    if(enable_irq) {
        GPIO_Handle.Pin       = GPIO_PIN_4;
        GPIO_Handle.Mode      = GPIO_MODE_EXTI_IT_FALLING;
        GPIO_Handle.Alternate = GPIO_FUNCTION_0;
        GPIO_Handle.Pull      = GPIO_PULLUP;
        gpio_init(GPIO_A, &GPIO_Handle);
    }
#endif

#if TOUCHPAD_TYPE_FR8000_EVM
    /* configure GPIO_PB4 and GPIO_PB5 to I2C0 mode */
    GPIO_Handle.Pin       = GPIO_PIN_4|GPIO_PIN_5;
    GPIO_Handle.Mode      = GPIO_MODE_AF_PP;
    GPIO_Handle.Pull      = GPIO_PULLUP;
    GPIO_Handle.Alternate = GPIO_FUNCTION_1;
    gpio_init(GPIO_B, &GPIO_Handle);

    /* configure GPIO_PB7 to reset pin */
    __SYSTEM_GPIO_CLK_ENABLE();
    GPIO_Handle.Pin       = GPIO_PIN_7;
    GPIO_Handle.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_Handle.Alternate = GPIO_FUNCTION_0;
    gpio_init(GPIO_B, &GPIO_Handle);
    gpio_write_pin(GPIO_B, GPIO_PIN_7, GPIO_PIN_SET);

    /* configure GPIO_PB6 to interrupt pin */
    if(enable_irq) {
        GPIO_Handle.Pin       = GPIO_PIN_6;
        GPIO_Handle.Mode      = GPIO_MODE_EXTI_IT_FALLING;
        GPIO_Handle.Alternate = GPIO_FUNCTION_0;
        GPIO_Handle.Pull      = GPIO_PULLUP;
        gpio_init(GPIO_B, &GPIO_Handle);
    }
#endif
}

static void touchpad_init_iic(void)
{
#if TOUCHPAD_TYPE_1
    __SYSTEM_I2C1_MASTER_CLK_SELECT_48M();
    __SYSTEM_I2C1_CLK_ENABLE();
    /* I2C Init */
    I2C0_Handle.I2Cx = I2C1;
    I2C0_Handle.Init.I2C_Mode = I2C_MODE_MASTER_7BIT;
    I2C0_Handle.Init.SCL_HCNT = 60;
    I2C0_Handle.Init.SCL_LCNT = 65;
    i2c_init(&I2C0_Handle);
#endif
#if TOUCHPAD_TYPE_FR8000_EVM
    __SYSTEM_I2C0_MASTER_CLK_SELECT_48M();
    __SYSTEM_I2C0_CLK_ENABLE();
    /* I2C Init */
    I2C0_Handle.I2Cx = I2C0;
    I2C0_Handle.Init.I2C_Mode = I2C_MODE_MASTER_7BIT;
    I2C0_Handle.Init.SCL_HCNT = 60;
    I2C0_Handle.Init.SCL_LCNT = 65;
    i2c_init(&I2C0_Handle);
#endif
}

void touchpad_read_data(uint8_t *rx_buffer, uint8_t length)
{
//    i2c_memory_read(&I2C0_Handle, TOUCHPAD_IIC_SLAVE_ADDRESS, 0, rx_buffer, length);
    memcpy(rx_buffer, touchpad_rx_buffer, length);
}

void touchpad_read_data_raw(uint8_t *rx_buffer, uint8_t length)
{
    i2c_memory_read(&I2C0_Handle, TOUCHPAD_IIC_SLAVE_ADDRESS, 0, rx_buffer, length);
}

void touchpad_init(void (*callback)(void))
{
    touchpad_init_pin(callback !=NULL);
    touchpad_init_iic();
    
    touchpad_int_callback = callback;

    /* reset touch pad chip */
    TOUCHPAD_RELEASE_RESET();
    co_delay_100us(100);
    TOUCHPAD_SET_RESET();
    co_delay_100us(50);
    TOUCHPAD_RELEASE_RESET();
    co_delay_100us(2000);

    if(callback) {
        NVIC_EnableIRQ(GPIO_IRQn);
    }
}

__attribute__((section("ram_code"))) void exti_isr(void)
{
    exti_get_LineStatus(EXTI_LINE14_PB6);
    exti_clear_LineStatus(EXTI_LINE14_PB6);
   
    i2c_memory_read(&I2C0_Handle, TOUCHPAD_IIC_SLAVE_ADDRESS, 0, touchpad_rx_buffer, TOUCHPAD_RX_BUFFER_SIZE);
    
    if(touchpad_int_callback) {
        touchpad_int_callback();
    }    
}

