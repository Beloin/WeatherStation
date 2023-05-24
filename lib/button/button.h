#include <stdio.h>
#include "freertos/FreeRTOS.h"

typedef struct
{
    void (*on_pressed)(int);
    uint8_t press_quantity;
    uint8_t flag_up;
    TickType_t _timer, _last_press;
} Button;

/**
 * This already setup isr interrupt
 */
void setup_button(Button *button, uint32_t gpio_num, void (on_pressed)(int));

void poll_button(Button *button);