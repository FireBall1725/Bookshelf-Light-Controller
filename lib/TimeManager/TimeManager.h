#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

class TimeManager {
private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;
    bool timeInitialized;
    unsigned long lastSyncTime;
    const unsigned long SYNC_INTERVAL = 3600000; // Sync every hour (1 hour = 3,600,000 ms)
    
    // NTP server configuration
    const char* ntpServer = "pool.ntp.org";
    const long gmtOffset_sec = 0; // Will be set based on timezone
    const int daylightOffset_sec = 3600; // 1 hour for DST
    
    // Timezone offset (adjust for your location)
    int timezoneOffset = -5; // Eastern Time (UTC-5), adjust as needed
    
public:
    TimeManager();
    void begin();
    void update();
    bool isTimeValid();
    String getFormattedTime();
    String getFormattedDateTime();
    String getTimestamp();
    unsigned long getEpochTime();
    void setTimezone(int offset);
    void forceTimeSync();
    bool needsSync();
    
private:
    void syncTime();
    String formatNumber(int number);
};

extern TimeManager timeManager;

#endif // TIME_MANAGER_H
