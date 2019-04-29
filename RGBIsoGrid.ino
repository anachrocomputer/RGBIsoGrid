/* RGBIsoGrid --- drive a 49-LED chain of WS8212B RGB LEDs in an isometric grid         2018-06-22 */

#include <avr/pgmspace.h>
#include "FastLED.h"


#define NUM_LEDS    (49)
#define BRIGHTNESS  (64)

#define BUTTON_PIN  (2)
#define DATA_PIN    (3)


CRGB Leds[NUM_LEDS];

struct polar {
  unsigned char r;
  unsigned int theta;
};

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

const unsigned char hex0[] = {18, 255};
const unsigned char hex1[] = {11, 12, 17, 25, 24, 19, 255};
const unsigned char hex2[] = {7, 6, 5, 13, 16, 26, 29, 30, 31, 23, 20, 10, 255};
const unsigned char hex3[] = {0, 1, 2, 3, 4, 14, 15, 27, 28, 36, 35, 34, 33, 32, 22, 21, 9, 8, 255};
const unsigned char hex4[] = {43, 44, 45, 46, 47, 48, 37, 38, 39, 40, 41, 42, 255};

const unsigned char row0[] = {43, 44, 45, 255};
const unsigned char row1[] = {0, 1, 2, 3, 255};
const unsigned char row2[] = {42, 8, 7, 6, 5, 4, 46, 255};
const unsigned char row3[] = {9, 10, 11, 12, 13, 14, 255};
const unsigned char row4[] = {41, 21, 20, 19, 18, 17, 16, 15, 47, 255};
const unsigned char row5[] = {22, 23, 24, 25, 26, 27, 255};
const unsigned char row6[] = {40, 32, 31, 30, 29, 28, 48, 255};
const unsigned char row7[] = {33, 34, 35, 36, 255};
const unsigned char row8[] = {39, 38, 37, 255};

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

const CRGB RandomColours[12] = {
  CRGB::Red, CRGB::Yellow, CRGB::Lime, CRGB::White,   CRGB::Blue, CRGB::Yellow, 
  CRGB::Red, CRGB::White,  CRGB::Lime, CRGB::Magenta, CRGB::Lime, CRGB::White
};


void setup(void)
{
  Serial.begin(9600);

  FastLED.addLeds<WS2812B,DATA_PIN>(Leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}


void loop(void)
{
  int ana;
  int led, level;
  int frame;
  int i;
  int hue;
  int dot = 0;
  int dir = 1;
  int mode = 0;

  for (frame = 0; ; frame++) {
    ana = analogRead(0);
    
    level = map(ana, 0, 1023, 0, 255);
    led = map(ana, 0, 1023, 0, NUM_LEDS - 1);

    switch (mode) {
    case 0:
      FastLED.showColor(CRGB::Yellow, level);
      break;
    case 1:
      FastLED.showColor(CHSV(level, 255, 255), BRIGHTNESS);
      break;
    case 2:
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
    case 3:
      FastLED.clear();
      Leds[led] = CRGB::Red;
      FastLED.show();
      break;
    case 4:
      for (i = 0; i < NUM_LEDS; i++) {
        if ((frame / 24) & 1)
          Leds[i] = pgm_read_dword(&(Frame02[i]));
        else
          Leds[i] = pgm_read_dword(&(Frame01[i]));
      }
  
      FastLED.show();
      break;
    case 5:
      FastLED.showColor(RandomColours[dot], level);
      if ((frame % 10) == 0)
        if (++dot > 11)
          dot = 0;
      break;
    case 6:
      for (i = 0; i < NUM_LEDS; i++) {
        hue = map(i, 0, NUM_LEDS - 1, 0, 255);
        hue = (hue + level) & 255;
        
        Leds[i] = CHSV(hue, 255, 255);
      }

      FastLED.show();
      break;
    case 7:
      FastLED.clear();
      
      sethex(hex0, CRGB::White);
      sethex(hex1, CHSV(level, 255, 255));
      sethex(hex2, CHSV((level + 64) & 255, 255, 255));
      sethex(hex3, CHSV((level + 128) & 255, 255, 255));
      sethex(hex4, CHSV((level + 192) & 255, 255, 255));
      FastLED.show();
      break;
    case 8:
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
    case 9:
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
    }

    if (digitalRead(BUTTON_PIN) == LOW) {
      while (digitalRead(BUTTON_PIN) == LOW)
        ;
   
      if (mode < 9)
        mode++;
      else
        mode = 0;
    }
    
    delay(20);
  }
}

void sethex(const unsigned char hex[], CRGB colour)
{
  int i;

  for (i = 0; (i < NUM_LEDS) && (hex[i] < NUM_LEDS); i++) {
    Leds[hex[i]] = colour;
  }
}

