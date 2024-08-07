/* RGBIsoGrid --- drive a 49-LED chain of WS8212B RGB LEDs in an isometric grid         2018-06-22 */

#include <avr/pgmspace.h>
#include "FastLED.h"


#define NUM_LEDS    (49)
#define BRIGHTNESS  (64)

#define BUTTON_PIN  (2)   // A push-button switch connected to ground, with a pull-up resistor
#define DATA_PIN    (3)   // Connection to the WS2812 LED chain


CRGB Leds[NUM_LEDS];

struct polar {
  unsigned char r;
  unsigned int theta;
};

// LED positions expressed as polar co-ordinates
const struct polar Posn[NUM_LEDS] = {
  {192, 120}, {192, 100}, {192, 80},  {192, 60},                                      // 0-3
  {192, 40},  {128, 60},  {128, 90},  {128, 120}, {192, 140},                         // 4-8
  {192, 160}, {128, 150}, {64, 120},  {64, 60},   {128, 30},  {192, 20},              // 9-14
  {192, 0},   {128, 0},   {64, 0},    {0, 0},     {64, 180},  {128, 180}, {192, 180}, // 15-21
  {192, 200}, {128, 210}, {64, 240},  {64, 300},  {128, 330}, {192, 340},             // 22-27
  {192, 320}, {128, 300}, {128, 270}, {128, 240}, {192, 220},                         // 28-32
  {192, 240}, {192, 260}, {192, 280}, {192, 300},                                     // 33-36
  {255, 300}, {255, 270}, {255, 240}, {255, 210}, {255, 180}, {255, 150},             // 37-42
  {255, 120}, {255, 90},  {255, 60},  {255, 30},  {255, 0},   {255, 330}              // 43-48
};

// Concentric hexagons, in terms of LED numbers
const unsigned char hex0[] = {18, 255};
const unsigned char hex1[] = {11, 12, 17, 25, 24, 19, 255};
const unsigned char hex2[] = {7, 6, 5, 13, 16, 26, 29, 30, 31, 23, 20, 10, 255};
const unsigned char hex3[] = {0, 1, 2, 3, 4, 14, 15, 27, 28, 36, 35, 34, 33, 32, 22, 21, 9, 8, 255};
const unsigned char hex4[] = {43, 44, 45, 46, 47, 48, 37, 38, 39, 40, 41, 42, 255};

// Horizontal rows on the hex grid, in terms of LED numbers
const unsigned char row0[] = {43, 44, 45, 255};
const unsigned char row1[] = {0, 1, 2, 3, 255};
const unsigned char row2[] = {42, 8, 7, 6, 5, 4, 46, 255};
const unsigned char row3[] = {9, 10, 11, 12, 13, 14, 255};
const unsigned char row4[] = {41, 21, 20, 19, 18, 17, 16, 15, 47, 255};
const unsigned char row5[] = {22, 23, 24, 25, 26, 27, 255};
const unsigned char row6[] = {40, 32, 31, 30, 29, 28, 48, 255};
const unsigned char row7[] = {33, 34, 35, 36, 255};
const unsigned char row8[] = {39, 38, 37, 255};

// Decimal digits 0-9, in terms of LED numbers
const unsigned char digit0[] = {0, 1, 2, 3, 4, 14, 15, 27, 28, 36, 35, 34, 33, 32, 22, 21, 9, 8, 255};
const unsigned char digit1[] = {0, 7, 11, 18, 25, 29, 36, 35, 255};
const unsigned char digit2[] = {3, 2, 1, 0, 8, 9, 20, 19, 18, 17, 16, 27, 28, 36, 35, 34, 33, 255};
const unsigned char digit3[] = {0, 1, 2, 3, 4, 14, 16, 17, 18, 19, 27, 28, 36, 35, 34, 33, 255};
const unsigned char digit4[] = {0, 7, 11, 18, 25, 29, 36, 33, 32, 22, 21, 20, 19, 17, 16, 15, 255};
const unsigned char digit5[] = {0, 1, 2, 3, 4, 14, 16, 17, 18, 19, 20, 22, 32, 33, 34, 35, 36, 255};
const unsigned char digit6[] = {0, 1, 2, 3, 4, 8, 9, 14, 16, 17, 18, 19, 20, 21, 22, 32, 33, 34, 35, 36, 255};
const unsigned char digit7[] = {0, 7, 11, 17, 18, 19, 25, 29, 36, 35, 34, 33, 255};
const unsigned char digit8[] = {0, 1, 2, 3, 4, 8, 9, 14, 27, 28, 36, 35, 34, 33, 32, 22, 20, 19, 18, 17, 16, 255};
const unsigned char digit9[] = {0, 1, 2, 3, 4, 14, 15, 27, 28, 36, 35, 34, 33, 32, 22, 20, 19, 18, 17, 16, 255};

// Two pre-set frames of animation
const long Frame01[NUM_LEDS] PROGMEM =
{
0xff0000, 0x000000, 0x000000, 0x00ff00, 0x000000, 0x00ff00, 0x000000, 0xff0000, // 0-7
0x000000, 0x000000, 0x000000, 0xff0000, 0x00ff00, 0x000000, 0x000000, 0x0000ff, // 8-15
0x0000ff, 0x0000ff, 0xffffff, 0x0000ff, 0x0000ff, 0x0000ff, 0x000000, 0x000000, // 16-23
0x00ff00, 0xff0000, 0x000000, 0x000000, 0x000000, 0xff0000, 0x000000, 0x00ff00, // 24-31
0x000000, 0x00ff00, 0x000000, 0x000000, 0xff0000, 0xff0000, 0x000000, 0x00ff00, // 32-39
0x000000, 0x0000ff, 0x000000, 0xff0000, 0x000000, 0x00ff00, 0x000000, 0x0000ff, // 40-47
0x000000                                                                        // 48
};

const long Frame02[NUM_LEDS] PROGMEM =
{
0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x00ff00, 0x00ff00, 0x00ff00, // 0-7
0x0000ff, 0x0000ff, 0x00ff00, 0xff0000, 0xff0000, 0x00ff00, 0x0000ff, 0x0000ff, // 8-15
0x00ff00, 0xff0000, 0xffffff, 0xff0000, 0x00ff00, 0x0000ff, 0x0000ff, 0x00ff00, // 16-23
0xff0000, 0xff0000, 0x00ff00, 0x0000ff, 0x0000ff, 0x00ff00, 0x00ff00, 0x00ff00, // 24-31
0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x00ffff, 0x00ffff, 0x00ffff,
0x00ffff, 0x00ffff, 0x00ffff, 0x00ffff, 0x00ffff, 0x00ffff, 0x00ffff, 0x00ffff, // 40-47
0x00ffff                                                                        // 48
};

// Some random LED colours
const CRGB RandomColours[12] = {
  CRGB::Red, CRGB::Yellow, CRGB::Lime, CRGB::White,   CRGB::Blue, CRGB::Yellow, 
  CRGB::Red, CRGB::White,  CRGB::Lime, CRGB::Magenta, CRGB::Lime, CRGB::White
};


// Rows of pink, blue & white
const CRGB FlagStripes[9] = {
  CRGB::Blue,  CRGB::Blue,
  CRGB::Magenta,  CRGB::Magenta,  // Magenta works better as a pink on my LEDs
  CRGB::White,
  CRGB::Magenta,  CRGB::Magenta,
  CRGB::Blue,  CRGB::Blue
};

// Six 60degree sectors of pink, blue & white
const CRGB FlagSectors[6] = {
  CRGB::Blue, CRGB::Magenta, CRGB::White,
  CRGB::Magenta, CRGB::Blue, CRGB::White
};


void setup(void)
{
  Serial.begin(9600);

  FastLED.addLeds<WS2812B,DATA_PIN>(Leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}


void loop(void)
{
  int ana;        // Analog pot reading, 0-1023
  int led, row, level;
  int frame;
  int i;
  int hue;
  int dot = 0;
  int dir = 1;    // Direction flag
  int digit = 0;  // Digit 0-9
  int mode = 0;   // Demo mode, selected by the push-button switch

  for (frame = 0; ; frame++) {
    ana = analogRead(0);    // Read analog pot
    
    level = map(ana, 0, 1023, 0, 255);          // Scale pot for 0-255
    row = map(ana, 0, 1023, 0, 9);              // Scale pot for 0-8 (row number)
    led = map(ana, 0, 1023, 0, NUM_LEDS - 1);   // Scale pot for LED index

    switch (mode) {
    case 0:   // Just show a fixed colour, with adjustable brightness
      FastLED.showColor(CRGB::Yellow, level);
      break;
    case 1:   // Show an adjustable hue, at fixed brightness
      FastLED.showColor(CHSV(level, 255, 255), BRIGHTNESS);
      break;
    case 2:   // Single LED lit, scanning up and down the chain
      FastLED.clear();
      Leds[dot] = CHSV(level, 255, 255);
      FastLED.show();
      Leds[dot] = CRGB::Black; // clear this LED for the next time around the loop

      if (frame & 1) {
        if (dot >= (NUM_LEDS - 1))
          dir = -1;
        else if (dot <= 0)
          dir = 1;

        dot += dir;
      }
      break;
    case 3:   // Pot controls which LED in chain is lit in red
      FastLED.clear();
      Leds[led] = CRGB::Red;
      FastLED.show();
      break;
    case 4:   // Display two alternating frames of animation (from Program memory)
      for (i = 0; i < NUM_LEDS; i++) {
        if ((frame / 24) & 1)
          Leds[i] = pgm_read_dword(&(Frame02[i]));
        else
          Leds[i] = pgm_read_dword(&(Frame01[i]));
      }
  
      FastLED.show();
      break;
    case 5:   // Display random colours (all LEDs the same colour)
      FastLED.showColor(RandomColours[dot], level);
      if ((frame % 10) == 0)
        if (++dot > 11)
          dot = 0;
      break;
    case 6:   // Hue-cycling by concentric hexagons
      FastLED.clear();
      
      sethex(hex0, CRGB::White);
      sethex(hex1, CHSV(level, 255, 255));
      sethex(hex2, CHSV((level + 64) & 255, 255, 255));
      sethex(hex3, CHSV((level + 128) & 255, 255, 255));
      sethex(hex4, CHSV((level + 192) & 255, 255, 255));
      FastLED.show();
      break;
    case 7:   // Hue-cycling by row
      FastLED.clear();
      
      sethex(row0, CHSV(level, 255, 255));
      sethex(row1, CHSV((level + 28) & 255, 255, 255));
      sethex(row2, CHSV((level + 57) & 255, 255, 255));
      sethex(row3, CHSV((level + 85) & 255, 255, 255));
      sethex(row4, CHSV((level + 114) & 255, 255, 255));
      sethex(row5, CHSV((level + 142) & 255, 255, 255));
      sethex(row6, CHSV((level + 171) & 255, 255, 255));
      sethex(row7, CHSV((level + 199) & 255, 255, 255));
      sethex(row8, CHSV((level + 228) & 255, 255, 255));
      FastLED.show();
      break;
    case 8:   // Hue-cycling rows of pink, blue & white
      FastLED.clear();
      
      sethex(row0, FlagStripes[row % 9]);
      sethex(row1, FlagStripes[(row + 1) % 9]);
      sethex(row2, FlagStripes[(row + 2) % 9]);
      sethex(row3, FlagStripes[(row + 3) % 9]);
      sethex(row4, FlagStripes[(row + 4) % 9]);
      sethex(row5, FlagStripes[(row + 5) % 9]);
      sethex(row6, FlagStripes[(row + 6) % 9]);
      sethex(row7, FlagStripes[(row + 7) % 9]);
      sethex(row8, FlagStripes[(row + 8) % 9]);
      FastLED.show();
      break;
    case 9:   // Rotating hue-cycle
      FastLED.clear();

      for (i = 0; i < NUM_LEDS; i++) {
        const unsigned char r = Posn[i].r;
        const unsigned int theta = Posn[i].theta;

        hue = map(theta, 0, 360, 0, 255);
        hue = (hue + level) & 255;

        Leds[i] = CHSV(hue, 255, 255);
      }

      Leds[hex0[0]] = CRGB::White;

      FastLED.show();
      break;
    case 10:  // Rotating pink, blue & white
      FastLED.clear();

      for (i = 0; i < NUM_LEDS; i++) {
        const unsigned int theta = (Posn[i].theta + level) % 360;

        hue = map(theta, 0, 360, 0, 6);
        Leds[i] = FlagSectors[hue];
      }

      Leds[hex0[0]] = CRGB::White;

      FastLED.show();
      break;
    case 11:  // Display digits 0-9, pot adjusts hue
      FastLED.clear();
      if ((frame % 75) == 0)
        if (digit < 9)
          digit++;
        else
          digit = 0;

      switch (digit) {
      case 0:
        sethex(digit0, CHSV(level, 255, 255));
        break;
      case 1:
        sethex(digit1, CHSV(level, 255, 255));
        break;
      case 2:
        sethex(digit2, CHSV(level, 255, 255));
        break;
      case 3:
        sethex(digit3, CHSV(level, 255, 255));
        break;
      case 4:
        sethex(digit4, CHSV(level, 255, 255));
        break;
      case 5:
        sethex(digit5, CHSV(level, 255, 255));
        break;
      case 6:
        sethex(digit6, CHSV(level, 255, 255));
        break;
      case 7:
        sethex(digit7, CHSV(level, 255, 255));
        break;
      case 8:
        sethex(digit8, CHSV(level, 255, 255));
        break;
      case 9:
        sethex(digit9, CHSV(level, 255, 255));
        break;
      }
      FastLED.show();
      break;
    }

    if (digitalRead(BUTTON_PIN) == LOW) {
      while (digitalRead(BUTTON_PIN) == LOW)
        ;
   
      if (mode < 11)
        mode++;
      else
        mode = 0;
    }
    
    delay(20);
  }
}


/* sethex --- set a list of LEDs to a given colour */

void sethex(const unsigned char hex[], CRGB colour)
{
  int i;

  for (i = 0; (i < NUM_LEDS) && (hex[i] < NUM_LEDS); i++) {
    Leds[hex[i]] = colour;
  }
}
