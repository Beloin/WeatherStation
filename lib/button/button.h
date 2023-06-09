#include <stdio.h>
#include "freertos/FreeRTOS.h"

typedef void (*_on_pressed)(int tm, TickType_t ms);

typedef struct
{
    _on_pressed on_pressed;
    uint8_t press_quantity;
    uint8_t flag_up;

    uint8_t flag_any;
    uint8_t flag_prev;
    uint8_t _ignore_next;

    uint16_t limit_ms;

    TickType_t _timer, _last_press, last_press_duration, last_flag;
} Button;

/**
 * This already setup isr interrupt
 */
void setup_button(Button *button, uint32_t gpio_num, _on_pressed on_pressed, uint16_t limit_ms);

void poll_button(Button *button);