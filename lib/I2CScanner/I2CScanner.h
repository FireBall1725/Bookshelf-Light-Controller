#ifndef I2CSCANNER_H
#define I2CSCANNER_H

#include <Arduino.h>
#include <Wire.h>

class I2CScanner {
public:
    static void init();
    static String scan();
    static void sendCommand(byte command);
    
private:
    static const int SDA_PIN = 4;
    static const int SCL_PIN = 5;
    static const int SCAN_START = 1;
    static const int SCAN_END = 128;
    static String getDeviceInfo(byte address);
};

#endif
