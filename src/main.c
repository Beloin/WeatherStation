#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/gpio.h>

#include <ssd1306.h>
#include <debug.h>
#include <font8x8_basic.h>

#include <dht.h>
#include <stdio.h>

// Project:

// Simple Weather Program: Show weather (Temperature and humidity) on OLED.
// On button click:
//  - 1 Click: Change to °F
//  - 2 Clicks: Show pretty animation // Use state machine to block other uses between clicks
//  - Hold 3secs: Shutdown/turnon screen (Battery Saver)

#define SDA_GPIO 21
#define SCL_GPIO 22
#define RESET_GPIO 33

#define SENSOR_GPIO 4

#define BUTTON_GPIO GPIO_NUM_12

#define CONFIG_SSD1306_128x32 1
#define CONFIG_SSD1306_128x64 0

#define DEBOUNCE_TIME_MS 50
#define BUTTON_TIME_MS 1000

TaskHandle_t buttonTaskHandler = NULL;

uint8_t lastButtonState = 0;

uint8_t buttonState = 0;
uint8_t schrodingerButtonState = 0;
uint8_t stableMs = 0;

uint8_t buttonPressed = 0;
uint8_t buttonReleased = 0;

// TODO: Later implement via interrupt
void checkButton()
{
    TickType_t current_tick;
    schrodingerButtonState = gpio_get_level(BUTTON_GPIO);

    if (buttonState == schrodingerButtonState)
    {
        current_tick = xTaskGetTickCount();
        stableMs += current_tick;
    }
    else
    {
        buttonState = schrodingerButtonState;
        stableMs = 0;
    }

    if (stableMs >= DEBOUNCE_TIME_MS)
    {
        if (buttonState)
        {
            buttonPressed = 1;
            buttonReleased = 0;
        }
        else
        {
            if (lastButtonState)
            {
                buttonPressed = 0;
                buttonReleased = 1;
            }
            else
            {
                buttonPressed = 0;
            }
        }

        lastButtonState = buttonState;
        stableMs = 0;
    }
}

uint8_t isPressed()
{
    uint8_t isPressed = buttonPressed;
    buttonPressed = 0;
    return isPressed;
}

uint8_t isReleased()
{
    uint8_t isReleased = buttonReleased;
    buttonReleased = 0;
    return isReleased;
}

void ButtonTask(void *arg)
{
    while (1)
    {
        checkButton();

        if (isPressed())
        {
            printf("Button Pressed");
        }

        if (isReleased())
        {
            printf("Button Released");
        }
    }
}

void app_main()
{
    debug("Starting Application\n");

    gpio_config_t button_config = {.mode = GPIO_MODE_INPUT, .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&button_config);
    gpio_set_level(BUTTON_GPIO, 0);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);

    // gpio_pulldown_en(BUTTON_GPIO); // How to enable via softwre?

    int16_t humidity = 0, temperature = 0;
    char dh11value[100];

    SSD1306_t dev;
    i2c_master_init(&dev, SDA_GPIO, SCL_GPIO, RESET_GPIO);
    dev._flip = true;

    ssd1306_clear_screen(&dev, false);

#if CONFIG_SSD1306_128x64
    ssd1306_init(&dev, 128, 64);
#endif // CONFIG_SSD1306_128x64

#if CONFIG_SSD1306_128x32
    ssd1306_init(&dev, 128, 32);
#endif // CONFIG_SSD1306_128x32

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    xTaskCreate(ButtonTask, "ButtonTask", 4096, NULL, 10, &buttonTaskHandler);

    int string_count;
    while (1)
    {
        dht_read_data(DHT_TYPE_DHT11, SENSOR_GPIO, &humidity, &temperature);
        string_count = sprintf(dh11value, "%.1fC | %.1f%c\n", (float)temperature / 10.f, (float)humidity / 10.f, 0x25); // U+0025

        ssd1306_display_text(&dev, 0, dh11value, string_count, false);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Fade Out
    ssd1306_fadeout(&dev);
}