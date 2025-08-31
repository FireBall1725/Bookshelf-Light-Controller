#include "mock_arduino.h"

// Mock Arduino function implementations
unsigned long millis() { return 1000; }
void delay(unsigned long ms) { /* Mock delay */ }
void yield() { /* Mock yield */ }

// Global mock instances
MockSPIFFS SPIFFS;
MockWiFi WiFi;
MockWire Wire;
MockWebServer server;
MockHTTPUpload upload;
MockSerial Serial;
