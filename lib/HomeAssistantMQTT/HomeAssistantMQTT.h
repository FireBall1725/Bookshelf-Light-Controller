#ifndef HOMEASSISTANTMQTT_H
#define HOMEASSISTANTMQTT_H

#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include "ConfigManager.h"

class HomeAssistantMQTT {
public:
    static bool init();
    static bool connect();
    static void disconnect();
    static bool isConnected();
    
    // Device discovery
    static void publishDeviceInfo();
    static void publishLEDState(const String& state);
    static void publishI2CDevices(const String& status);
    static void publishSystemStatus(const String& uptime, int rssi);
    
    // Message handling
    static void setMessageCallback(std::function<void(const String&, const String&)> callback);
    
    // Topic helpers
    static String getDiscoveryTopic();
    static String getStateTopic();
    static String getCommandTopic();
    static String getAvailabilityTopic();

private:
    static AsyncMqttClient mqttClient;
    static bool connected;
    static unsigned long lastReconnectAttempt;
    
    static void onMqttConnect(bool sessionPresent);
    static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    static void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    
    static void publishDiscoveryMessage(const String& entityId, const String& name, const String& deviceClass, const String& stateClass);
};

#endif
