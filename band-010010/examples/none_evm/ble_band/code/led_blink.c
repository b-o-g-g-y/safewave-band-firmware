/**
 * Simple LED blink program
 */
#include <stdint.h>
#include "driver_system.h"
#include "driver_gpio.h"
#include "co_log.h"

// Using PA0 as LED pin
#define LED_PORT GPIO_A
#define LED_PIN GPIO_PIN_5

void system_init(void)
{
    // Initialize system clock
    system_set_port_mux(LED_PORT, LED_PIN, 0);

    // Configure LED pin
    GPIO_InitTypeDef gpio_config;
    gpio_config.Mode = GPIO_MODE_OUTPUT_PP; // Push-pull output
    gpio_config.Pull = GPIO_PULLUP;         // Pull-up enabled
    gpio_config.Pin = (1 << LED_PIN);       // PA0
    gpio_init(LED_PORT, &gpio_config);
}

int main(void)
{
    // Initialize system
    system_init();

    // Main loop - blink LED
    while (1)
    {
        // Turn LED on
        gpio_write_pin(LED_PORT, LED_PIN, GPIO_PIN_SET);

        // Delay
        for (volatile int i = 0; i < 500000; i++)
            ;

        // Turn LED off
        gpio_write_pin(LED_PORT, LED_PIN, GPIO_PIN_CLEAR);

        // Delay
        for (volatile int i = 0; i < 500000; i++)
            ;
    }
}