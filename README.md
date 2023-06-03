# Simple Weather Station

In this project, we show a simple Weather Application, using ESP32 and OLED.

- 1 Button Click: Change temperature representation
- 2 Button Clicks: Show Simple Animation
- 3 Button Clicks: Turn monitor off/on

To implement button `on_click` the GPIO_INTR_POSEDGE was used to interrupt on up edge of the click, as wrote in `lib/button/button.c`.
The animation is a simple ball animation, using "ticks" as a source of control in order to dont't hold the processor for too long.


## Circuit Diagram


<div class="side">
    <div class="middle">
        <p style="text-align: center">Harware Circuit</p>
        <img src="./docs/circuit2.jpeg" alt="Harware Circuits">
    </div>
    <div class="middle">
        <p style="text-align: center">Schematic Circuit</p>
        <img src="./docs/kicad.jpeg" alt="Schematic Circuits">
    </div>
</div>

<style>

.side {
    display: grid;
    grid-template-columns: 1fr 1fr 1fr;
    column-gap: 20px;
    place-items: center;
}

.middle {
}

.img {
    max-width: 100%;
    max-height: 100%;
}

</style>


## Documents about interrupt in ESP32
 - https://circuitdigest.com/microcontroller-projects/esp32-timers-and-timer-interrupts
 - https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/peripherals/timer.html#timer-api-timer-initialization


## References

1. SSD1306 datasheet, can be found [here](https://www.alldatasheet.com/view.jsp?Searchword=Ssd1306%20datasheet&gad=1&gclid=Cj0KCQjw0tKiBhC6ARIsAAOXutlKWRNAzstZ96tXT6xcJW5a0YPrZwLqGcYIT_aOV5m33F_SBqrCLvEaAmuXEALw_wcB).
1. ESP-IDF Implementation of SSD1306 by `nopnop2002`, can be found [here](https://github.com/nopnop2002/esp-idf-ssd1306).
1. DHT11 Datasheet can be found [here](https://www.alldatasheet.com/view.jsp?Searchword=Dht11%20datasheet&gad=1&gclid=Cj0KCQjw9deiBhC1ARIsAHLjR2AGMjLPtpIg5_IqeDmJ1VmYxvTDiGTHjKHhD6Tg9kuFvFfot_n41hsaAssLEALw_wcB).
1. DHT11 Driver implementation by `UncleRus`, can be found [here](https://github.com/UncleRus/esp-idf-lib/tree/master/components/dht).