![Static Badge](https://img.shields.io/badge/MCU-ATmega328-green "MCU:ATmega328")
![Static Badge](https://img.shields.io/badge/BOARD-Arduino-green "BOARD:Arduino")
![Static Badge](https://img.shields.io/badge/DISPLAY-WS2812-green "DISPLAY:WS2812")

# RGBIsoGrid

This Arduino sketch drives a string of 49 WS2812 RGB LEDs, arranged
in an isometric grid layout.
One additional LED (it's a 50-LED string) at the far end is unused.

The Arduino is also connected to a button and an analog potentiometer.
The button selects which demo to execute, and the pot controls the
demo.

## Library Usage

This sketch requires the use of the 'FastLED' library.
The library provides all the code to efficiently drive the LED string.

## Hardware Setup

The LED chain is connected to Arduino digital pin 3 (change this in the
code by changing 'DATA_PIN').
There's an analog pot connected to Arduino analog pin 0 for setting
hues, brightness and so on.
The mode selection is done by a push-button switch on Arduino digital
pin 2 ('BUTTON_PIN' in the code).
The button has a pull-up resistor connected, and it grounds the input pin
when pressed.
