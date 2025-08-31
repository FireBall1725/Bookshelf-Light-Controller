#include "TimeManager.h"

TimeManager::TimeManager() 
    : timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec),
      timeInitialized(false),
      lastSyncTime(0) {
}

void TimeManager::begin() {
    timeClient.begin();
    timeClient.setTimeOffset(timezoneOffset * 3600); // Convert hours to seconds
    timeClient.setUpdateInterval(60000); // Update every minute
    timeInitialized = false;
    lastSyncTime = 0;
}

void TimeManager::update() {
    timeClient.update();
    
    // Check if we need to sync time
    if (needsSync()) {
        syncTime();
    }
}

bool TimeManager::isTimeValid() {
    return timeInitialized && timeClient.isTimeSet();
}

String TimeManager::getFormattedTime() {
    if (!isTimeValid()) {
        return "00:00:00";
    }
    
    int hours = timeClient.getHours();
    int minutes = timeClient.getMinutes();
    int seconds = timeClient.getSeconds();
    
    return formatNumber(hours) + ":" + formatNumber(minutes) + ":" + formatNumber(seconds);
}

String TimeManager::getFormattedDateTime() {
    if (!isTimeValid()) {
        return "1970-01-01 00:00:00";
    }
    
    // Get time components from NTPClient
    int hours = timeClient.getHours();
    int minutes = timeClient.getMinutes();
    int seconds = timeClient.getSeconds();
    
    // Get epoch time for date calculation
    unsigned long epochTime = timeClient.getEpochTime();
    unsigned long daysSinceEpoch = epochTime / 86400;
    
    // Simple date calculation (approximate)
    // Starting from 1970-01-01
    unsigned long year = 1970;
    unsigned long month = 1;
    unsigned long day = 1;
    
    // Add days to get current date (simplified)
    while (daysSinceEpoch > 365) {
        daysSinceEpoch -= 365;
        year++;
    }
    
    // Convert remaining days to month/day (simplified)
    unsigned long monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (int i = 0; i < 12 && daysSinceEpoch > monthDays[i]; i++) {
        daysSinceEpoch -= monthDays[i];
        month++;
    }
    day += daysSinceEpoch;
    
    return String(year) + "-" + formatNumber(month) + "-" + formatNumber(day) + " " +
           formatNumber(hours) + ":" + formatNumber(minutes) + ":" + formatNumber(seconds);
}

String TimeManager::getTimestamp() {
    if (!isTimeValid()) {
        return "[1970-01-01 00:00:00]";
    }
    
    return "[" + getFormattedDateTime() + "]";
}

unsigned long TimeManager::getEpochTime() {
    if (!isTimeValid()) {
        return 0;
    }
    
    return timeClient.getEpochTime();
}

void TimeManager::setTimezone(int offset) {
    timezoneOffset = offset;
    timeClient.setTimeOffset(timezoneOffset * 3600);
}

void TimeManager::forceTimeSync() {
    syncTime();
}

bool TimeManager::needsSync() {
    return !timeInitialized || 
           (millis() - lastSyncTime > SYNC_INTERVAL) ||
           !timeClient.isTimeSet();
}

void TimeManager::syncTime() {
    if (WiFi.status() != WL_CONNECTED) {
        return; // Can't sync without WiFi
    }
    
    // Force NTP update
    timeClient.forceUpdate();
    
    // Wait for time to be set (with timeout)
    unsigned long startTime = millis();
    while (!timeClient.isTimeSet() && (millis() - startTime < 10000)) {
        delay(100);
    }
    
    if (timeClient.isTimeSet()) {
        timeInitialized = true;
        lastSyncTime = millis();
    }
}

String TimeManager::formatNumber(int number) {
    if (number < 10) {
        return "0" + String(number);
    }
    return String(number);
}

// Global instance
TimeManager timeManager;
