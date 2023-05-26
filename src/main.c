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

// TODO: Create animation... How?
char const ball = 'O';

uint8_t curr_ball_row = 0;
uint8_t curr_ball_column = 0;
TickType_t last_ball_tick;
char row[16] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}; // 128 Bits per column

// TODO: Make it not overload main thread.
void ball_animation(SSD1306_t *dev, uint8_t rows)
{
    uint8_t i, j;

    ssd1306_clear_screen(dev, false);
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < 16; j++)
        {
            if (j > 0)
                row[j - 1] = ' ';
            row[j] = ball;

            ssd1306_display_text(dev, i, row, 16, false);
            vTaskDelay(220 / portTICK_PERIOD_MS);
        }
        row[j - 1] = ' ';

        ssd1306_clear_line(dev, i, false);
    }
    ssd1306_clear_screen(dev, false);
}

void tick_ball_animation(SSD1306_t *dev, uint8_t rows, TickType_t current_tick)
{
    if (current_tick - last_ball_tick < (200 / portTICK_PERIOD_MS))
    {
        return;
    }

    curr_ball_column++;
    if (curr_ball_column >= 16)
    {
        curr_ball_column = 0;
        ssd1306_clear_line(dev, curr_ball_row++, false);
    }

    if (curr_ball_row == rows)
    {
        curr_ball_row = 0;
        // for (uint8_t i = 0; i < 16; i++)
        // {
        //     row[i] = ' ';
        // }
        row[15] = ' ';
    }

    if (curr_ball_column > 0)
    {
        row[curr_ball_column - 1] = ' ';
    }

    row[curr_ball_column] = ball;
    ssd1306_display_text(dev, curr_ball_row, row, 16, false);
}

// TODO: How to turn off? i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_OFF, true);				// AE

void update_representation();
void on_pressed(int press_qnt);

void on_pressed(int press_qnt)
{
    printf("Button Pressed %d Times\n", press_qnt);

    if (press_qnt == 1)
    {
        if (application_mode == 0)
        {
            update_representation();
        }
    }

    if (press_qnt == 2)
    {
        application_mode = application_mode ^ 0x1;
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
    uint8_t rows = 64;
    ssd1306_init(&dev, 128, 64);
#endif // CONFIG_SSD1306_128x64

#if CONFIG_SSD1306_128x32
    uint8_t rows = 32;
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
            // TODO: Work as "ticks", so animation will not stop the button click
            // ball_animation(&dev, rows / 8); // rows / 8 since we will be using pages
            // application_mode = 0;
            tick_ball_animation(&dev, rows / 8, xTaskGetTickCount());
            break;
        default:
            ssd1306_display_text(&dev, 0, dh11value, dh11_count, false);
            break;
        }

        poll_button(&button);
    }

    // Fade Out
    // ssd1306_fadeout(&dev);
}