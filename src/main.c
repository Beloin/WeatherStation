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

#include <button.h>

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

#define DEBOUNCE_TIME_MS 165
#define BUTTON_TIME_MS 500

uint8_t application_mode = 0;

void update_representation();
void on_pressed(int press_qnt);

void on_pressed(int press_qnt)
{
    printf("Button Pressed %d Times\n", press_qnt);

    if (press_qnt == 1)
    {
        update_representation();
    }

    if (press_qnt == 2)
    {
    }
}

uint8_t current_representation = 1; // 1 = °C | 2 = °F | 4 = K
void update_representation()
{
    current_representation = (current_representation << 1);
    if (current_representation > 4)
    {
        current_representation = 1;
    }
}

char dh11value[25];
int dh11_count;
TaskHandle_t dht11_task;
void read_dht_task(void *arg)
{
    while (1)
    {
        int16_t humidity = 0, temperature = 0;
        char representation = 0;
        dht_read_data(DHT_TYPE_DHT11, SENSOR_GPIO, &humidity, &temperature);

        float float_temp = (float)temperature / 10.f;
        float humidity_percentage = (float)humidity / 10.f;
        switch (current_representation)
        {
        case 1:
            representation = 'C';
            break;
        case 2:
            representation = 'F';
            float_temp = (float_temp * 1.8) + 32; // °F = (°C * 9/5) + 32
            break;
        case 4:
            representation = 'K';
            float_temp += 273.15; // °K = °C + 273.15
            break;
        default:
            representation = 'C';
            break;
        }

        dh11_count = sprintf(dh11value, "%.1f%c%c | %.1f%c\n", float_temp, 0x80, representation, humidity_percentage, 0x25); // U+0025

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    debug("Starting Application\n");

    SSD1306_t dev;
    i2c_master_init(&dev, SDA_GPIO, SCL_GPIO, RESET_GPIO);
    dev._flip = true;

#if CONFIG_SSD1306_128x64
    ssd1306_init(&dev, 128, 64);
#endif // CONFIG_SSD1306_128x64

#if CONFIG_SSD1306_128x32
    ssd1306_init(&dev, 128, 32);
#endif // CONFIG_SSD1306_128x32

    Button button;
    setup_button(&button, BUTTON_GPIO, on_pressed);
    button.on_pressed = on_pressed;

    ssd1306_clear_screen(&dev, false);
    xTaskCreate(read_dht_task, "dht11task", 4096, NULL, 5, &dht11_task);

    while (1)
    {
        // Hard-coded State Machine:
        switch (application_mode)
        {
        case 0:
            ssd1306_display_text(&dev, 0, dh11value, dh11_count, false);
            break;
        case 1:
            // implement animation
        default:
            ssd1306_display_text(&dev, 0, dh11value, dh11_count, false);
            break;
        }

        poll_button(&button);
    }

    // Fade Out
    // ssd1306_fadeout(&dev);
}