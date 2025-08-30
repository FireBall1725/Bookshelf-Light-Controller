#include "ConfigManager.h"

// Static member initialization
MQTTConfig ConfigManager::mqttConfig;
WiFiConfig ConfigManager::wifiConfig;
const char* ConfigManager::CONFIG_FILE = "/config.json";

bool ConfigManager::init() {
    // SPIFFS is initialized in main.cpp, so we don't need to initialize it here
    setDefaults();
    loadConfig();
    return true;
}

bool ConfigManager::loadConfig() {
    return parseConfigFile();
}

bool ConfigManager::saveConfig() {
    return writeConfigFile();
}

MQTTConfig ConfigManager::getMQTTConfig() {
    return mqttConfig;
}

bool ConfigManager::updateMQTTConfig(const String& brokerIP, int brokerPort, 
                                   const String& username, const String& password,
                                   const String& deviceName, const String& deviceId,
                                   const String& mqttPrefix) {
    mqttConfig.brokerIP = brokerIP;
    mqttConfig.brokerPort = brokerPort;
    mqttConfig.username = username;
    mqttConfig.password = password;
    mqttConfig.deviceName = deviceName;
    mqttConfig.deviceId = deviceId;
    mqttConfig.mqttPrefix = mqttPrefix;
    
    return saveConfig();
}

WiFiConfig ConfigManager::getWiFiConfig() {
    return wifiConfig;
}

void ConfigManager::setWiFiConfig(const String& ssid, const String& password) {
    wifiConfig.ssid = ssid;
    wifiConfig.password = password;
    saveConfig();
}

bool ConfigManager::parseConfigFile() {
    if (!SPIFFS.exists(CONFIG_FILE)) {
        return false;
    }
    
    File file = SPIFFS.open(CONFIG_FILE, "r");
    if (!file) {
        return false;
    }
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        return false;
    }
    
    // Parse MQTT config
    if (doc.containsKey("mqtt")) {
        JsonObject mqtt = doc["mqtt"];
        mqttConfig.brokerIP = mqtt["brokerIP"] | "192.168.1.100";
        mqttConfig.brokerPort = mqtt["brokerPort"] | 1883;
        mqttConfig.username = mqtt["username"] | "";
        mqttConfig.password = mqtt["password"] | "";
        mqttConfig.deviceName = mqtt["deviceName"] | "ESP32_C3_Mini";
        mqttConfig.deviceId = mqtt["deviceId"] | "esp32_c3_mini_1";
        mqttConfig.mqttPrefix = mqtt["mqttPrefix"] | "homeassistant";
    }
    
    // Parse WiFi config
    if (doc.containsKey("wifi")) {
        JsonObject wifi = doc["wifi"];
        wifiConfig.ssid = wifi["ssid"] | "";
        wifiConfig.password = wifi["password"] | "";
    }
    
    return true;
}

bool ConfigManager::writeConfigFile() {
    DynamicJsonDocument doc(1024);
    
    // MQTT config
    JsonObject mqtt = doc.createNestedObject("mqtt");
    mqtt["brokerIP"] = mqttConfig.brokerIP;
    mqtt["brokerPort"] = mqttConfig.brokerPort;
    mqtt["username"] = mqttConfig.username;
    mqtt["password"] = mqttConfig.password;
    mqtt["deviceName"] = mqttConfig.deviceName;
    mqtt["deviceId"] = mqttConfig.deviceId;
    mqtt["mqttPrefix"] = mqttConfig.mqttPrefix;
    
    // WiFi config
    JsonObject wifi = doc.createNestedObject("wifi");
    wifi["ssid"] = wifiConfig.ssid;
    wifi["password"] = wifiConfig.password;
    
    File file = SPIFFS.open(CONFIG_FILE, "w");
    if (!file) {
        return false;
    }
    
    size_t bytesWritten = serializeJson(doc, file);
    file.close();
    
    return bytesWritten > 0;
}

void ConfigManager::setDefaults() {
    mqttConfig.brokerIP = "192.168.1.100";
    mqttConfig.brokerPort = 1883;
    mqttConfig.username = "";
    mqttConfig.password = "";
    mqttConfig.deviceName = "ESP32_C3_Mini";
    mqttConfig.deviceId = "esp32_c3_mini_1";
    mqttConfig.mqttPrefix = "homeassistant";
    
    wifiConfig.ssid = "";
    wifiConfig.password = "";
}
