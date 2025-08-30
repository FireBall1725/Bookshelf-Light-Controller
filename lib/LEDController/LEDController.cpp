#include "LEDController.h"

CRGB LEDController::leds[NUM_LEDS];

void LEDController::init() {
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
}

void LEDController::setColor(CRGB color) {
    leds[0] = color;
    FastLED.show();
}

void LEDController::setColorByName(String colorName) {
    if (colorName == "red") setColor(CRGB::Red);
    else if (colorName == "green") setColor(CRGB::Green);
    else if (colorName == "blue") setColor(CRGB::Blue);
    else if (colorName == "yellow") setColor(CRGB::Yellow);
    else if (colorName == "purple") setColor(CRGB::Purple);
    else if (colorName == "cyan") setColor(CRGB::Cyan);
    else if (colorName == "white") setColor(CRGB::White);
    else if (colorName == "off") setColor(CRGB::Black);
}

void LEDController::startupSequence() {
    setColor(CRGB::Red);
    delay(500);
    setColor(CRGB::Green);
    delay(500);
    setColor(CRGB::Blue);
    delay(500);
    setColor(CRGB::Black);
}

void LEDController::wifiConfigMode() {
    setColor(CRGB::Blue);
}

void LEDController::wifiConnected() {
    setColor(CRGB::Green);
    delay(1000);
    setColor(CRGB::Black);
}

void LEDController::wifiFailed() {
    setColor(CRGB::Red);
    delay(1000);
    setColor(CRGB::Black);
}
