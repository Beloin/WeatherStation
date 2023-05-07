#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

#define CONFIG_SSD1306_128x32 1
#define CONFIG_SSD1306_128x64 0

void app_main()
{
    debug("Starting Application\n");

    int16_t humidity = 0, temperature = 0;
    char dh11value[100];

    int center, top, bottom;
    char lineChar[20];

    SSD1306_t dev;
    i2c_master_init(&dev, SDA_GPIO, SCL_GPIO, RESET_GPIO);

    dev._flip = true;

    ssd1306_init(&dev, 128, 32);
    ssd1306_clear_screen(&dev, false);

#if CONFIG_SSD1306_128x64
    top = 2;
    center = 3;
    bottom = 8;
    ssd1306_display_text(&dev, 0, "SSD1306 128x64", 14, false);
    ssd1306_display_text(&dev, 1, "ABCDEFGHIJKLMNOP", 16, false);
    ssd1306_display_text(&dev, 2, "abcdefghijklmnop", 16, false);
    ssd1306_display_text(&dev, 3, "Hello World!!", 13, false);
    ssd1306_display_text(&dev, 4, "SSD1306 128x64", 14, true);
    ssd1306_display_text(&dev, 5, "ABCDEFGHIJKLMNOP", 16, true);
    ssd1306_display_text(&dev, 6, "abcdefghijklmnop", 16, true);
    ssd1306_display_text(&dev, 7, "Hello World!!", 13, true);
#endif // CONFIG_SSD1306_128x64

#if CONFIG_SSD1306_128x32
    top = 1;
    center = 1;
    bottom = 4;
    // ssd1306_display_text(&dev, 0, "SSD1306 128x32", 14, false);
    // ssd1306_display_text(&dev, 1, "Hello World!!", 13, false);
    // ssd1306_display_text(&dev, 2, "SSD1306 128x32", 14, true);
    // ssd1306_display_text(&dev, 3, "Hello World!!", 13, true);
#endif // CONFIG_SSD1306_128x32

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    int tt;
    while (1)
    {
        dht_read_data(DHT_TYPE_DHT11, SENSOR_GPIO, &humidity, &temperature);
        tt = sprintf(dh11value, "%.1fC | %.1f%c\n", (float)temperature / 10.f, (float)humidity / 10.f, 0x25); // U+0025

        ssd1306_display_text(&dev, 0, dh11value, tt, false);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // // Display Count Down
    // uint8_t image[24];
    // memset(image, 0, sizeof(image));
    // ssd1306_display_image(&dev, top, (6 * 8 - 1), image, sizeof(image));
    // ssd1306_display_image(&dev, top + 1, (6 * 8 - 1), image, sizeof(image));
    // ssd1306_display_image(&dev, top + 2, (6 * 8 - 1), image, sizeof(image));
    // for (int font = 0x39; font > 0x30; font--)
    // {
    //     memset(image, 0, sizeof(image));
    //     ssd1306_display_image(&dev, top + 1, (7 * 8 - 1), image, 8);
    //     memcpy(image, font8x8_basic_tr[font], 8);
    //     if (dev._flip)
    //         ssd1306_flip(image, 8);
    //     ssd1306_display_image(&dev, top + 1, (7 * 8 - 1), image, 8);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }

    // // Scroll Up
    // ssd1306_clear_screen(&dev, false);
    // ssd1306_contrast(&dev, 0xff);
    // ssd1306_display_text(&dev, 0, "---Scroll  UP---", 16, true);
    // // ssd1306_software_scroll(&dev, 7, 1);
    // ssd1306_software_scroll(&dev, (dev._pages - 1), 1);
    // for (int line = 0; line < bottom + 10; line++)
    // {
    //     lineChar[0] = 0x01;
    //     sprintf(&lineChar[1], " Line %02d", line);
    //     ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
    //     vTaskDelay(500 / portTICK_PERIOD_MS);
    // }
    // vTaskDelay(3000 / portTICK_PERIOD_MS);

    // // Scroll Down
    // ssd1306_clear_screen(&dev, false);
    // ssd1306_contrast(&dev, 0xff);
    // ssd1306_display_text(&dev, 0, "--Scroll  DOWN--", 16, true);
    // ssd1306_software_scroll(&dev, 1, (dev._pages - 1));
    // for (int line = 0; line < bottom + 10; line++)
    // {
    //     lineChar[0] = 0x02;
    //     sprintf(&lineChar[1], " Line %02d", line);
    //     ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
    //     vTaskDelay(500 / portTICK_PERIOD_MS);
    // }
    // vTaskDelay(3000 / portTICK_PERIOD_MS);

    // // Page Down
    // ssd1306_clear_screen(&dev, false);
    // ssd1306_contrast(&dev, 0xff);
    // ssd1306_display_text(&dev, 0, "---Page	DOWN---", 16, true);
    // ssd1306_software_scroll(&dev, 1, (dev._pages - 1));
    // for (int line = 0; line < bottom + 10; line++)
    // {
    //     if ((line % (dev._pages - 1)) == 0)
    //         ssd1306_scroll_clear(&dev);
    //     lineChar[0] = 0x02;
    //     sprintf(&lineChar[1], " Line %02d", line);
    //     ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
    //     vTaskDelay(500 / portTICK_PERIOD_MS);
    // }
    // vTaskDelay(3000 / portTICK_PERIOD_MS);

    // // Horizontal Scroll
    // ssd1306_clear_screen(&dev, false);
    // ssd1306_contrast(&dev, 0xff);
    // ssd1306_display_text(&dev, center, "Horizontal", 10, false);
    // ssd1306_hardware_scroll(&dev, SCROLL_RIGHT);
    // vTaskDelay(5000 / portTICK_PERIOD_MS);
    // ssd1306_hardware_scroll(&dev, SCROLL_LEFT);
    // vTaskDelay(5000 / portTICK_PERIOD_MS);
    // ssd1306_hardware_scroll(&dev, SCROLL_STOP);

    // // Vertical Scroll
    // ssd1306_clear_screen(&dev, false);
    // ssd1306_contrast(&dev, 0xff);
    // ssd1306_display_text(&dev, center, "Vertical", 8, false);
    // ssd1306_hardware_scroll(&dev, SCROLL_DOWN);
    // vTaskDelay(5000 / portTICK_PERIOD_MS);
    // ssd1306_hardware_scroll(&dev, SCROLL_UP);
    // vTaskDelay(5000 / portTICK_PERIOD_MS);
    // ssd1306_hardware_scroll(&dev, SCROLL_STOP);

    // // Invert
    // ssd1306_clear_screen(&dev, true);
    // ssd1306_contrast(&dev, 0xff);
    // ssd1306_display_text(&dev, center, "  Good Bye!!", 12, true);
    // vTaskDelay(5000 / portTICK_PERIOD_MS);

    // Fade Out
    ssd1306_fadeout(&dev);
}