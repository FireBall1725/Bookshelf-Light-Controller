#include "HomeAssistantMQTT.h"

// Static member initialization
AsyncMqttClient HomeAssistantMQTT::mqttClient;
bool HomeAssistantMQTT::connected = false;
unsigned long HomeAssistantMQTT::lastReconnectAttempt = 0;

bool HomeAssistantMQTT::init() {
    MQTTConfig config = ConfigManager::getMQTTConfig();
    
    mqttClient.setServer(config.brokerIP.c_str(), config.brokerPort);
    
    if (config.username.length() > 0) {
        mqttClient.setCredentials(config.username.c_str(), config.password.c_str());
    }
    
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);
    
    return true;
}

bool HomeAssistantMQTT::connect() {
    if (mqttClient.connected()) {
        return true;
    }
    
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        mqttClient.connect();
    }
    
    return false;
}

void HomeAssistantMQTT::disconnect() {
    mqttClient.disconnect();
}

bool HomeAssistantMQTT::isConnected() {
    return mqttClient.connected();
}

void HomeAssistantMQTT::publishDeviceInfo() {
    MQTTConfig config = ConfigManager::getMQTTConfig();
    
    DynamicJsonDocument doc(1024);
    doc["identifiers"] = config.deviceId;
    doc["name"] = config.deviceName;
    doc["manufacturer"] = "ESP32";
    doc["model"] = "ESP32-C3-Mini";
    
    String deviceInfo;
    serializeJson(doc, deviceInfo);
    
    String topic = getDiscoveryTopic() + "/device";
    mqttClient.publish(topic.c_str(), 0, false, deviceInfo.c_str());
}

void HomeAssistantMQTT::publishLEDState(const String& state) {
    String topic = getStateTopic() + "/led";
    mqttClient.publish(topic.c_str(), 0, false, state.c_str());
}

void HomeAssistantMQTT::publishI2CDevices(const String& status) {
    String topic = getStateTopic() + "/i2c";
    mqttClient.publish(topic.c_str(), 0, false, status.c_str());
}

void HomeAssistantMQTT::publishSystemStatus(const String& uptime, int rssi) {
    DynamicJsonDocument doc(512);
    doc["uptime"] = uptime;
    doc["rssi"] = rssi;
    doc["free_heap"] = ESP.getFreeHeap();
    
    String statusPayload;
    serializeJson(doc, statusPayload);
    
    String topic = getStateTopic() + "/system";
    mqttClient.publish(topic.c_str(), 0, false, statusPayload.c_str());
}

void HomeAssistantMQTT::setMessageCallback(std::function<void(const String&, const String&)> callback) {
    // Store the callback for use in onMqttMessage
    // This is a simplified implementation
}

String HomeAssistantMQTT::getDiscoveryTopic() {
    MQTTConfig config = ConfigManager::getMQTTConfig();
    return config.mqttPrefix + "/sensor/" + config.deviceId;
}

String HomeAssistantMQTT::getStateTopic() {
    MQTTConfig config = ConfigManager::getMQTTConfig();
    return config.mqttPrefix + "/sensor/" + config.deviceId + "/state";
}

String HomeAssistantMQTT::getCommandTopic() {
    MQTTConfig config = ConfigManager::getMQTTConfig();
    return config.mqttPrefix + "/sensor/" + config.deviceId + "/command";
}

String HomeAssistantMQTT::getAvailabilityTopic() {
    MQTTConfig config = ConfigManager::getMQTTConfig();
    return config.mqttPrefix + "/sensor/" + config.deviceId + "/availability";
}

void HomeAssistantMQTT::onMqttConnect(bool sessionPresent) {
    connected = true;
    
    // Publish availability
    String availabilityTopic = getAvailabilityTopic();
    mqttClient.publish(availabilityTopic.c_str(), 0, true, "online");
    
    // Publish device info
    publishDeviceInfo();
}

void HomeAssistantMQTT::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    connected = false;
}

void HomeAssistantMQTT::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    // Handle incoming MQTT messages if needed
}

void HomeAssistantMQTT::publishDiscoveryMessage(const String& entityId, const String& name, const String& deviceClass, const String& stateClass) {
    MQTTConfig config = ConfigManager::getMQTTConfig();
    
    DynamicJsonDocument doc(1024);
    doc["name"] = name;
    doc["device_class"] = deviceClass;
    doc["state_class"] = stateClass;
    doc["device"]["identifiers"] = config.deviceId;
    
    String discoveryPayload;
    serializeJson(doc, discoveryPayload);
    
    String topic = getDiscoveryTopic() + "/" + entityId + "/config";
    mqttClient.publish(topic.c_str(), 0, true, discoveryPayload.c_str());
}
