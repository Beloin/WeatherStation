#include "button.h"

#include <driver/gpio.h>
#include "freertos/task.h"

#include <esp_err.h>

#define DEBOUNCE_TIME_MS 20
#define BUTTON_TIME_MS 500

void configure_isr(uint32_t gpio_num, Button *button);
void send_on_pressed(Button *button);

void setup_button(Button *button, uint32_t gpio_num, _on_pressed on_pressed, uint16_t limit_ms)
{
    button->flag_up = 0;
    button->flag_any = 0;
    button->flag_prev = 0;

    button->press_quantity = 0;
    button->_ignore_next = 0;
    button->on_pressed = on_pressed;
    button->limit_ms = limit_ms;

    configure_isr(gpio_num, button);
}

void IRAM_ATTR up_interrupt(void *args)
{
    Button *btn = (Button *)args;
    btn->flag_any = 1;
}

void configure_isr(uint32_t gpio_num, Button *button)
{
    esp_err_t err = gpio_install_isr_service(0);
    if (err)
    {
        printf("Error in `gpio_install_isr_service` = %d\n", err);
    }

    err = gpio_set_intr_type(gpio_num, GPIO_INTR_ANYEDGE); // Do this to use diff things
    if (err)
    {
        printf("Error in `gpio_set_intr_type` = %d\n", err);
    }

    err = gpio_set_direction(gpio_num, GPIO_MODE_INPUT);
    if (err)
    {
        printf("Error in `gpio_set_direction` = %d\n", err);
    }

    err = gpio_intr_enable(gpio_num);
    if (err)
    {
        printf("Error in `gpio_intr_enable` = %d\n", err);
    }

    err = gpio_isr_handler_add(gpio_num, up_interrupt, button);
    if (err)
    {
        printf("Error in `gpio_isr_handler_add` = %d\n", err);
    }
}

void send_on_pressed(Button *button)
{
    button->on_pressed(button->press_quantity, button->last_press_duration);
    button->press_quantity = 0;
}

void poll_button(Button *button)
{
    TickType_t current_tick, since;
    current_tick = xTaskGetTickCount();

    if (button->flag_any == 1)
    {
        since = current_tick - button->_timer;

        if (since > (DEBOUNCE_TIME_MS / portTICK_PERIOD_MS))
        {
            button->flag_any = 0;
            button->_timer = xTaskGetTickCount();

            if (button->flag_prev == 1)
            {
                button->flag_prev = 0;

                if (button->_ignore_next)
                {
                    button->_ignore_next = 0;
                }
                else
                {
                    button->press_quantity++;
                    button->last_press_duration = ((current_tick - button->last_flag) * portTICK_PERIOD_MS) + DEBOUNCE_TIME_MS;
                    button->_last_press = current_tick;
                }
            }
            else
            {
                button->last_flag = current_tick;
                button->flag_prev = 1;
            }
        }
    }
    else
    {
        button->_timer = xTaskGetTickCount();
    }

    if (!button->_ignore_next && button->flag_prev && ((current_tick - button->last_flag) * portTICK_PERIOD_MS) + DEBOUNCE_TIME_MS > 3000)
    {
        printf("Reached 3000ms\n");
        button->_timer = xTaskGetTickCount();
        button->_ignore_next = 1;
        button->press_quantity++;
        button->last_press_duration = ((current_tick - button->last_flag) * portTICK_PERIOD_MS) + DEBOUNCE_TIME_MS;
        button->_last_press = current_tick;
    }

    if (button->press_quantity > 0)
    {
        if (current_tick - button->_last_press >= (BUTTON_TIME_MS / portTICK_PERIOD_MS))
        {
            send_on_pressed(button);
        }
    }
}