#ifndef I2CSCANNER_H
#define I2CSCANNER_H

#include <Arduino.h>
#include <Wire.h>

class I2CScanner {
public:
    static void init();
    static void init(int sdaPin, int sclPin);
    static String scan();
    static String scanWithDetails();
    static void sendCommand(byte command);
    
private:
    static int SDA_PIN;
    static int SCL_PIN;
    static const int SCAN_START = 1;
    static const int SCAN_END = 128;
    static String getDeviceInfo(byte address);
    static bool testAddress(byte address);
};

#endif
