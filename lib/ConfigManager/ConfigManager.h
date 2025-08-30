#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

struct MQTTConfig {
    String brokerIP;
    int brokerPort;
    String username;
    String password;
    String deviceName;
    String deviceId;
    String mqttPrefix;
};

struct WiFiConfig {
    String ssid;
    String password;
};

class ConfigManager {
public:
    static bool init();
    static bool loadConfig();
    static bool saveConfig();
    
    // MQTT Configuration
    static MQTTConfig getMQTTConfig();
    static bool updateMQTTConfig(const String& brokerIP, int brokerPort, 
                                const String& username, const String& password,
                                const String& deviceName, const String& deviceId,
                                const String& mqttPrefix);
    
    // WiFi Configuration
    static WiFiConfig getWiFiConfig();
    static void setWiFiConfig(const String& ssid, const String& password);

private:
    static MQTTConfig mqttConfig;
    static WiFiConfig wifiConfig;
    static const char* CONFIG_FILE;
    
    static bool parseConfigFile();
    static bool writeConfigFile();
    static void setDefaults();
};

#endif
