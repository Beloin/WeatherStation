#include "button.h"
#include <driver/gpio.h>

#define DEBOUNCE_TIME_MS 165
#define BUTTON_TIME_MS 500

void setup_button(Button *button, uint32_t gpio_num, void *(on_pressed)(int))
{
    button->flag_up = 0;
}

void IRAM_ATTR up_interrupt(void *args)
{
    Button *btn = (Button *)args;
    btn->flag_up = 1;
}

void configure_isr(uint32_t gpio_num, Button *button)
{
    gpio_install_isr_service(0);

    gpio_set_intr_type(gpio_num, GPIO_INTR_POSEDGE);
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT);

    gpio_intr_enable(gpio_num);
    gpio_isr_handler_add(gpio_num, up_interrupt, button);
}

void poll_button(Button *button)
{
    TickType_t current_tick, since;
    current_tick = xTaskGetTickCount();

    if (button->flag_up == 1)
    {
        since = current_tick - button->_timer;

        if (since > (DEBOUNCE_TIME_MS / portTICK_PERIOD_MS))
        {
            button->press_quantity++;

            button->_last_press = xTaskGetTickCount();
            button->_timer = xTaskGetTickCount();
            button->flag_up = 0;
        }
    }
    else
    {
        button->_timer = xTaskGetTickCount();
    }

    if (button->press_quantity > 0)
    {
        if (current_tick - button->_last_press >= (BUTTON_TIME_MS / portTICK_PERIOD_MS))
        {
            button->on_pressed(button->press_quantity);
            button->press_quantity = 0;
        }
    }
}