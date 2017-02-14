#include "Arduino.h"
#include "AudioSpectrum.h"

#include "Adafruit_NeoPixel.h"
#ifdef __AVR__
#include "avr/power.h"
#endif
#include "avr/pgmspace.h"

#define PIN 44
Adafruit_NeoPixel spectrum_strip = Adafruit_NeoPixel(280, PIN, NEO_GRB + NEO_KHZ800);

#include "PixelResource.h"


