# RGBIsoGrid

This Arduino sketch drives a string of 49 WS2812 RGB LEDs, arranged
in an isometric grid layout.
One additional LED (it's a 50-LED string) at the far end is unused.

The Arduino is also connected to a button and an analog potentiometer.
The button selects which demo to execute, and the pot controls the
demo.

## Libray Usage

This sketch requires the use of the 'FastLED' library.
The library provides all the code to efficiently drive the LED string.
