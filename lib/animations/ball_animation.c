#include "freertos/FreeRTOS.h"
#include <stdio.h>
#include <ssd1306.h>

char const ball = 'O';

uint8_t curr_ball_row = 0;
uint8_t curr_ball_column = 0;
TickType_t last_ball_tick;
char row[16] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}; // 128 Bits per column


// TODO: Separar a animação do hardware de tela
void tick_ball_animation(SSD1306_t *dev, uint8_t rows, TickType_t current_tick)
{
    if (current_tick - last_ball_tick < (100 / portTICK_PERIOD_MS))
    {
        return;
    }
    last_ball_tick = current_tick;

    curr_ball_column++;
    if (curr_ball_column >= 16)
    {
        row[15] = ' ';
        curr_ball_column = 0;
        ssd1306_clear_line(dev, curr_ball_row++, false);
    }

    if (curr_ball_row == rows)
    {
        curr_ball_row = 0;
        row[15] = ' ';
    }

    if (curr_ball_column > 0)
    {
        row[curr_ball_column - 1] = ' ';
    }

    row[curr_ball_column] = ball;
    ssd1306_display_text(dev, curr_ball_row, row, 16, false);
}

void reset_ball_animation()
{
    curr_ball_column = 0;
    curr_ball_row = 0;

    for (uint8_t i = 0; i < 16; i++)
    {
        row[i] = ' ';
    }
}
