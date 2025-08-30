#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

#include <Arduino.h>
#include <FastLED.h>

class LEDController {
public:
    static const int NUM_LEDS = 1;
    static const int LED_PIN = 8;
    static const int BRIGHTNESS = 128;
    
    static void init();
    static void setColor(CRGB colour);
    static void setColorByName(String colourName);
    static void setBrightness(uint8_t brightness);
    static void startupSequence();
    static void wifiConfigMode();
    static void wifiConnected();
    static void wifiFailed();
    
private:
    static CRGB leds[NUM_LEDS];
    static void show();
};

#endif
